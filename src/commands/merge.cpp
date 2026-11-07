#include "commands/commands.h"
#include "utils/error.h"
#include "utils/handle_guard.h"

#include <CLI/CLI.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace vanillapdf {
namespace cli {

namespace {

struct MergeOptions {
    std::vector<std::string> inputs;
    std::string output;
};

int process_merge(const MergeOptions& options) {
    // Use the first input as the base document and append the rest. Document_Create
    // would seed the output with a blank page, leaving a spurious leading page.
    const auto& base_path = options.inputs.front();

    DocumentGuard merged_document;
    THROW_IF_NOT_SUCCESS(Document_Open(base_path.c_str(), merged_document.out()), "Failed to open " + base_path);
    std::cerr << "Merged: " << base_path << "\n";

    for (size_t i = 1; i < options.inputs.size(); i++) {
        const auto& input_path = options.inputs[i];

        DocumentGuard input_document;
        THROW_IF_NOT_SUCCESS(Document_Open(input_path.c_str(), input_document.out()), "Failed to open " + input_path);
        THROW_IF_NOT_SUCCESS(Document_AppendDocument(merged_document, input_document),
            "Failed to append " + input_path);

        std::cerr << "Merged: " << input_path << "\n";
    }

    THROW_IF_NOT_SUCCESS(Document_Save(merged_document, options.output.c_str()), "Failed to save output");
    std::cerr << "Output: " << options.output << "\n";

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

} // namespace

void register_merge(CLI::App& app, int& exit_code) {
    auto options = std::make_shared<MergeOptions>();
    auto* command = app.add_subcommand("merge", "Merge multiple PDF files into one");

    command->add_option("inputs", options->inputs, "Input PDF files")
        ->required()
        ->expected(-1)
        ->check(CLI::ExistingFile);
    command->add_option("-o,--output", options->output, "Output PDF file")
        ->required();

    command->callback([options, &exit_code]() {
        exit_code = process_merge(*options);
    });
}

} // cli
} // vanillapdf
