#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "commands/commands.hpp"
#include "utils/handles.hpp"

namespace vanillapdf::cli::commands {

struct MergeOptions {
    std::vector<std::string> inputs;
    std::string output;
};

static int run_merge(const MergeOptions& opts) {
    // Use the first input as the base document and append the rest. Document_Create
    // would seed the output with a blank page, leaving a spurious leading page.
    DocumentPtr out_document;
    check(Document_Open(opts.inputs.front().c_str(), out_document.put()),
          "Failed to open " + opts.inputs.front());
    std::cerr << "Merged: " << opts.inputs.front() << "\n";

    for (size_t i = 1; i < opts.inputs.size(); ++i) {
        const auto& input_path = opts.inputs[i];
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
            report_error(e);
            std::exit(1);
        }
    });
}

} // namespace vanillapdf::cli::commands
