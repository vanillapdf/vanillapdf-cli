#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "commands/commands.hpp"
#include "utils/error.hpp"

namespace vpdf::commands {

struct SignAddOptions {
    std::string input;
    std::string output;
    std::string key_file;
    std::string password;
};

struct SignListOptions {
    std::string input;
};

struct SignRemoveOptions {
    std::string input;
    std::string output;
};

static int run_sign_add(const SignAddOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    PKCS12KeyPtr pkcs12;
    check(PKCS12Key_CreateFromFile(opts.key_file.c_str(), opts.password.c_str(), pkcs12.put()),
          "Failed to load PKCS#12 key");

    SigningKeyPtr signing_key;
    check(PKCS12Key_ToSigningKey(pkcs12.get(), signing_key.put()), "Failed to create signing key");

    DocumentSignatureSettingsPtr settings;
    check(DocumentSignatureSettings_Create(settings.put()), "Failed to create signature settings");
    check(DocumentSignatureSettings_SetSigningKey(settings.get(), signing_key.get()),
          "Failed to set signing key");

    FilePtr dest_file;
    check(File_Create(opts.output.c_str(), dest_file.put()), "Failed to create output file");

    check(Document_Sign(document.get(), dest_file.get(), settings.get()), "Failed to sign document");
    std::cerr << "Signed: " << opts.output << "\n";

    return 0;
}

static void print_optional_sig_field(DigitalSignatureHandle* sig, const char* label,
                                     error_type (*getter)(DigitalSignatureHandle*, StringObjectHandle**)) {
    StringObjectPtr str;
    error_type err = getter(sig, str.put());
    if (err == VANILLAPDF_ERROR_OPTIONAL_ENTRY_MISSING) return;
    check(err, label);
    std::cout << "    " << label << ": " << to_string(str.get()) << "\n";
}

static int run_sign_list(const SignListOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    CatalogPtr catalog;
    check(Document_GetCatalog(document.get(), catalog.put()), "Failed to get catalog");

    InteractiveFormPtr acro_form;
    error_type form_err = Catalog_GetAcroForm(catalog.get(), acro_form.put());
    if (form_err == VANILLAPDF_ERROR_OPTIONAL_ENTRY_MISSING) {
        std::cout << "File: " << opts.input << "\n";
        std::cout << "Signatures: 0\n";
        return 0;
    }
    check(form_err, "Failed to get interactive form");

    FieldCollectionPtr fields;
    check(InteractiveForm_GetFields(acro_form.get(), fields.put()), "Failed to get fields");

    size_type field_count = 0;
    check(FieldCollection_GetSize(fields.get(), &field_count), "Failed to get field count");

    std::cout << "File: " << opts.input << "\n";

    int sig_index = 0;
    for (size_type i = 0; i < field_count; ++i) {
        FieldPtr field;
        check(FieldCollection_At(fields.get(), i, field.put()), "Failed to get field");

        FieldType type = FieldType_Undefined;
        check(Field_GetType(field.get(), &type), "Failed to get field type");

        if (type != FieldType_Signature) continue;

        SignatureFieldPtr sig_field;
        check(Field_ToSignature(field.get(), sig_field.put()), "Failed to convert to signature field");

        DigitalSignaturePtr sig;
        error_type sig_err = SignatureField_GetValue(sig_field.get(), sig.put());
        if (sig_err == VANILLAPDF_ERROR_OPTIONAL_ENTRY_MISSING) continue;
        check(sig_err, "Failed to get signature value");

        ++sig_index;
        std::cout << "Signature #" << sig_index << ":\n";

        print_optional_sig_field(sig.get(), "Name", DigitalSignature_GetName);
        print_optional_sig_field(sig.get(), "Reason", DigitalSignature_GetReason);
        print_optional_sig_field(sig.get(), "Location", DigitalSignature_GetLocation);
        print_optional_sig_field(sig.get(), "Contact", DigitalSignature_GetContactInfo);
    }

    std::cout << "Signatures: " << sig_index << "\n";

    return 0;
}

static int run_sign_remove(const SignRemoveOptions& /*opts*/) {
    std::cerr << "Error: Signature removal is not yet supported by the vanillapdf library\n";
    return 1;
}

void register_sign(CLI::App& app) {
    auto* cmd = app.add_subcommand("sign", "Manage PDF digital signatures");
    cmd->require_subcommand(1);

    // sign add
    auto add_opts = std::make_shared<SignAddOptions>();
    auto* add_cmd = cmd->add_subcommand("add", "Add a digital signature to a PDF document");
    add_cmd->add_option("input", add_opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    add_cmd->add_option("-o,--output", add_opts->output, "Output PDF file")
        ->required();
    add_cmd->add_option("-k,--key", add_opts->key_file, "PKCS#12 key file (.p12/.pfx)")
        ->required()
        ->check(CLI::ExistingFile);
    add_cmd->add_option("-p,--password", add_opts->password, "Key file password");

    add_cmd->callback([add_opts]() {
        try {
            std::exit(run_sign_add(*add_opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });

    // sign list
    auto list_opts = std::make_shared<SignListOptions>();
    auto* list_cmd = cmd->add_subcommand("list", "List digital signatures in a PDF document");
    list_cmd->add_option("input", list_opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);

    list_cmd->callback([list_opts]() {
        try {
            std::exit(run_sign_list(*list_opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });

    // sign remove
    auto rm_opts = std::make_shared<SignRemoveOptions>();
    auto* rm_cmd = cmd->add_subcommand("remove", "Remove digital signatures from a PDF document");
    rm_cmd->add_option("input", rm_opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    rm_cmd->add_option("-o,--output", rm_opts->output, "Output PDF file")
        ->required();

    rm_cmd->callback([rm_opts]() {
        try {
            std::exit(run_sign_remove(*rm_opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
