#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "commands/commands.hpp"
#include "utils/handles.hpp"

namespace vpdf::commands {

struct MergeOptions {
    std::vector<std::string> inputs;
    std::string output;
};

static int run_merge(const MergeOptions& opts) {
    DocumentPtr out_document;
    check(Document_Create(opts.output.c_str(), out_document.put()), "Failed to create output document");

    for (const auto& input_path : opts.inputs) {
        DocumentPtr in_document;
        check(Document_Open(input_path.c_str(), in_document.put()), "Failed to open " + input_path);

        check(Document_AppendDocument(out_document.get(), in_document.get()),
              "Failed to append " + input_path);

        std::cerr << "Merged: " << input_path << "\n";
    }

    check(Document_Save(out_document.get(), opts.output.c_str()), "Failed to save output");
    std::cerr << "Output: " << opts.output << "\n";

    return 0;
}

void register_merge(CLI::App& app) {
    auto opts = std::make_shared<MergeOptions>();

    auto* cmd = app.add_subcommand("merge", "Merge multiple PDF files into one");
    cmd->add_option("inputs", opts->inputs, "Input PDF files")
        ->required()
        ->expected(-1)
        ->check(CLI::ExistingFile);
    cmd->add_option("-o,--output", opts->output, "Output PDF file")
        ->required();

    cmd->callback([opts]() {
        try {
            std::exit(run_merge(*opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
