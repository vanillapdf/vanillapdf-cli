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

    // TODO: implement signing using vanillapdf PKCS#12 signature API
    // 1. Create or locate signature field
    // 2. Configure signing key from PKCS#12 file
    // 3. Apply signature
    // 4. Save with incremental update

    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save signed document");
    std::cerr << "Signed: " << opts.output << "\n";

    return 0;
}

static int run_sign_list(const SignListOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    // TODO: implement signature enumeration using vanillapdf API
    // 1. Enumerate signature fields
    // 2. For each signature, report signer, timestamp, validity

    std::cout << "File: " << opts.input << "\n";
    std::cout << "Signature listing not yet implemented\n";

    return 0;
}

static int run_sign_remove(const SignRemoveOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    // TODO: implement signature removal using vanillapdf API
    // 1. Locate signature fields
    // 2. Remove signatures
    // 3. Save document

    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save document");
    std::cerr << "Signatures removed: " << opts.output << "\n";

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
}

} // namespace vpdf::commands
