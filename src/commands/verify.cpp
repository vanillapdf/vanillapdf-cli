#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "commands/commands.hpp"
#include "utils/error.hpp"

namespace vpdf::commands {

struct VerifyOptions {
    std::string input;
};

static int run_verify(const VerifyOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    // TODO: implement signature verification using vanillapdf API
    // 1. Enumerate signature fields
    // 2. For each signature, verify integrity and validity
    // 3. Report verification status

    std::cout << "File: " << opts.input << "\n";
    std::cout << "Signature verification not yet implemented\n";

    return 0;
}

void register_verify(CLI::App& app) {
    auto opts = std::make_shared<VerifyOptions>();

    auto* cmd = app.add_subcommand("verify", "Verify PDF digital signatures");
    cmd->add_option("input", opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);

    cmd->callback([opts]() {
        try {
            std::exit(run_verify(*opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
