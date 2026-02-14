#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>

#include "commands/commands.hpp"
#include "utils/error.hpp"

namespace vpdf::commands {

struct InfoOptions {
    std::string input;
};

static int run_info(const InfoOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    CatalogPtr catalog;
    check(Document_GetCatalog(document.get(), catalog.put()), "Failed to get catalog");

    PageTreePtr page_tree;
    check(Catalog_GetPages(catalog.get(), page_tree.put()), "Failed to get page tree");

    size_type page_count = 0;
    check(PageTree_GetPageCount(page_tree.get(), &page_count), "Failed to get page count");

    std::cout << "File: " << opts.input << "\n";
    std::cout << "Pages: " << page_count << "\n";

    return 0;
}

void register_info(CLI::App& app) {
    auto opts = std::make_shared<InfoOptions>();

    auto* cmd = app.add_subcommand("info", "Display PDF file information");
    cmd->add_option("input", opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);

    cmd->callback([opts]() {
        try {
            std::exit(run_info(*opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
