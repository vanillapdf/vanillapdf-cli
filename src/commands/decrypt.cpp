#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "commands/commands.hpp"
#include "utils/error.hpp"

namespace vpdf::commands {

struct DecryptOptions {
    std::string input;
    std::string output;
    std::string password;
};

static int run_decrypt(const DecryptOptions& opts) {
    // TODO: implement decryption using vanillapdf standard security API
    // 1. Open encrypted document with password
    // 2. Save decrypted document

    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save decrypted document");
    std::cerr << "Decrypted: " << opts.output << "\n";

    return 0;
}

void register_decrypt(CLI::App& app) {
    auto opts = std::make_shared<DecryptOptions>();

    auto* cmd = app.add_subcommand("decrypt", "Decrypt a password-protected PDF document");
    cmd->add_option("input", opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    cmd->add_option("-o,--output", opts->output, "Output PDF file")
        ->required();
    cmd->add_option("-p,--password", opts->password, "Document password")
        ->required();

    cmd->callback([opts]() {
        try {
            std::exit(run_decrypt(*opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
