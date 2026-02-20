#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "commands/commands.hpp"
#include "utils/handles.hpp"

namespace vpdf::commands {

struct CompressOptions {
    std::string input;
    std::string output;
};

static int run_compress(const CompressOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save compressed document");
    std::cerr << "Compressed: " << opts.output << "\n";

    return 0;
}

void register_compress(CLI::App& app) {
    auto opts = std::make_shared<CompressOptions>();

    auto* cmd = app.add_subcommand("compress", "Compress and optimize a PDF document");
    cmd->add_option("input", opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    cmd->add_option("-o,--output", opts->output, "Output PDF file")
        ->required();

    cmd->callback([opts]() {
        try {
            std::exit(run_compress(*opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
