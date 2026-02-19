#include <CLI/CLI.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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

struct SignVerifyOptions {
    std::string input;
    std::vector<std::string> trusted_certs;
    bool load_system_certs = true;
    bool skip_cert_validation = false;
    bool check_signing_time = false;
    bool allow_weak = false;
    bool json_output = false;
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

struct SigVerifyResult {
    int index = 0;
    SignatureVerificationStatusType status = SignatureStatus_Undefined;
    std::string message;
    bool sig_valid    = false;
    bool doc_intact   = false;
    bool cert_trusted = false;
    std::string signer;
};

static const char* verification_status_string(SignatureVerificationStatusType status) {
    switch (status) {
    case SignatureStatus_Valid:                   return "Valid";
    case SignatureStatus_Invalid:                return "Invalid";
    case SignatureStatus_CertificateExpired:     return "CertificateExpired";
    case SignatureStatus_CertificateNotYetValid: return "CertificateNotYetValid";
    case SignatureStatus_CertificateRevoked:     return "CertificateRevoked";
    case SignatureStatus_CertificateUntrusted:   return "CertificateUntrusted";
    case SignatureStatus_DocumentModified:       return "DocumentModified";
    case SignatureStatus_WeakAlgorithm:          return "WeakAlgorithm";
    case SignatureStatus_MissingCertificate:     return "MissingCertificate";
    case SignatureStatus_Unknown:                return "Unknown";
    default:                                     return "Undefined";
    }
}

static std::vector<SigVerifyResult> collect_verify_results(
    const std::string& input_path,
    TrustedCertificateStoreHandle* store,
    SignatureVerificationSettingsHandle* settings)
{
    DocumentPtr document;
    check(Document_Open(input_path.c_str(), document.put()), "Failed to open document");

    CatalogPtr catalog;
    check(Document_GetCatalog(document.get(), catalog.put()), "Failed to get catalog");

    InteractiveFormPtr acro_form;
    error_type form_err = Catalog_GetAcroForm(catalog.get(), acro_form.put());
    if (form_err == VANILLAPDF_ERROR_OPTIONAL_ENTRY_MISSING) {
        return {};
    }
    check(form_err, "Failed to get interactive form");

    FieldCollectionPtr fields;
    check(InteractiveForm_GetFields(acro_form.get(), fields.put()), "Failed to get fields");

    size_type field_count = 0;
    check(FieldCollection_GetSize(fields.get(), &field_count), "Failed to get field count");

    std::vector<SigVerifyResult> results;
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

        // Extract signed byte ranges from file
        ByteRangeCollectionPtr ranges;
        check(DigitalSignature_GetByteRange(sig.get(), ranges.put()), "Failed to get byte range");

        size_type range_count = 0;
        check(ByteRangeCollection_GetSize(ranges.get(), &range_count), "Failed to get range count");

        std::vector<char> signed_bytes;
        std::ifstream pdf_file(input_path, std::ios::binary);
        check(pdf_file ? VANILLAPDF_ERROR_SUCCESS : VANILLAPDF_ERROR_GENERAL,
              "Failed to open file for byte range extraction");

        for (size_type r = 0; r < range_count; ++r) {
            ByteRangePtr range;
            check(ByteRangeCollection_GetValue(ranges.get(), r, range.put()), "Failed to get byte range entry");

            IntegerObjectPtr offset_obj, length_obj;
            check(ByteRange_GetOffset(range.get(), offset_obj.put()), "Failed to get range offset");
            check(ByteRange_GetLength(range.get(), length_obj.put()), "Failed to get range length");

            bigint_type offset = 0, length = 0;
            check(IntegerObject_GetIntegerValue(offset_obj.get(), &offset), "Failed to get offset value");
            check(IntegerObject_GetIntegerValue(length_obj.get(), &length), "Failed to get length value");

            std::vector<char> chunk(static_cast<size_t>(length));
            pdf_file.seekg(static_cast<std::streamoff>(offset));
            pdf_file.read(chunk.data(), static_cast<std::streamsize>(length));
            signed_bytes.insert(signed_bytes.end(), chunk.begin(), chunk.end());
        }

        BufferPtr signed_data_buf;
        check(Buffer_CreateFromData(signed_bytes.data(), signed_bytes.size(), signed_data_buf.put()),
              "Failed to create signed data buffer");

        HexadecimalStringObjectPtr contents;
        check(DigitalSignature_GetContents(sig.get(), contents.put()), "Failed to get signature contents");

        BufferPtr sig_buf;
        check(HexadecimalStringObject_GetValue(contents.get(), sig_buf.put()), "Failed to get signature bytes");

        SignatureVerificationResultPtr vresult;
        check(SignatureVerifier_Verify(signed_data_buf.get(), sig_buf.get(), store, settings, vresult.put()),
              "Failed to verify signature");

        SigVerifyResult result;
        result.index = ++sig_index;

        check(SignatureVerificationResult_GetStatus(vresult.get(), &result.status),
              "Failed to get verification status");

        BufferPtr msg_buf;
        if (SignatureVerificationResult_GetMessage(vresult.get(), msg_buf.put()) == VANILLAPDF_ERROR_SUCCESS) {
            string_type msg_data = nullptr;
            size_type msg_size = 0;
            if (Buffer_GetData(msg_buf.get(), &msg_data, &msg_size) == VANILLAPDF_ERROR_SUCCESS && msg_size > 0) {
                result.message = std::string(msg_data, msg_size);
            }
        }

        boolean_type bval = VANILLAPDF_RV_FALSE;
        check(SignatureVerificationResult_IsSignatureValid(vresult.get(), &bval), "Failed to check signature validity");
        result.sig_valid = (bval != VANILLAPDF_RV_FALSE);

        bval = VANILLAPDF_RV_FALSE;
        check(SignatureVerificationResult_IsDocumentIntact(vresult.get(), &bval), "Failed to check document integrity");
        result.doc_intact = (bval != VANILLAPDF_RV_FALSE);

        bval = VANILLAPDF_RV_FALSE;
        check(SignatureVerificationResult_IsCertificateTrusted(vresult.get(), &bval), "Failed to check certificate trust");
        result.cert_trusted = (bval != VANILLAPDF_RV_FALSE);

        BufferPtr name_buf;
        if (SignatureVerificationResult_GetSignerCommonName(vresult.get(), name_buf.put()) == VANILLAPDF_ERROR_SUCCESS) {
            string_type name_data = nullptr;
            size_type name_size = 0;
            if (Buffer_GetData(name_buf.get(), &name_data, &name_size) == VANILLAPDF_ERROR_SUCCESS && name_size > 0) {
                result.signer = std::string(name_data, name_size);
            }
        }

        results.push_back(std::move(result));
    }

    return results;
}

static void print_verify_text(const std::string& file, const std::vector<SigVerifyResult>& results) {
    std::cout << "File: " << file << "\n";
    if (results.empty()) {
        std::cout << "No signatures found\n";
        return;
    }
    for (const auto& r : results) {
        std::cout << "Signature #" << r.index << ":\n";
        std::cout << "  Status: " << verification_status_string(r.status) << "\n";
        if (!r.message.empty()) {
            std::cout << "  Message: " << r.message << "\n";
        }
        std::cout << "  Signature valid: " << (r.sig_valid ? "yes" : "no") << "\n";
        std::cout << "  Document intact: " << (r.doc_intact ? "yes" : "no") << "\n";
        std::cout << "  Certificate trusted: " << (r.cert_trusted ? "yes" : "no") << "\n";
        if (!r.signer.empty()) {
            std::cout << "  Signer: " << r.signer << "\n";
        }
    }
}

static std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    out += '"';
    for (char c : s) {
        if (c == '"')       out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else                out += c;
    }
    out += '"';
    return out;
}

static void print_verify_json(const std::string& file, const std::vector<SigVerifyResult>& results) {
    std::cout << "{\n";
    std::cout << "  \"file\": " << json_escape(file) << ",\n";
    std::cout << "  \"signatures\": [";
    for (size_t j = 0; j < results.size(); ++j) {
        const auto& r = results[j];
        if (j > 0) std::cout << ",";
        std::cout << "\n    {\n";
        std::cout << "      \"index\": " << r.index << ",\n";
        std::cout << "      \"status\": " << json_escape(verification_status_string(r.status)) << ",\n";
        if (!r.message.empty()) {
            std::cout << "      \"message\": " << json_escape(r.message) << ",\n";
        }
        std::cout << "      \"signature_valid\": " << (r.sig_valid ? "true" : "false") << ",\n";
        std::cout << "      \"document_intact\": " << (r.doc_intact ? "true" : "false") << ",\n";
        std::cout << "      \"certificate_trusted\": " << (r.cert_trusted ? "true" : "false");
        if (!r.signer.empty()) {
            std::cout << ",\n      \"signer\": " << json_escape(r.signer);
        }
        std::cout << "\n    }";
    }
    std::cout << "\n  ]\n}\n";
}

static int run_sign_verify(const SignVerifyOptions& opts) {
    TrustedCertificateStorePtr store;
    check(TrustedCertificateStore_Create(store.put()), "Failed to create certificate store");
    if (opts.load_system_certs) {
        check(TrustedCertificateStore_LoadSystemDefaults(store.get()), "Failed to load system certificates");
    }
    for (const auto& cert_path : opts.trusted_certs) {
        std::ifstream cert_file(cert_path, std::ios::binary);
        check(cert_file ? VANILLAPDF_ERROR_SUCCESS : VANILLAPDF_ERROR_GENERAL,
              "Failed to open certificate file: " + cert_path);
        std::string pem_data((std::istreambuf_iterator<char>(cert_file)), std::istreambuf_iterator<char>());
        BufferPtr cert_buf;
        check(Buffer_CreateFromData(pem_data.data(), pem_data.size(), cert_buf.put()),
              "Failed to create certificate buffer");
        check(TrustedCertificateStore_AddCertificateFromPEM(store.get(), cert_buf.get()),
              "Failed to add certificate from: " + cert_path);
    }

    SignatureVerificationSettingsPtr settings;
    check(SignatureVerificationSettings_Create(settings.put()), "Failed to create verification settings");
    if (opts.skip_cert_validation) {
        check(SignatureVerificationSettings_SetSkipCertificateValidation(settings.get(), VANILLAPDF_RV_TRUE),
              "Failed to set skip certificate validation");
    }
    if (opts.check_signing_time) {
        check(SignatureVerificationSettings_SetCheckSigningTimeFlag(settings.get(), VANILLAPDF_RV_TRUE),
              "Failed to set check signing time");
    }
    if (opts.allow_weak) {
        check(SignatureVerificationSettings_SetAllowWeakAlgorithmsFlag(settings.get(), VANILLAPDF_RV_TRUE),
              "Failed to set allow weak algorithms");
    }

    auto results = collect_verify_results(opts.input, store.get(), settings.get());

    if (opts.json_output) {
        print_verify_json(opts.input, results);
    } else {
        print_verify_text(opts.input, results);
    }

    return 0;
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

    // sign verify
    auto verify_opts = std::make_shared<SignVerifyOptions>();
    auto* verify_cmd = cmd->add_subcommand("verify", "Verify digital signatures in a PDF document");
    verify_cmd->add_option("input", verify_opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    verify_cmd->add_option("-c,--trusted-cert", verify_opts->trusted_certs,
                           "PEM certificate file to add to the trust store (repeatable)")
        ->check(CLI::ExistingFile);
    verify_cmd->add_flag("!--no-system-certs", verify_opts->load_system_certs,
                         "Do not load system default trusted certificates");
    verify_cmd->add_flag("--skip-cert-validation", verify_opts->skip_cert_validation,
                         "Skip certificate chain validation (cryptographic check only)");
    verify_cmd->add_flag("--check-signing-time", verify_opts->check_signing_time,
                         "Verify that the certificate was valid at the time of signing");
    verify_cmd->add_flag("--allow-weak", verify_opts->allow_weak,
                         "Allow weak digest algorithms (MD5, SHA-1, RSA < 2048)");
    verify_cmd->add_flag("--json", verify_opts->json_output,
                         "Output results as JSON");

    verify_cmd->callback([verify_opts]() {
        try {
            std::exit(run_sign_verify(*verify_opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
