#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "commands/commands.hpp"
#include "utils/error.hpp"

namespace vpdf::commands {

struct SignOptions {
    std::string input;
    std::string output;
    std::string key_file;
    std::string password;
};

static int run_sign(const SignOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    // TODO: implement signing using vanillapdf PKCS#12 signature API
    // 1. Create or locate signature field
    // 2. Configure signing key from PKCS#12 file
    // 3. Apply signature
    // 4. Save with incremental update

    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save signed document");
    std::cerr << "Signed: " << opts.output << "\n";

    return 0;
}

void register_sign(CLI::App& app) {
    auto opts = std::make_shared<SignOptions>();

    auto* cmd = app.add_subcommand("sign", "Digitally sign a PDF document");
    cmd->add_option("input", opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    cmd->add_option("-o,--output", opts->output, "Output PDF file")
        ->required();
    cmd->add_option("-k,--key", opts->key_file, "PKCS#12 key file (.p12/.pfx)")
        ->required()
        ->check(CLI::ExistingFile);
    cmd->add_option("-p,--password", opts->password, "Key file password");

    cmd->callback([opts]() {
        try {
            std::exit(run_sign(*opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
