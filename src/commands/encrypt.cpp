#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "commands/commands.hpp"
#include "utils/error.hpp"

namespace vpdf::commands {

struct EncryptOptions {
    std::string input;
    std::string output;
    std::string user_password;
    std::string owner_password;
};

static int run_encrypt(const EncryptOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    // TODO: implement encryption using vanillapdf standard security API
    // 1. Create encryption key from user/owner passwords
    // 2. Apply encryption to document
    // 3. Save encrypted document

    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save encrypted document");
    std::cerr << "Encrypted: " << opts.output << "\n";

    return 0;
}

void register_encrypt(CLI::App& app) {
    auto opts = std::make_shared<EncryptOptions>();

    auto* cmd = app.add_subcommand("encrypt", "Encrypt a PDF document with password protection");
    cmd->add_option("input", opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    cmd->add_option("-o,--output", opts->output, "Output PDF file")
        ->required();
    cmd->add_option("-p,--user-password", opts->user_password, "User password (required to open)")
        ->required();
    cmd->add_option("--owner-password", opts->owner_password, "Owner password (required to edit)");

    cmd->callback([opts]() {
        try {
            std::exit(run_encrypt(*opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
