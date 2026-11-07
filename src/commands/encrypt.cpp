#include "commands/commands.h"
#include "utils/error.h"
#include "utils/handle_guard.h"

#include <CLI/CLI.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace vanillapdf {
namespace cli {

namespace {

struct EncryptAddOptions {
    std::string input;
    std::string output;
    std::string user_password;
    std::string owner_password;
    std::string algorithm = "aes";
    int bits = 0;  // 0 = auto: 256 for AES, 128 for RC4
    std::vector<std::string> allow;
};

struct EncryptRemoveOptions {
    std::string input;
    std::string output;
    std::string password;
};

const int ALL_PERMISSIONS =
    UserAccessPermissionFlag_PrintDegraded
    | UserAccessPermissionFlag_ModifyContents
    | UserAccessPermissionFlag_CopyAndExtract
    | UserAccessPermissionFlag_AddAnnotations
    | UserAccessPermissionFlag_FillForms
    | UserAccessPermissionFlag_ExtractText
    | UserAccessPermissionFlag_AssembleDocument
    | UserAccessPermissionFlag_PrintFaithful;

// Map a permission token to its flags
int parse_permission(const std::string& token) {
    if (token == "none") {
        return UserAccessPermissionFlag_None;
    }

    if (token == "all") {
        return ALL_PERMISSIONS;
    }

    if (token == "print") {
        return UserAccessPermissionFlag_PrintDegraded;
    }

    // High-quality print also requires the base print bit (PDF 32000 Table 22)
    if (token == "print-high") {
        return UserAccessPermissionFlag_PrintFaithful | UserAccessPermissionFlag_PrintDegraded;
    }

    if (token == "modify") {
        return UserAccessPermissionFlag_ModifyContents;
    }

    if (token == "copy") {
        return UserAccessPermissionFlag_CopyAndExtract;
    }

    if (token == "annotate") {
        return UserAccessPermissionFlag_AddAnnotations;
    }

    if (token == "fill-forms") {
        return UserAccessPermissionFlag_FillForms;
    }

    if (token == "extract") {
        return UserAccessPermissionFlag_ExtractText;
    }

    if (token == "assemble") {
        return UserAccessPermissionFlag_AssembleDocument;
    }

    throw PdfException(VANILLAPDF_ERROR_PARAMETER_VALUE, "Unknown permission: " + token);
}

UserAccessPermissionFlags parse_permissions(const std::vector<std::string>& tokens) {
    int permission_mask = UserAccessPermissionFlag_None;
    for (const auto& token : tokens) {
        permission_mask |= parse_permission(token);
    }

    return static_cast<UserAccessPermissionFlags>(permission_mask);
}

int process_encrypt_add(const EncryptAddOptions& options) {
    // Resolve algorithm and validate the key length against it.
    // AES supports 128/256; RC4 supports 40/128. Default: AES-256, RC4-128.
    bool is_aes = (options.algorithm == "aes");
    int default_bits = is_aes ? 256 : 128;
    int key_length = (options.bits != 0) ? options.bits : default_bits;

    if (is_aes && key_length != 128 && key_length != 256) {
        throw PdfException(VANILLAPDF_ERROR_PARAMETER_VALUE, "AES supports only 128 or 256-bit keys");
    }

    if (!is_aes && key_length != 40 && key_length != 128) {
        throw PdfException(VANILLAPDF_ERROR_PARAMETER_VALUE, "RC4 supports only 40 or 128-bit keys");
    }

    if (!is_aes) {
        std::cerr << "Warning: RC4 is a legacy, insecure cipher; prefer AES.\n";
    }

    DocumentGuard document;
    THROW_IF_NOT_SUCCESS(Document_Open(options.input.c_str(), document.out()), "Failed to open document");

    DocumentEncryptionSettingsGuard settings;
    THROW_IF_NOT_SUCCESS(DocumentEncryptionSettings_Create(settings.out()), "Failed to create encryption settings");

    auto algorithm = is_aes ? EncryptionAlgorithmType_AES : EncryptionAlgorithmType_RC4;
    THROW_IF_NOT_SUCCESS(DocumentEncryptionSettings_SetAlgorithm(settings, algorithm),
        "Failed to set encryption algorithm");
    THROW_IF_NOT_SUCCESS(DocumentEncryptionSettings_SetKeyLength(settings, key_length), "Failed to set key length");

    if (!options.allow.empty()) {
        auto permissions = parse_permissions(options.allow);
        THROW_IF_NOT_SUCCESS(DocumentEncryptionSettings_SetUserAccessPermissions(settings, permissions),
            "Failed to set access permissions");
    }

    auto user_password_data = options.user_password.c_str();
    auto user_password_size = static_cast<size_type>(options.user_password.size());

    BufferGuard user_password_buffer;
    THROW_IF_NOT_SUCCESS(Buffer_CreateFromData(user_password_data, user_password_size, user_password_buffer.out()),
        "Failed to create user password buffer");
    THROW_IF_NOT_SUCCESS(DocumentEncryptionSettings_SetUserPassword(settings, user_password_buffer),
        "Failed to set user password");

    if (!options.owner_password.empty()) {
        auto owner_password_data = options.owner_password.c_str();
        auto owner_password_size = static_cast<size_type>(options.owner_password.size());

        BufferGuard owner_password_buffer;
        THROW_IF_NOT_SUCCESS(Buffer_CreateFromData(owner_password_data, owner_password_size, owner_password_buffer.out()),
            "Failed to create owner password buffer");
        THROW_IF_NOT_SUCCESS(DocumentEncryptionSettings_SetOwnerPassword(settings, owner_password_buffer),
            "Failed to set owner password");
    }

    THROW_IF_NOT_SUCCESS(Document_AddEncryption(document, settings), "Failed to add encryption");
    THROW_IF_NOT_SUCCESS(Document_Save(document, options.output.c_str()), "Failed to save encrypted document");

    const char* algorithm_name = is_aes ? "AES-" : "RC4-";
    std::cerr << "Encrypted (" << algorithm_name << key_length << "): " << options.output << "\n";

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

int process_encrypt_remove(const EncryptRemoveOptions& options) {
    FileGuard file;
    THROW_IF_NOT_SUCCESS(File_Open(options.input.c_str(), file.out()), "Failed to open file");

    // The password must be set after the file is initialized (via Document_OpenFile),
    // matching the library's own decrypt tool ordering
    DocumentGuard document;
    THROW_IF_NOT_SUCCESS(Document_OpenFile(file, document.out()), "Failed to open document");
    THROW_IF_NOT_SUCCESS(File_SetEncryptionPassword(file, options.password.c_str()), "Failed to set password");
    THROW_IF_NOT_SUCCESS(Document_RemoveEncryption(document), "Failed to remove encryption");
    THROW_IF_NOT_SUCCESS(Document_Save(document, options.output.c_str()), "Failed to save decrypted document");

    std::cerr << "Decrypted: " << options.output << "\n";

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

} // namespace

void register_encrypt(CLI::App& app, int& exit_code) {
    auto* command = app.add_subcommand("encrypt", "Manage PDF encryption");
    command->require_subcommand(1);

    // encrypt add
    auto add_options = std::make_shared<EncryptAddOptions>();
    auto* add_command = command->add_subcommand("add", "Add password encryption to a PDF document");

    add_command->add_option("input", add_options->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    add_command->add_option("-o,--output", add_options->output, "Output PDF file")
        ->required();
    add_command->add_option("-p,--user-password", add_options->user_password, "User password (required to open)")
        ->required();
    add_command->add_option("--owner-password", add_options->owner_password, "Owner password (required to edit)");
    add_command->add_option("--algorithm", add_options->algorithm, "Cipher: aes (recommended) or rc4 (legacy)")
        ->check(CLI::IsMember({"aes", "rc4"}));
    add_command->add_option("--bits", add_options->bits,
        "Key length in bits (AES: 128/256, RC4: 40/128; default AES-256, RC4-128)")
        ->check(CLI::IsMember({40, 128, 256}));
    add_command->add_option("--allow", add_options->allow,
        "Comma-separated permissions to grant; omit to allow all. "
        "Values: none all print print-high modify copy annotate fill-forms extract assemble")
        ->delimiter(',')
        ->check(CLI::IsMember({"none", "all", "print", "print-high", "modify", "copy",
            "annotate", "fill-forms", "extract", "assemble"}));

    add_command->callback([add_options, &exit_code]() {
        exit_code = process_encrypt_add(*add_options);
    });

    // encrypt remove
    auto remove_options = std::make_shared<EncryptRemoveOptions>();
    auto* remove_command = command->add_subcommand("remove", "Remove encryption from a PDF document");

    remove_command->add_option("input", remove_options->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    remove_command->add_option("-o,--output", remove_options->output, "Output PDF file")
        ->required();
    remove_command->add_option("-p,--password", remove_options->password, "Document password")
        ->required();

    remove_command->callback([remove_options, &exit_code]() {
        exit_code = process_encrypt_remove(*remove_options);
    });
}

} // cli
} // vanillapdf
