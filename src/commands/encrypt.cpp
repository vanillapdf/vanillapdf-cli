#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "commands/commands.hpp"
#include "utils/handles.hpp"

namespace vanillapdf::cli::commands {

struct EncryptAddOptions {
    std::string input;
    std::string output;
    std::string user_password;
    std::string owner_password;
    std::string algorithm = "aes";
    int bits = 0;  // 0 = auto: 256 for AES, 128 for RC4
    std::vector<std::string> allow;
};

// Map a permission token to its flag. "all" and "none" are handled by the caller.
static UserAccessPermissionFlags parse_permissions(const std::vector<std::string>& tokens) {
    int mask = UserAccessPermissionFlag_None;
    for (const auto& t : tokens) {
        if (t == "none")            mask |= UserAccessPermissionFlag_None;
        else if (t == "all")        mask |= UserAccessPermissionFlag_PrintDegraded
                                          | UserAccessPermissionFlag_ModifyContents
                                          | UserAccessPermissionFlag_CopyAndExtract
                                          | UserAccessPermissionFlag_AddAnnotations
                                          | UserAccessPermissionFlag_FillForms
                                          | UserAccessPermissionFlag_ExtractText
                                          | UserAccessPermissionFlag_AssembleDocument
                                          | UserAccessPermissionFlag_PrintFaithful;
        else if (t == "print")      mask |= UserAccessPermissionFlag_PrintDegraded;
        // High-quality print also requires the base print bit (PDF 32000 Table 22).
        else if (t == "print-high") mask |= UserAccessPermissionFlag_PrintFaithful
                                          | UserAccessPermissionFlag_PrintDegraded;
        else if (t == "modify")     mask |= UserAccessPermissionFlag_ModifyContents;
        else if (t == "copy")       mask |= UserAccessPermissionFlag_CopyAndExtract;
        else if (t == "annotate")   mask |= UserAccessPermissionFlag_AddAnnotations;
        else if (t == "fill-forms") mask |= UserAccessPermissionFlag_FillForms;
        else if (t == "extract")    mask |= UserAccessPermissionFlag_ExtractText;
        else if (t == "assemble")   mask |= UserAccessPermissionFlag_AssembleDocument;
        else throw pdf_error(VANILLAPDF_ERROR_PARAMETER_VALUE, "Unknown permission: " + t);
    }
    return static_cast<UserAccessPermissionFlags>(mask);
}

struct EncryptRemoveOptions {
    std::string input;
    std::string output;
    std::string password;
};

static int run_encrypt_add(const EncryptAddOptions& opts) {
    // Resolve algorithm and validate the key length against it.
    // AES supports 128/256; RC4 supports 40/128. Default: AES-256, RC4-128.
    const bool is_aes = (opts.algorithm == "aes");
    const int bits = (opts.bits != 0) ? opts.bits : (is_aes ? 256 : 128);

    if (is_aes && bits != 128 && bits != 256) {
        throw pdf_error(VANILLAPDF_ERROR_PARAMETER_VALUE, "AES supports only 128 or 256-bit keys");
    }
    if (!is_aes && bits != 40 && bits != 128) {
        throw pdf_error(VANILLAPDF_ERROR_PARAMETER_VALUE, "RC4 supports only 40 or 128-bit keys");
    }
    if (!is_aes) {
        std::cerr << "Warning: RC4 is a legacy, insecure cipher; prefer AES.\n";
    }

    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    DocumentEncryptionSettingsPtr settings;
    check(DocumentEncryptionSettings_Create(settings.put()), "Failed to create encryption settings");

    check(DocumentEncryptionSettings_SetAlgorithm(settings.get(),
              is_aes ? EncryptionAlgorithmType_AES : EncryptionAlgorithmType_RC4),
          "Failed to set encryption algorithm");
    check(DocumentEncryptionSettings_SetKeyLength(settings.get(), bits),
          "Failed to set key length");

    if (!opts.allow.empty()) {
        check(DocumentEncryptionSettings_SetUserAccessPermissions(settings.get(),
                                                                  parse_permissions(opts.allow)),
              "Failed to set access permissions");
    }

    BufferPtr user_buf;
    check(Buffer_CreateFromData(opts.user_password.c_str(),
                                static_cast<size_type>(opts.user_password.size()),
                                user_buf.put()),
          "Failed to create user password buffer");
    check(DocumentEncryptionSettings_SetUserPassword(settings.get(), user_buf.get()),
          "Failed to set user password");

    if (!opts.owner_password.empty()) {
        BufferPtr owner_buf;
        check(Buffer_CreateFromData(opts.owner_password.c_str(),
                                    static_cast<size_type>(opts.owner_password.size()),
                                    owner_buf.put()),
              "Failed to create owner password buffer");
        check(DocumentEncryptionSettings_SetOwnerPassword(settings.get(), owner_buf.get()),
              "Failed to set owner password");
    }

    check(Document_AddEncryption(document.get(), settings.get()), "Failed to add encryption");
    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save encrypted document");
    std::cerr << "Encrypted (" << (is_aes ? "AES-" : "RC4-") << bits << "): " << opts.output << "\n";

    return 0;
}

static int run_encrypt_remove(const EncryptRemoveOptions& opts) {
    FilePtr file;
    check(File_Open(opts.input.c_str(), file.put()), "Failed to open file");

    // The password must be set after the file is initialized (via Document_OpenFile),
    // matching the library's own decrypt tool ordering.
    DocumentPtr document;
    check(Document_OpenFile(file.get(), document.put()), "Failed to open document");
    check(File_SetEncryptionPassword(file.get(), opts.password.c_str()), "Failed to set password");
    check(Document_RemoveEncryption(document.get()), "Failed to remove encryption");
    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save decrypted document");
    std::cerr << "Decrypted: " << opts.output << "\n";

    return 0;
}

void register_encrypt(CLI::App& app) {
    auto* cmd = app.add_subcommand("encrypt", "Manage PDF encryption");
    cmd->require_subcommand(1);

    // encrypt add
    auto add_opts = std::make_shared<EncryptAddOptions>();
    auto* add_cmd = cmd->add_subcommand("add", "Add password encryption to a PDF document");
    add_cmd->add_option("input", add_opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    add_cmd->add_option("-o,--output", add_opts->output, "Output PDF file")
        ->required();
    add_cmd->add_option("-p,--user-password", add_opts->user_password, "User password (required to open)")
        ->required();
    add_cmd->add_option("--owner-password", add_opts->owner_password, "Owner password (required to edit)");
    add_cmd->add_option("--algorithm", add_opts->algorithm, "Cipher: aes (recommended) or rc4 (legacy)")
        ->check(CLI::IsMember({"aes", "rc4"}));
    add_cmd->add_option("--bits", add_opts->bits,
                        "Key length in bits (AES: 128/256, RC4: 40/128; default AES-256, RC4-128)")
        ->check(CLI::IsMember({40, 128, 256}));
    add_cmd->add_option("--allow", add_opts->allow,
                        "Comma-separated permissions to grant; omit to allow all. "
                        "Values: none all print print-high modify copy annotate fill-forms extract assemble")
        ->delimiter(',')
        ->check(CLI::IsMember({"none", "all", "print", "print-high", "modify", "copy",
                               "annotate", "fill-forms", "extract", "assemble"}));

    add_cmd->callback([add_opts]() {
        try {
            std::exit(run_encrypt_add(*add_opts));
        } catch (const pdf_error& e) {
            report_error(e);
            std::exit(1);
        }
    });

    // encrypt remove
    auto rm_opts = std::make_shared<EncryptRemoveOptions>();
    auto* rm_cmd = cmd->add_subcommand("remove", "Remove encryption from a PDF document");
    rm_cmd->add_option("input", rm_opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    rm_cmd->add_option("-o,--output", rm_opts->output, "Output PDF file")
        ->required();
    rm_cmd->add_option("-p,--password", rm_opts->password, "Document password")
        ->required();

    rm_cmd->callback([rm_opts]() {
        try {
            std::exit(run_encrypt_remove(*rm_opts));
        } catch (const pdf_error& e) {
            report_error(e);
            std::exit(1);
        }
    });
}

} // namespace vanillapdf::cli::commands
