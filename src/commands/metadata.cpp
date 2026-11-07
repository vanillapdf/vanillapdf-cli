#include "commands/commands.h"
#include "utils/error.h"
#include "utils/handle_guard.h"

#include <CLI/CLI.hpp>
#include <fmt/chrono.h>

#include <ctime>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace vanillapdf {
namespace cli {

namespace {

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

const MetadataField METADATA_FIELDS[] = {
    {"Title",    "--title",    "Document title",                        &MetadataOptions::title},
    {"Author",   "--author",   "Document author",                       &MetadataOptions::author},
    {"Subject",  "--subject",  "Document subject",                      &MetadataOptions::subject},
    {"Keywords", "--keywords", "Document keywords",                     &MetadataOptions::keywords},
    {"Creator",  "--creator",  "Application that created the original", &MetadataOptions::creator},
};

// Format the current time as a PDF date string: D:YYYYMMDDHHmmSSOHH'mm'
// (PDF 32000-1:2008, 7.9.4). Emitted in UTC, so the timezone is a literal
// "+00'00'" and no local-offset calculation is needed.
std::string current_pdf_date() {
    auto current_time = std::time(nullptr);
    auto current_time_utc = fmt::gmtime(current_time);
    return fmt::format("D:{:%Y%m%d%H%M%S}+00'00'", current_time_utc);
}

// Resolve a trailer entry (a direct dictionary or an indirect reference to one)
// to its underlying dictionary object
DictionaryObjectGuard resolve_dictionary(ObjectHandle* object) {
    ObjectType object_type = ObjectType_Undefined;
    THROW_IF_NOT_SUCCESS(Object_GetObjectType(object, &object_type), "Failed to get object type");

    DictionaryObjectGuard dictionary;

    if (object_type == ObjectType_IndirectReference) {
        IndirectReferenceObjectGuard reference;
        THROW_IF_NOT_SUCCESS(IndirectReferenceObject_FromObject(object, reference.out()), "Failed to read reference");

        ObjectGuard referenced_object;
        THROW_IF_NOT_SUCCESS(IndirectReferenceObject_GetReferencedObject(reference, referenced_object.out()),
            "Failed to resolve reference");

        THROW_IF_NOT_SUCCESS(DictionaryObject_FromObject(referenced_object, dictionary.out()),
            "Info entry does not reference a dictionary");

        return dictionary;
    }

    THROW_IF_NOT_SUCCESS(DictionaryObject_FromObject(object, dictionary.out()), "Info entry is not a dictionary");
    return dictionary;
}

// Locate the document information dictionary. The chain is ordered newest-first,
// so the first trailer holding an /Info entry wins. If no trailer has one, create
// a fresh Info dictionary, register it as a new indirect object, and link it from
// the newest trailer.
DictionaryObjectGuard get_or_create_info(FileHandle* file) {
    XrefChainGuard xref_chain;
    THROW_IF_NOT_SUCCESS(File_XrefChain(file, xref_chain.out()), "Failed to get xref chain");

    XrefChainIteratorGuard xref_iterator;
    THROW_IF_NOT_SUCCESS(XrefChain_GetIterator(xref_chain, xref_iterator.out()), "Failed to iterate xref chain");

    NameObjectGuard info_key;
    THROW_IF_NOT_SUCCESS(NameObject_CreateFromDecodedString("Info", info_key.out()), "Failed to create Info name");

    DictionaryObjectGuard newest_trailer;

    while (true) {
        boolean_type iterator_valid = VANILLAPDF_RV_FALSE;
        THROW_IF_NOT_SUCCESS(XrefChainIterator_IsValid(xref_iterator, &iterator_valid), "Failed to check iterator");
        if (iterator_valid != VANILLAPDF_RV_TRUE) {
            break;
        }

        XrefGuard xref;
        THROW_IF_NOT_SUCCESS(XrefChainIterator_GetValue(xref_iterator, xref.out()), "Failed to get xref section");

        DictionaryObjectGuard trailer;
        THROW_IF_NOT_SUCCESS(Xref_GetTrailerDictionary(xref, trailer.out()), "Failed to get trailer");

        boolean_type info_found = VANILLAPDF_RV_FALSE;
        ObjectGuard info_object;
        THROW_IF_NOT_SUCCESS(DictionaryObject_TryFind(trailer, info_key, &info_found, info_object.out()),
            "Failed to look up Info entry");

        if (info_found == VANILLAPDF_RV_TRUE) {
            return resolve_dictionary(info_object);
        }

        // Remember the newest trailer as the place to attach a new Info dictionary.
        // The guard is move-only, so std::move is mandatory here.
        if (!newest_trailer) {
            newest_trailer = std::move(trailer);
        }

        THROW_IF_NOT_SUCCESS(XrefChainIterator_Next(xref_iterator), "Failed to advance iterator");
    }

    if (!newest_trailer) {
        throw PdfException(VANILLAPDF_ERROR_GENERAL, "Document has no trailer dictionary");
    }

    // No Info dictionary present: create one and register it as a new indirect object
    DictionaryObjectGuard info_dictionary;
    THROW_IF_NOT_SUCCESS(DictionaryObject_Create(info_dictionary.out()), "Failed to create Info dictionary");

    ObjectGuard info_dictionary_object;
    THROW_IF_NOT_SUCCESS(DictionaryObject_ToObject(info_dictionary, info_dictionary_object.out()),
        "Failed to convert Info dictionary");

    XrefUsedEntryGuard xref_entry;
    THROW_IF_NOT_SUCCESS(File_AllocateNewEntry(file, xref_entry.out()), "Failed to allocate new object entry");
    THROW_IF_NOT_SUCCESS(XrefUsedEntry_SetReference(xref_entry, info_dictionary_object),
        "Failed to register Info object");

    biguint_type object_number = 0;
    ushort_type generation_number = 0;
    THROW_IF_NOT_SUCCESS(XrefUsedEntry_GetObjectNumber(xref_entry, &object_number), "Failed to read object number");
    THROW_IF_NOT_SUCCESS(XrefUsedEntry_GetGenerationNumber(xref_entry, &generation_number),
        "Failed to read generation number");

    // Link the new object from the trailer as an indirect reference
    IndirectReferenceObjectGuard info_reference;
    THROW_IF_NOT_SUCCESS(IndirectReferenceObject_Create(info_reference.out()), "Failed to create Info reference");
    THROW_IF_NOT_SUCCESS(IndirectReferenceObject_SetObjectNumber(info_reference, object_number),
        "Failed to set reference number");
    THROW_IF_NOT_SUCCESS(IndirectReferenceObject_SetGenerationNumber(info_reference, generation_number),
        "Failed to set reference generation");

    ObjectGuard info_reference_object;
    THROW_IF_NOT_SUCCESS(IndirectReferenceObject_ToObject(info_reference, info_reference_object.out()),
        "Failed to convert Info reference");
    THROW_IF_NOT_SUCCESS(DictionaryObject_Insert(newest_trailer, info_key, info_reference_object, VANILLAPDF_RV_TRUE),
        "Failed to link Info dictionary into trailer");

    return info_dictionary;
}

// Set a literal-string entry in the info dictionary, or remove it when the value
// is empty
void apply_field(DictionaryObjectHandle* info_dictionary, const char* key, const std::string& value) {
    NameObjectGuard key_name;
    THROW_IF_NOT_SUCCESS(NameObject_CreateFromDecodedString(key, key_name.out()), "Failed to create name object");

    if (value.empty()) {
        boolean_type removed = VANILLAPDF_RV_FALSE;
        THROW_IF_NOT_SUCCESS(DictionaryObject_Remove(info_dictionary, key_name, &removed), "Failed to remove entry");
        return;
    }

    LiteralStringObjectGuard literal_string;
    THROW_IF_NOT_SUCCESS(LiteralStringObject_CreateFromDecodedString(value.c_str(), literal_string.out()),
        "Failed to create string value");

    StringObjectGuard string_object;
    THROW_IF_NOT_SUCCESS(LiteralStringObject_ToStringObject(literal_string, string_object.out()),
        "Failed to convert string");

    ObjectGuard value_object;
    THROW_IF_NOT_SUCCESS(StringObject_ToObject(string_object, value_object.out()), "Failed to convert value");

    THROW_IF_NOT_SUCCESS(DictionaryObject_Insert(info_dictionary, key_name, value_object, VANILLAPDF_RV_TRUE),
        "Failed to set entry");
}

int process_metadata_set(const MetadataOptions& options) {
    FileGuard file;
    THROW_IF_NOT_SUCCESS(File_Open(options.input.c_str(), file.out()), "Failed to open file");

    // Document_OpenFile initializes the file (reads the xref chain) and shares its
    // object graph, so mutations below are persisted by the standard Document_Save.
    DocumentGuard document;
    THROW_IF_NOT_SUCCESS(Document_OpenFile(file, document.out()), "Failed to open document");

    // For an encrypted document the writer must re-encrypt the strings we insert,
    // which needs the encryption key. Establish it from the password (which may be
    // empty for owner-only encryption). The password must be set after the file is
    // initialized, matching the encrypt/decrypt path.
    boolean_type encrypted = VANILLAPDF_RV_FALSE;
    THROW_IF_NOT_SUCCESS(File_IsEncrypted(file, &encrypted), "Failed to determine encryption state");
    if (encrypted == VANILLAPDF_RV_TRUE) {
        THROW_IF_NOT_SUCCESS(File_SetEncryptionPassword(file, options.password.c_str()), "Failed to set password");
    }

    auto info_dictionary = get_or_create_info(file);

    for (const auto& field : METADATA_FIELDS) {
        const auto& value = options.*(field.member);
        if (value.has_value()) {
            apply_field(info_dictionary, field.key, value.value());
        }
    }

    // Production fields are always stamped: a full save republishes the document,
    // so this tool is its producer and "now" is its modification time.
    auto modification_date = current_pdf_date();
    apply_field(info_dictionary, "Producer", "vanillapdf-cli " VANILLAPDF_CLI_VERSION);
    apply_field(info_dictionary, "ModDate", modification_date);

    THROW_IF_NOT_SUCCESS(Document_Save(document, options.output.c_str()), "Failed to save document");
    std::cerr << "Output: " << options.output << "\n";

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

} // namespace

void register_metadata(CLI::App& app, int& exit_code) {
    auto* command = app.add_subcommand("metadata", "Read or write PDF document metadata");
    command->require_subcommand(1);

    // metadata set
    auto set_options = std::make_shared<MetadataOptions>();
    auto* set_command = command->add_subcommand("set", "Set document information fields");

    set_command->add_option("input", set_options->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    set_command->add_option("-o,--output", set_options->output, "Output PDF file")
        ->required();
    set_command->add_option("-p,--password", set_options->password,
        "Password for an encrypted input (required to re-encrypt on save)");

    for (const auto& field : METADATA_FIELDS) {
        // Through a callback, not bound to the std::optional: CLI11 turns an empty
        // value for a wrapper type into a default-constructed one, which would leave
        // the field untouched instead of removing it.
        auto member = field.member;
        set_command->add_option_function<std::string>(
            field.flag,
            [set_options, member](const std::string& value) { (*set_options).*member = value; },
            field.description);
    }

    set_command->callback([set_options, &exit_code]() {
        exit_code = process_metadata_set(*set_options);
    });
}

} // cli
} // vanillapdf
