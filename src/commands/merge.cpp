#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "commands/commands.hpp"
#include "utils/error.hpp"

namespace vpdf::commands {

struct MergeOptions {
    std::vector<std::string> inputs;
    std::string output;
};

static int run_merge(const MergeOptions& opts) {
    DocumentPtr out_document;
    check(Document_Create(opts.output.c_str(), out_document.put()), "Failed to create output document");

    CatalogPtr out_catalog;
    check(Document_GetCatalog(out_document.get(), out_catalog.put()), "Failed to get output catalog");

    PageTreePtr out_pages;
    check(Catalog_GetPages(out_catalog.get(), out_pages.put()), "Failed to get output page tree");

    for (const auto& input_path : opts.inputs) {
        DocumentPtr in_document;
        check(Document_Open(input_path.c_str(), in_document.put()), "Failed to open " + input_path);

        CatalogPtr in_catalog;
        check(Document_GetCatalog(in_document.get(), in_catalog.put()), "Failed to get catalog");

        PageTreePtr in_pages;
        check(Catalog_GetPages(in_catalog.get(), in_pages.put()), "Failed to get page tree");

        size_type count = 0;
        check(PageTree_GetPageCount(in_pages.get(), &count), "Failed to get page count");

        for (size_type i = 0; i < count; ++i) {
            PageObjectPtr page;
            check(PageTree_GetPage(in_pages.get(), i, page.put()), "Failed to get page");
            check(PageTree_AppendPage(out_pages.get(), page.get()), "Failed to append page");
        }

        std::cerr << "Merged: " << input_path << " (" << count << " pages)\n";
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
