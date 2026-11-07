#include "commands/commands.h"
#include "utils/encoding.h"
#include "utils/error.h"
#include "utils/handle_guard.h"
#include "utils/json.h"

#include <CLI/CLI.hpp>
#include <fmt/format.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace vanillapdf {
namespace cli {

namespace {

struct SignAddOptions {
    std::string input;
    std::string output;
    std::string key_file;
    std::string password;
    std::string digest = "sha256";
    std::string name;
    std::string reason;
    std::string location;
    bool no_signing_time = false;  // by default the signing time is set to now
};

struct SignListOptions {
    std::string input;
    bool json_output = false;
};

struct SignVerifyOptions {
    std::string input;
    std::vector<std::string> trusted_certificates;
    bool load_system_certificates = true;
    bool skip_certificate_validation = false;
    bool check_signing_time = false;
    bool allow_weak_algorithms = false;
    bool json_output = false;
};

using SignatureSettingsStringSetter = error_type (*)(DocumentSignatureSettingsHandle*, LiteralStringObjectHandle*);
using SignatureStringGetter = error_type (*)(DigitalSignatureHandle*, StringObjectHandle**);

// The signature dictionary fields listed by `sign list`, with label and JSON key
struct SignatureField {
    const char* label;
    const char* key;
    SignatureStringGetter getter;
};

const SignatureField SIGNATURE_FIELDS[] = {
    {"Name",     "name",     DigitalSignature_GetName},
    {"Reason",   "reason",   DigitalSignature_GetReason},
    {"Location", "location", DigitalSignature_GetLocation},
    {"Contact",  "contact",  DigitalSignature_GetContactInfo},
};

constexpr int SIGNATURE_FIELD_COUNT = sizeof(SIGNATURE_FIELDS) / sizeof(SIGNATURE_FIELDS[0]);

struct SignatureListEntry {
    int index = 0;
    std::optional<std::string> values[SIGNATURE_FIELD_COUNT];
    std::optional<std::string> signing_time;
    std::optional<std::string> error;  // set when this signature could not be read
};

struct SignatureVerifyEntry {
    int index = 0;
    SignatureVerificationStatusType status = SignatureStatus_Undefined;
    std::string message;
    bool signature_valid = false;
    bool document_intact = false;
    bool certificate_trusted = false;
    std::string signer;
};

MessageDigestAlgorithmType parse_digest(const std::string& name) {
    if (name == "md2") {
        return MessageDigestAlgorithmType_MD2;
    }

    if (name == "md4") {
        return MessageDigestAlgorithmType_MD4;
    }

    if (name == "md5") {
        return MessageDigestAlgorithmType_MD5;
    }

    if (name == "sha1") {
        return MessageDigestAlgorithmType_SHA1;
    }

    if (name == "sha224") {
        return MessageDigestAlgorithmType_SHA224;
    }

    if (name == "sha256") {
        return MessageDigestAlgorithmType_SHA256;
    }

    if (name == "sha384") {
        return MessageDigestAlgorithmType_SHA384;
    }

    if (name == "sha512") {
        return MessageDigestAlgorithmType_SHA512;
    }

    if (name == "mdc2") {
        return MessageDigestAlgorithmType_MDC2;
    }

    if (name == "ripemd160") {
        return MessageDigestAlgorithmType_RIPEMD160;
    }

    if (name == "whirlpool") {
        return MessageDigestAlgorithmType_WHIRLPOOL;
    }

    throw PdfException(VANILLAPDF_ERROR_PARAMETER_VALUE, "Unknown digest: " + name);
}

// Set an optional literal-string field on the signature settings when the value
// is non-empty
void set_optional_signature_string(DocumentSignatureSettingsHandle* settings, const std::string& value,
    SignatureSettingsStringSetter setter, const char* context) {
    if (value.empty()) {
        return;
    }

    LiteralStringObjectGuard literal_string;
    THROW_IF_NOT_SUCCESS(LiteralStringObject_CreateFromDecodedString(value.c_str(), literal_string.out()), context);
    THROW_IF_NOT_SUCCESS(setter(settings, literal_string), context);
}

// Copy the contents of a library buffer into a string, or an empty string when
// the buffer cannot be read
std::string buffer_to_string(BufferHandle* buffer) {
    string_type data = nullptr;
    size_type size = 0;
    error_type result = Buffer_GetData(buffer, &data, &size);
    if (result != VANILLAPDF_ERROR_SUCCESS || size == 0) {
        return std::string();
    }

    return std::string(data, size);
}

int process_sign_add(const SignAddOptions& options) {
    DocumentGuard document;
    THROW_IF_NOT_SUCCESS(Document_Open(options.input.c_str(), document.out()), "Failed to open document");

    PKCS12KeyGuard pkcs12_key;
    THROW_IF_NOT_SUCCESS(PKCS12Key_CreateFromFile(options.key_file.c_str(), options.password.c_str(), pkcs12_key.out()),
        "Failed to load PKCS#12 key");

    SigningKeyGuard signing_key;
    THROW_IF_NOT_SUCCESS(PKCS12Key_ToSigningKey(pkcs12_key, signing_key.out()), "Failed to create signing key");

    auto digest = parse_digest(options.digest);

    DocumentSignatureSettingsGuard settings;
    THROW_IF_NOT_SUCCESS(DocumentSignatureSettings_Create(settings.out()), "Failed to create signature settings");
    THROW_IF_NOT_SUCCESS(DocumentSignatureSettings_SetDigest(settings, digest), "Failed to set digest algorithm");
    THROW_IF_NOT_SUCCESS(DocumentSignatureSettings_SetSigningKey(settings, signing_key), "Failed to set signing key");

    set_optional_signature_string(settings, options.name,
        DocumentSignatureSettings_SetName, "Failed to set signer name");
    set_optional_signature_string(settings, options.reason,
        DocumentSignatureSettings_SetReason, "Failed to set reason");
    set_optional_signature_string(settings, options.location,
        DocumentSignatureSettings_SetLocation, "Failed to set location");

    if (!options.no_signing_time) {
        DateGuard current_time;
        THROW_IF_NOT_SUCCESS(Date_CreateCurrent(current_time.out()), "Failed to create signing time");
        THROW_IF_NOT_SUCCESS(DocumentSignatureSettings_SetSigningTime(settings, current_time),
            "Failed to set signing time");
    }

    FileGuard destination_file;
    THROW_IF_NOT_SUCCESS(File_Create(options.output.c_str(), destination_file.out()), "Failed to create output file");

    THROW_IF_NOT_SUCCESS(Document_Sign(document, destination_file, settings), "Failed to sign document");
    std::cerr << "Signed: " << options.output << "\n";

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

// An absent optional signature entry surfaces as OBJECT_MISSING (or
// OPTIONAL_ENTRY_MISSING); either is a normal "not present", not an error
bool is_absent(error_type result) {
    return (result == VANILLAPDF_ERROR_OBJECT_MISSING) || (result == VANILLAPDF_ERROR_OPTIONAL_ENTRY_MISSING);
}

// Read one optional string field. A cleanly-absent entry and an unreadable one
// both yield nullopt so a single bad field never aborts the listing; a genuine
// read failure is surfaced under --verbose.
std::optional<std::string> get_signature_field(DigitalSignatureHandle* signature, const SignatureField& field) {
    StringObjectGuard text;
    error_type result = field.getter(signature, text.out());
    if (result == VANILLAPDF_ERROR_SUCCESS) {
        return to_string_utf8(text);
    }

    if (!is_absent(result) && g_verbose) {
        auto detail = last_error_message();
        std::cerr << "Warning: could not read signature " << field.label << ": " << detail << "\n";
    }

    return std::nullopt;
}

// Format the signature's /M signing time as an ISO 8601 string, or nullopt if
// absent or unreadable
std::optional<std::string> get_signing_time(DigitalSignatureHandle* signature) {
    DateGuard date;
    error_type result = DigitalSignature_GetSigningTime(signature, date.out());
    if (result != VANILLAPDF_ERROR_SUCCESS) {
        if (!is_absent(result) && g_verbose) {
            auto detail = last_error_message();
            std::cerr << "Warning: could not read signing time: " << detail << "\n";
        }

        return std::nullopt;
    }

    integer_type year = 0;
    integer_type month = 0;
    integer_type day = 0;
    integer_type hour = 0;
    integer_type minute = 0;
    integer_type second = 0;
    THROW_IF_NOT_SUCCESS(Date_GetYear(date, &year), "Failed to get signing-time year");
    THROW_IF_NOT_SUCCESS(Date_GetMonth(date, &month), "Failed to get signing-time month");
    THROW_IF_NOT_SUCCESS(Date_GetDay(date, &day), "Failed to get signing-time day");
    THROW_IF_NOT_SUCCESS(Date_GetHour(date, &hour), "Failed to get signing-time hour");
    THROW_IF_NOT_SUCCESS(Date_GetMinute(date, &minute), "Failed to get signing-time minute");
    THROW_IF_NOT_SUCCESS(Date_GetSecond(date, &second), "Failed to get signing-time second");

    auto signing_time = fmt::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}", year, month, day, hour, minute, second);

    TimezoneType timezone = TimezoneType_Undefined;
    THROW_IF_NOT_SUCCESS(Date_GetTimezone(date, &timezone), "Failed to get signing-time timezone");

    if (timezone == TimezoneType_UTC) {
        signing_time += "Z";
    } else if (timezone == TimezoneType_Later || timezone == TimezoneType_Earlier) {
        integer_type hour_offset = 0;
        integer_type minute_offset = 0;
        THROW_IF_NOT_SUCCESS(Date_GetHourOffset(date, &hour_offset), "Failed to get signing-time hour offset");
        THROW_IF_NOT_SUCCESS(Date_GetMinuteOffset(date, &minute_offset), "Failed to get signing-time minute offset");

        char offset_sign = (timezone == TimezoneType_Later) ? '+' : '-';
        signing_time += fmt::format("{}{:02}:{:02}", offset_sign, hour_offset, minute_offset);
    }

    return signing_time;
}

void print_optional_json(const std::optional<std::string>& value) {
    if (value.has_value()) {
        std::cout << json_escape(value.value());
    } else {
        std::cout << "null";
    }
}

void print_list_text(const std::string& file, const std::vector<SignatureListEntry>& entries) {
    std::cout << "File: " << file << "\n";

    for (const auto& entry : entries) {
        std::cout << "Signature #" << entry.index << ":\n";
        if (entry.error.has_value()) {
            std::cout << "    <error: " << entry.error.value() << ">\n";
            continue;
        }

        for (int i = 0; i < SIGNATURE_FIELD_COUNT; i++) {
            if (entry.values[i].has_value()) {
                std::cout << "    " << SIGNATURE_FIELDS[i].label << ": " << entry.values[i].value() << "\n";
            }
        }

        if (entry.signing_time.has_value()) {
            std::cout << "    Signing Time: " << entry.signing_time.value() << "\n";
        }
    }

    std::cout << "Signatures: " << entries.size() << "\n";
}

void print_list_json(const std::string& file, const std::vector<SignatureListEntry>& entries) {
    std::cout << "{\n";
    std::cout << "  \"file\": " << json_escape(file) << ",\n";
    std::cout << "  \"signatures\": [";

    for (size_t i = 0; i < entries.size(); i++) {
        const auto& entry = entries[i];
        if (i > 0) {
            std::cout << ",";
        }

        std::cout << "\n    {\n";
        std::cout << "      \"index\": " << entry.index;

        for (int field_index = 0; field_index < SIGNATURE_FIELD_COUNT; field_index++) {
            std::cout << ",\n      " << json_escape(SIGNATURE_FIELDS[field_index].key) << ": ";
            print_optional_json(entry.values[field_index]);
        }

        std::cout << ",\n      \"signing_time\": ";
        print_optional_json(entry.signing_time);

        std::cout << ",\n      \"error\": ";
        print_optional_json(entry.error);

        std::cout << "\n    }";
    }

    std::cout << "\n  ]\n}\n";
}

// Read the fields of one signature field into entry. Returns false for an
// unsigned signature field (no value), which is not listed. A malformed
// signature is recorded in entry.error rather than aborting the whole listing.
bool read_signature_entry(FieldHandle* field, SignatureListEntry& entry) {
    try {
        SignatureFieldGuard signature_field;
        THROW_IF_NOT_SUCCESS(Field_ToSignature(field, signature_field.out()), "Failed to convert to signature field");

        DigitalSignatureGuard signature;
        error_type signature_result = SignatureField_GetValue(signature_field, signature.out());
        if (signature_result == VANILLAPDF_ERROR_OPTIONAL_ENTRY_MISSING) {
            return false;
        }

        THROW_IF_NOT_SUCCESS(signature_result, "Failed to get signature value");

        for (int i = 0; i < SIGNATURE_FIELD_COUNT; i++) {
            entry.values[i] = get_signature_field(signature, SIGNATURE_FIELDS[i]);
        }

        entry.signing_time = get_signing_time(signature);
    } catch (const PdfException& ex) {
        entry.error = ex.what();
    }

    return true;
}

int process_sign_list(const SignListOptions& options) {
    DocumentGuard document;
    THROW_IF_NOT_SUCCESS(Document_Open(options.input.c_str(), document.out()), "Failed to open document");

    CatalogGuard catalog;
    THROW_IF_NOT_SUCCESS(Document_GetCatalog(document, catalog.out()), "Failed to get catalog");

    std::vector<SignatureListEntry> entries;

    InteractiveFormGuard interactive_form;
    error_type form_result = Catalog_GetAcroForm(catalog, interactive_form.out());
    if (form_result != VANILLAPDF_ERROR_OPTIONAL_ENTRY_MISSING) {
        THROW_IF_NOT_SUCCESS(form_result, "Failed to get interactive form");

        FieldCollectionGuard fields;
        THROW_IF_NOT_SUCCESS(InteractiveForm_GetFields(interactive_form, fields.out()), "Failed to get fields");

        size_type field_count = 0;
        THROW_IF_NOT_SUCCESS(FieldCollection_GetSize(fields, &field_count), "Failed to get field count");

        int signature_index = 0;
        for (size_type i = 0; i < field_count; i++) {
            FieldGuard field;
            THROW_IF_NOT_SUCCESS(FieldCollection_At(fields, i, field.out()), "Failed to get field");

            FieldType field_type = FieldType_Undefined;
            THROW_IF_NOT_SUCCESS(Field_GetType(field, &field_type), "Failed to get field type");

            if (field_type != FieldType_Signature) {
                continue;
            }

            SignatureListEntry entry;
            bool is_signed = read_signature_entry(field, entry);
            if (!is_signed) {
                continue;
            }

            entry.index = ++signature_index;
            entries.push_back(entry);
        }
    }

    if (options.json_output) {
        print_list_json(options.input, entries);
    } else {
        print_list_text(options.input, entries);
    }

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

const char* verification_status_name(SignatureVerificationStatusType status) {
    switch (status) {
        case SignatureStatus_Valid:                  return "Valid";
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

// Read the bytes covered by the signature's /ByteRange from the PDF file
std::vector<char> read_signed_bytes(const std::string& input_path, DigitalSignatureHandle* signature) {
    ByteRangeCollectionGuard byte_ranges;
    THROW_IF_NOT_SUCCESS(DigitalSignature_GetByteRange(signature, byte_ranges.out()), "Failed to get byte range");

    size_type range_count = 0;
    THROW_IF_NOT_SUCCESS(ByteRangeCollection_GetSize(byte_ranges, &range_count), "Failed to get range count");

    std::ifstream pdf_file(input_path, std::ios::binary);
    if (!pdf_file) {
        throw PdfException(VANILLAPDF_ERROR_GENERAL, "Failed to open file for byte range extraction");
    }

    std::vector<char> signed_bytes;

    for (size_type i = 0; i < range_count; i++) {
        ByteRangeGuard byte_range;
        THROW_IF_NOT_SUCCESS(ByteRangeCollection_GetValue(byte_ranges, i, byte_range.out()),
            "Failed to get byte range entry");

        IntegerObjectGuard offset_object;
        IntegerObjectGuard length_object;
        THROW_IF_NOT_SUCCESS(ByteRange_GetOffset(byte_range, offset_object.out()), "Failed to get range offset");
        THROW_IF_NOT_SUCCESS(ByteRange_GetLength(byte_range, length_object.out()), "Failed to get range length");

        bigint_type offset = 0;
        bigint_type length = 0;
        THROW_IF_NOT_SUCCESS(IntegerObject_GetIntegerValue(offset_object, &offset), "Failed to get offset value");
        THROW_IF_NOT_SUCCESS(IntegerObject_GetIntegerValue(length_object, &length), "Failed to get length value");

        auto offset_converted = static_cast<std::streamoff>(offset);
        auto length_converted = static_cast<size_t>(length);

        std::vector<char> chunk(length_converted);
        pdf_file.seekg(offset_converted);
        pdf_file.read(chunk.data(), static_cast<std::streamsize>(length_converted));
        signed_bytes.insert(signed_bytes.end(), chunk.begin(), chunk.end());
    }

    return signed_bytes;
}

bool read_verification_flag(SignatureVerificationResultHandle* verification_result,
    error_type (*getter)(SignatureVerificationResultHandle*, boolean_type*), const char* context) {
    boolean_type flag_value = VANILLAPDF_RV_FALSE;
    THROW_IF_NOT_SUCCESS(getter(verification_result, &flag_value), context);
    return (flag_value != VANILLAPDF_RV_FALSE);
}

SignatureVerifyEntry verify_signature(const std::string& input_path, DigitalSignatureHandle* signature,
    TrustedCertificateStoreHandle* certificate_store, SignatureVerificationSettingsHandle* settings) {
    auto signed_bytes = read_signed_bytes(input_path, signature);

    BufferGuard signed_data_buffer;
    THROW_IF_NOT_SUCCESS(Buffer_CreateFromData(signed_bytes.data(), signed_bytes.size(), signed_data_buffer.out()),
        "Failed to create signed data buffer");

    HexadecimalStringObjectGuard signature_contents;
    THROW_IF_NOT_SUCCESS(DigitalSignature_GetContents(signature, signature_contents.out()),
        "Failed to get signature contents");

    BufferGuard signature_buffer;
    THROW_IF_NOT_SUCCESS(HexadecimalStringObject_GetValue(signature_contents, signature_buffer.out()),
        "Failed to get signature bytes");

    SignatureVerificationResultGuard verification_result;
    THROW_IF_NOT_SUCCESS(SignatureVerifier_Verify(signed_data_buffer, signature_buffer, certificate_store, settings,
        verification_result.out()), "Failed to verify signature");

    SignatureVerifyEntry entry;
    THROW_IF_NOT_SUCCESS(SignatureVerificationResult_GetStatus(verification_result, &entry.status),
        "Failed to get verification status");

    BufferGuard message_buffer;
    error_type message_result = SignatureVerificationResult_GetMessage(verification_result, message_buffer.out());
    if (message_result == VANILLAPDF_ERROR_SUCCESS) {
        entry.message = buffer_to_string(message_buffer);
    }

    entry.signature_valid = read_verification_flag(verification_result,
        SignatureVerificationResult_IsSignatureValid, "Failed to check signature validity");
    entry.document_intact = read_verification_flag(verification_result,
        SignatureVerificationResult_IsDocumentIntact, "Failed to check document integrity");
    entry.certificate_trusted = read_verification_flag(verification_result,
        SignatureVerificationResult_IsCertificateTrusted, "Failed to check certificate trust");

    BufferGuard signer_buffer;
    error_type signer_result = SignatureVerificationResult_GetSignerCommonName(verification_result,
        signer_buffer.out());
    if (signer_result == VANILLAPDF_ERROR_SUCCESS) {
        entry.signer = buffer_to_string(signer_buffer);
    }

    return entry;
}

std::vector<SignatureVerifyEntry> collect_verify_entries(const std::string& input_path,
    TrustedCertificateStoreHandle* certificate_store, SignatureVerificationSettingsHandle* settings) {
    std::vector<SignatureVerifyEntry> entries;

    DocumentGuard document;
    THROW_IF_NOT_SUCCESS(Document_Open(input_path.c_str(), document.out()), "Failed to open document");

    CatalogGuard catalog;
    THROW_IF_NOT_SUCCESS(Document_GetCatalog(document, catalog.out()), "Failed to get catalog");

    InteractiveFormGuard interactive_form;
    error_type form_result = Catalog_GetAcroForm(catalog, interactive_form.out());
    if (form_result == VANILLAPDF_ERROR_OPTIONAL_ENTRY_MISSING) {
        return entries;
    }

    THROW_IF_NOT_SUCCESS(form_result, "Failed to get interactive form");

    FieldCollectionGuard fields;
    THROW_IF_NOT_SUCCESS(InteractiveForm_GetFields(interactive_form, fields.out()), "Failed to get fields");

    size_type field_count = 0;
    THROW_IF_NOT_SUCCESS(FieldCollection_GetSize(fields, &field_count), "Failed to get field count");

    int signature_index = 0;

    for (size_type i = 0; i < field_count; i++) {
        FieldGuard field;
        THROW_IF_NOT_SUCCESS(FieldCollection_At(fields, i, field.out()), "Failed to get field");

        FieldType field_type = FieldType_Undefined;
        THROW_IF_NOT_SUCCESS(Field_GetType(field, &field_type), "Failed to get field type");

        if (field_type != FieldType_Signature) {
            continue;
        }

        SignatureFieldGuard signature_field;
        THROW_IF_NOT_SUCCESS(Field_ToSignature(field, signature_field.out()), "Failed to convert to signature field");

        DigitalSignatureGuard signature;
        error_type signature_result = SignatureField_GetValue(signature_field, signature.out());
        if (signature_result == VANILLAPDF_ERROR_OPTIONAL_ENTRY_MISSING) {
            continue;
        }

        THROW_IF_NOT_SUCCESS(signature_result, "Failed to get signature value");

        auto entry = verify_signature(input_path, signature, certificate_store, settings);
        entry.index = ++signature_index;
        entries.push_back(entry);
    }

    return entries;
}

void print_verify_text(const std::string& file, const std::vector<SignatureVerifyEntry>& entries) {
    std::cout << "File: " << file << "\n";
    if (entries.empty()) {
        std::cout << "No signatures found\n";
        return;
    }

    for (const auto& entry : entries) {
        std::cout << "Signature #" << entry.index << ":\n";
        std::cout << "  Status: " << verification_status_name(entry.status) << "\n";
        if (!entry.message.empty()) {
            std::cout << "  Message: " << entry.message << "\n";
        }

        std::cout << "  Signature valid: " << (entry.signature_valid ? "yes" : "no") << "\n";
        std::cout << "  Document intact: " << (entry.document_intact ? "yes" : "no") << "\n";
        std::cout << "  Certificate trusted: " << (entry.certificate_trusted ? "yes" : "no") << "\n";
        if (!entry.signer.empty()) {
            std::cout << "  Signer: " << entry.signer << "\n";
        }
    }
}

void print_verify_json(const std::string& file, const std::vector<SignatureVerifyEntry>& entries) {
    std::cout << "{\n";
    std::cout << "  \"file\": " << json_escape(file) << ",\n";
    std::cout << "  \"signatures\": [";

    for (size_t i = 0; i < entries.size(); i++) {
        const auto& entry = entries[i];
        if (i > 0) {
            std::cout << ",";
        }

        auto status = verification_status_name(entry.status);

        std::cout << "\n    {\n";
        std::cout << "      \"index\": " << entry.index << ",\n";
        std::cout << "      \"status\": " << json_escape(status) << ",\n";
        if (!entry.message.empty()) {
            std::cout << "      \"message\": " << json_escape(entry.message) << ",\n";
        }

        std::cout << "      \"signature_valid\": " << (entry.signature_valid ? "true" : "false") << ",\n";
        std::cout << "      \"document_intact\": " << (entry.document_intact ? "true" : "false") << ",\n";
        std::cout << "      \"certificate_trusted\": " << (entry.certificate_trusted ? "true" : "false");
        if (!entry.signer.empty()) {
            std::cout << ",\n      \"signer\": " << json_escape(entry.signer);
        }

        std::cout << "\n    }";
    }

    std::cout << "\n  ]\n}\n";
}

void add_trusted_certificate(TrustedCertificateStoreHandle* certificate_store, const std::string& certificate_path) {
    std::ifstream certificate_file(certificate_path, std::ios::binary);
    if (!certificate_file) {
        throw PdfException(VANILLAPDF_ERROR_GENERAL, "Failed to open certificate file: " + certificate_path);
    }

    std::istreambuf_iterator<char> file_begin(certificate_file);
    std::istreambuf_iterator<char> file_end;
    std::string pem_data(file_begin, file_end);

    BufferGuard certificate_buffer;
    THROW_IF_NOT_SUCCESS(Buffer_CreateFromData(pem_data.data(), pem_data.size(), certificate_buffer.out()),
        "Failed to create certificate buffer");
    THROW_IF_NOT_SUCCESS(TrustedCertificateStore_AddCertificateFromPEM(certificate_store, certificate_buffer),
        "Failed to add certificate from: " + certificate_path);
}

int process_sign_verify(const SignVerifyOptions& options) {
    TrustedCertificateStoreGuard certificate_store;
    THROW_IF_NOT_SUCCESS(TrustedCertificateStore_Create(certificate_store.out()), "Failed to create certificate store");

    if (options.load_system_certificates) {
        THROW_IF_NOT_SUCCESS(TrustedCertificateStore_LoadSystemDefaults(certificate_store),
            "Failed to load system certificates");
    }

    for (const auto& certificate_path : options.trusted_certificates) {
        add_trusted_certificate(certificate_store, certificate_path);
    }

    SignatureVerificationSettingsGuard settings;
    THROW_IF_NOT_SUCCESS(SignatureVerificationSettings_Create(settings.out()),
        "Failed to create verification settings");

    if (options.skip_certificate_validation) {
        THROW_IF_NOT_SUCCESS(SignatureVerificationSettings_SetSkipCertificateValidation(settings, VANILLAPDF_RV_TRUE),
            "Failed to set skip certificate validation");
    }

    if (options.check_signing_time) {
        THROW_IF_NOT_SUCCESS(SignatureVerificationSettings_SetCheckSigningTimeFlag(settings, VANILLAPDF_RV_TRUE),
            "Failed to set check signing time");
    }

    if (options.allow_weak_algorithms) {
        THROW_IF_NOT_SUCCESS(SignatureVerificationSettings_SetAllowWeakAlgorithmsFlag(settings, VANILLAPDF_RV_TRUE),
            "Failed to set allow weak algorithms");
    }

    auto entries = collect_verify_entries(options.input, certificate_store, settings);

    if (options.json_output) {
        print_verify_json(options.input, entries);
    } else {
        print_verify_text(options.input, entries);
    }

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

} // namespace

void register_sign(CLI::App& app, int& exit_code) {
    auto* command = app.add_subcommand("sign", "Manage PDF digital signatures");
    command->require_subcommand(1);

    // sign add
    auto add_options = std::make_shared<SignAddOptions>();
    auto* add_command = command->add_subcommand("add", "Add a digital signature to a PDF document");

    add_command->add_option("input", add_options->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    add_command->add_option("-o,--output", add_options->output, "Output PDF file")
        ->required();
    add_command->add_option("-k,--key", add_options->key_file, "PKCS#12 key file (.p12/.pfx)")
        ->required()
        ->check(CLI::ExistingFile);
    add_command->add_option("-p,--password", add_options->password, "Key file password");
    add_command->add_option("--digest", add_options->digest, "Message digest algorithm for signing (default: sha256)")
        ->check(CLI::IsMember({"md2", "md4", "md5", "sha1", "sha224", "sha256",
            "sha384", "sha512", "mdc2", "ripemd160", "whirlpool"}));
    add_command->add_option("--name", add_options->name, "Signer name recorded in the signature");
    add_command->add_option("--reason", add_options->reason, "Reason for signing");
    add_command->add_option("--location", add_options->location, "Location of signing");
    add_command->add_flag("--no-signing-time", add_options->no_signing_time,
        "Do not record a signing time (recorded as the current time by default)");

    add_command->callback([add_options, &exit_code]() {
        exit_code = process_sign_add(*add_options);
    });

    // sign list
    auto list_options = std::make_shared<SignListOptions>();
    auto* list_command = command->add_subcommand("list", "List digital signatures in a PDF document");

    list_command->add_option("input", list_options->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    list_command->add_flag("--json", list_options->json_output, "Output results as JSON");

    list_command->callback([list_options, &exit_code]() {
        exit_code = process_sign_list(*list_options);
    });

    // sign verify
    auto verify_options = std::make_shared<SignVerifyOptions>();
    auto* verify_command = command->add_subcommand("verify", "Verify digital signatures in a PDF document");

    verify_command->add_option("input", verify_options->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    verify_command->add_option("-c,--trusted-cert", verify_options->trusted_certificates,
        "PEM certificate file to add to the trust store (repeatable)")
        ->check(CLI::ExistingFile);
    verify_command->add_flag("!--no-system-certs", verify_options->load_system_certificates,
        "Do not load system default trusted certificates");
    verify_command->add_flag("--skip-cert-validation", verify_options->skip_certificate_validation,
        "Skip certificate chain validation (cryptographic check only)");
    verify_command->add_flag("--check-signing-time", verify_options->check_signing_time,
        "Verify that the certificate was valid at the time of signing");
    verify_command->add_flag("--allow-weak", verify_options->allow_weak_algorithms,
        "Allow weak digest algorithms (MD5, SHA-1, RSA < 2048)");
    verify_command->add_flag("--json", verify_options->json_output, "Output results as JSON");

    verify_command->callback([verify_options, &exit_code]() {
        exit_code = process_sign_verify(*verify_options);
    });
}

} // cli
} // vanillapdf
