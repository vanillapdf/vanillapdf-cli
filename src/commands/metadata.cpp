#include <CLI/CLI.hpp>
#include <ctime>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include <fmt/chrono.h>

#include "commands/commands.hpp"
#include "utils/handles.hpp"

namespace vanillapdf::cli::commands {

struct MetadataOptions {
    std::string input;
    std::string output;
    std::string password;
    // nullopt: leave untouched. empty string: remove the key. otherwise: set it.
    std::optional<std::string> title;
    std::optional<std::string> author;
    std::optional<std::string> subject;
    std::optional<std::string> keywords;
    std::optional<std::string> creator;
};

// A user-settable field of the document information dictionary. These are the
// fields describing the *content* of the document; the fields describing its
// *production* (Producer, ModDate) are managed by the tool, not the user.
struct MetadataField {
    const char* key;          // PDF dictionary key, e.g. "Title"
    const char* flag;         // CLI option, e.g. "--title"
    const char* description;  // help text
    std::optional<std::string> MetadataOptions::*member;
};

static const MetadataField metadata_fields[] = {
    {"Title",    "--title",    "Document title",                &MetadataOptions::title},
    {"Author",   "--author",   "Document author",               &MetadataOptions::author},
    {"Subject",  "--subject",  "Document subject",               &MetadataOptions::subject},
    {"Keywords", "--keywords", "Document keywords",              &MetadataOptions::keywords},
    {"Creator",  "--creator",  "Application that created the original", &MetadataOptions::creator},
};

// Format the current local time as a PDF date string: D:YYYYMMDDHHmmSSOHH'mm'
// (PDF 32000-1:2008, 7.9.4). Emitted in UTC, so the timezone is a literal
// "+00'00'" and no local-offset calculation is needed.
static std::string current_pdf_date() {
    return fmt::format("D:{:%Y%m%d%H%M%S}+00'00'", fmt::gmtime(std::time(nullptr)));
}

// Resolve a trailer entry (a direct dictionary or an indirect reference to one)
// to its underlying dictionary object.
static DictionaryObjectPtr resolve_dictionary(ObjectHandle* obj) {
    ObjectType type = ObjectType_Undefined;
    check(Object_GetObjectType(obj, &type), "Failed to get object type");

    if (type == ObjectType_IndirectReference) {
        IndirectReferenceObjectPtr ref;
        check(IndirectReferenceObject_FromObject(obj, ref.put()), "Failed to read reference");
        ObjectPtr referenced;
        check(IndirectReferenceObject_GetReferencedObject(ref.get(), referenced.put()),
              "Failed to resolve reference");
        DictionaryObjectPtr dict;
        check(DictionaryObject_FromObject(referenced.get(), dict.put()),
              "Info entry does not reference a dictionary");
        return dict;
    }

    DictionaryObjectPtr dict;
    check(DictionaryObject_FromObject(obj, dict.put()), "Info entry is not a dictionary");
    return dict;
}

// Locate the document information dictionary. The chain is ordered newest-first,
// so the first trailer holding an /Info entry wins. If no trailer has one, create
// a fresh Info dictionary, register it as a new indirect object, and link it from
// the newest trailer.
static DictionaryObjectPtr get_or_create_info(FileHandle* file) {
    XrefChainPtr chain;
    check(File_XrefChain(file, chain.put()), "Failed to get xref chain");

    XrefChainIteratorPtr it;
    check(XrefChain_GetIterator(chain.get(), it.put()), "Failed to iterate xref chain");

    NameObjectPtr info_key;
    check(NameObject_CreateFromDecodedString("Info", info_key.put()), "Failed to create Info name");

    DictionaryObjectPtr newest_trailer;

    while (true) {
        boolean_type valid = VANILLAPDF_RV_FALSE;
        check(XrefChainIterator_IsValid(it.get(), &valid), "Failed to check iterator");
        if (valid != VANILLAPDF_RV_TRUE) {
            break;
        }

        XrefPtr xref;
        check(XrefChainIterator_GetValue(it.get(), xref.put()), "Failed to get xref section");

        DictionaryObjectPtr trailer;
        check(Xref_GetTrailerDictionary(xref.get(), trailer.put()), "Failed to get trailer");

        boolean_type found = VANILLAPDF_RV_FALSE;
        ObjectPtr info_obj;
        check(DictionaryObject_TryFind(trailer.get(), info_key.get(), &found, info_obj.put()),
              "Failed to look up Info entry");
        if (found == VANILLAPDF_RV_TRUE) {
            return resolve_dictionary(info_obj.get());
        }

        // Remember the newest trailer as the place to attach a new Info dictionary.
        if (!newest_trailer) {
            newest_trailer = std::move(trailer);
        }

        check(XrefChainIterator_Next(it.get()), "Failed to advance iterator");
    }

    if (!newest_trailer) {
        throw pdf_error(VANILLAPDF_ERROR_GENERAL, "Document has no trailer dictionary");
    }

    // No Info dictionary present: create one and register it as a new indirect object.
    DictionaryObjectPtr info;
    check(DictionaryObject_Create(info.put()), "Failed to create Info dictionary");

    ObjectPtr info_as_object;
    check(DictionaryObject_ToObject(info.get(), info_as_object.put()), "Failed to convert Info dictionary");

    XrefUsedEntryPtr entry;
    check(File_AllocateNewEntry(file, entry.put()), "Failed to allocate new object entry");
    check(XrefUsedEntry_SetReference(entry.get(), info_as_object.get()), "Failed to register Info object");

    biguint_type obj_number = 0;
    ushort_type gen_number = 0;
    check(XrefUsedEntry_GetObjectNumber(entry.get(), &obj_number), "Failed to read object number");
    check(XrefUsedEntry_GetGenerationNumber(entry.get(), &gen_number), "Failed to read generation number");

    // Link the new object from the trailer as an indirect reference.
    IndirectReferenceObjectPtr ref;
    check(IndirectReferenceObject_Create(ref.put()), "Failed to create Info reference");
    check(IndirectReferenceObject_SetObjectNumber(ref.get(), obj_number), "Failed to set reference number");
    check(IndirectReferenceObject_SetGenerationNumber(ref.get(), gen_number), "Failed to set reference generation");

    ObjectPtr ref_as_object;
    check(IndirectReferenceObject_ToObject(ref.get(), ref_as_object.put()), "Failed to convert Info reference");
    check(DictionaryObject_Insert(newest_trailer.get(), info_key.get(), ref_as_object.get(), VANILLAPDF_RV_TRUE),
          "Failed to link Info dictionary into trailer");

    return info;
}

// Set a literal-string entry in the info dictionary, or remove it when the value
// is empty.
static void apply_field(DictionaryObjectHandle* info, const char* key, const std::string& value) {
    NameObjectPtr name;
    check(NameObject_CreateFromDecodedString(key, name.put()), "Failed to create name object");

    if (value.empty()) {
        boolean_type removed = VANILLAPDF_RV_FALSE;
        check(DictionaryObject_Remove(info, name.get(), &removed), "Failed to remove entry");
        return;
    }

    LiteralStringObjectPtr literal;
    check(LiteralStringObject_CreateFromDecodedString(value.c_str(), literal.put()),
          "Failed to create string value");

    StringObjectPtr string_obj;
    check(LiteralStringObject_ToStringObject(literal.get(), string_obj.put()), "Failed to convert string");

    ObjectPtr value_obj;
    check(StringObject_ToObject(string_obj.get(), value_obj.put()), "Failed to convert value");

    check(DictionaryObject_Insert(info, name.get(), value_obj.get(), VANILLAPDF_RV_TRUE),
          "Failed to set entry");
}

static int run_metadata_set(const MetadataOptions& opts) {
    FilePtr file;
    check(File_Open(opts.input.c_str(), file.put()), "Failed to open file");

    // Document_OpenFile initializes the file (reads the xref chain) and shares its
    // object graph, so mutations below are persisted by the standard Document_Save.
    DocumentPtr document;
    check(Document_OpenFile(file.get(), document.put()), "Failed to open document");

    // For an encrypted document the writer must re-encrypt the strings we insert,
    // which needs the encryption key. Establish it from the password (which may be
    // empty for owner-only encryption). The password must be set after the file is
    // initialized, matching the encrypt/decrypt path.
    boolean_type encrypted = VANILLAPDF_RV_FALSE;
    check(File_IsEncrypted(file.get(), &encrypted), "Failed to determine encryption state");
    if (encrypted == VANILLAPDF_RV_TRUE) {
        check(File_SetEncryptionPassword(file.get(), opts.password.c_str()),
              "Failed to set password");
    }

    DictionaryObjectPtr info = get_or_create_info(file.get());

    for (const auto& field : metadata_fields) {
        const std::optional<std::string>& value = opts.*(field.member);
        if (value.has_value()) {
            apply_field(info.get(), field.key, *value);
        }
    }

    // Production fields are always stamped: a full save republishes the document,
    // so this tool is its producer and "now" is its modification time.
    apply_field(info.get(), "Producer", "vanillapdf-cli " VANILLAPDF_CLI_VERSION);
    apply_field(info.get(), "ModDate", current_pdf_date());

    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save document");
    std::cerr << "Output: " << opts.output << "\n";

    return 0;
}

void register_metadata(CLI::App& app) {
    auto* cmd = app.add_subcommand("metadata", "Read or write PDF document metadata");
    cmd->require_subcommand(1);

    auto set_opts = std::make_shared<MetadataOptions>();
    auto* set_cmd = cmd->add_subcommand("set", "Set document information fields");
    set_cmd->add_option("input", set_opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    set_cmd->add_option("-o,--output", set_opts->output, "Output PDF file")
        ->required();
    set_cmd->add_option("-p,--password", set_opts->password,
                        "Password for an encrypted input (required to re-encrypt on save)");

    for (const auto& field : metadata_fields) {
        set_cmd->add_option(field.flag, set_opts.get()->*(field.member), field.description);
    }

    set_cmd->callback([set_opts]() {
        try {
            std::exit(run_metadata_set(*set_opts));
        } catch (const pdf_error& e) {
            report_error(e);
            std::exit(1);
        }
    });
}

} // namespace vanillapdf::cli::commands
