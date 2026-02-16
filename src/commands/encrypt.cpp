#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "commands/commands.hpp"
#include "utils/error.hpp"

namespace vpdf::commands {

struct EncryptAddOptions {
    std::string input;
    std::string output;
    std::string user_password;
    std::string owner_password;
};

struct EncryptRemoveOptions {
    std::string input;
    std::string output;
    std::string password;
};

static int run_encrypt_add(const EncryptAddOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    DocumentEncryptionSettingsPtr settings;
    check(DocumentEncryptionSettings_Create(settings.put()), "Failed to create encryption settings");

    check(DocumentEncryptionSettings_SetAlgorithm(settings.get(), EncryptionAlgorithmType_RC4),
          "Failed to set encryption algorithm");
    check(DocumentEncryptionSettings_SetKeyLength(settings.get(), 128),
          "Failed to set key length");

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
    std::cerr << "Encrypted: " << opts.output << "\n";

    return 0;
}

static int run_encrypt_remove(const EncryptRemoveOptions& opts) {
    FilePtr file;
    check(File_Open(opts.input.c_str(), file.put()), "Failed to open file");
    check(File_SetEncryptionPassword(file.get(), opts.password.c_str()), "Failed to set password");
    check(File_Initialize(file.get()), "Failed to initialize file");

    DocumentPtr document;
    check(Document_OpenFile(file.get(), document.put()), "Failed to open document");
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

    add_cmd->callback([add_opts]() {
        try {
            std::exit(run_encrypt_add(*add_opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
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
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
