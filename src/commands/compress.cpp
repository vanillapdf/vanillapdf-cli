#include "commands/commands.h"
#include "utils/error.h"
#include "utils/handle_guard.h"

#include <CLI/CLI.hpp>

#include <iostream>
#include <memory>
#include <string>

namespace vanillapdf {
namespace cli {

namespace {

struct CompressOptions {
    std::string input;
    std::string output;
};

int process_compress(const CompressOptions& options) {
    DocumentGuard document;
    THROW_IF_NOT_SUCCESS(Document_Open(options.input.c_str(), document.out()), "Failed to open document");

    THROW_IF_NOT_SUCCESS(Document_Save(document, options.output.c_str()), "Failed to save compressed document");
    std::cerr << "Compressed: " << options.output << "\n";

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

} // namespace

void register_compress(CLI::App& app, int& exit_code) {
    auto options = std::make_shared<CompressOptions>();
    auto* command = app.add_subcommand("compress", "Compress and optimize a PDF document");

    command->add_option("input", options->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    command->add_option("-o,--output", options->output, "Output PDF file")
        ->required();

    command->callback([options, &exit_code]() {
        exit_code = process_compress(*options);
    });
}

} // cli
} // vanillapdf
