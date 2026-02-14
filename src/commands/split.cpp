#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "commands/commands.hpp"
#include "utils/error.hpp"

namespace vpdf::commands {

struct SplitOptions {
    std::string input;
    std::string output;
    std::vector<int> pages;
};

static int run_split(const SplitOptions& opts) {
    DocumentPtr in_document;
    check(Document_Open(opts.input.c_str(), in_document.put()), "Failed to open document");

    CatalogPtr in_catalog;
    check(Document_GetCatalog(in_document.get(), in_catalog.put()), "Failed to get catalog");

    PageTreePtr in_pages;
    check(Catalog_GetPages(in_catalog.get(), in_pages.put()), "Failed to get page tree");

    size_type total_pages = 0;
    check(PageTree_GetPageCount(in_pages.get(), &total_pages), "Failed to get page count");

    DocumentPtr out_document;
    check(Document_Create(opts.output.c_str(), out_document.put()), "Failed to create output document");

    CatalogPtr out_catalog;
    check(Document_GetCatalog(out_document.get(), out_catalog.put()), "Failed to get output catalog");

    PageTreePtr out_pages;
    check(Catalog_GetPages(out_catalog.get(), out_pages.put()), "Failed to get output page tree");

    if (opts.pages.empty()) {
        for (size_type i = 0; i < total_pages; ++i) {
            PageObjectPtr page;
            check(PageTree_GetPage(in_pages.get(), i, page.put()), "Failed to get page");
            check(PageTree_AppendPage(out_pages.get(), page.get()), "Failed to append page");
        }
    } else {
        for (int page_num : opts.pages) {
            auto idx = static_cast<size_type>(page_num - 1);
            if (idx >= total_pages) {
                std::cerr << "Warning: page " << page_num << " out of range (1-" << total_pages << "), skipping\n";
                continue;
            }
            PageObjectPtr page;
            check(PageTree_GetPage(in_pages.get(), idx, page.put()), "Failed to get page");
            check(PageTree_AppendPage(out_pages.get(), page.get()), "Failed to append page");
        }
    }

    check(Document_Save(out_document.get(), opts.output.c_str()), "Failed to save output");
    std::cerr << "Split to: " << opts.output << "\n";

    return 0;
}

void register_split(CLI::App& app) {
    auto opts = std::make_shared<SplitOptions>();

    auto* cmd = app.add_subcommand("split", "Split pages from a PDF document");
    cmd->add_option("input", opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    cmd->add_option("-o,--output", opts->output, "Output PDF file")
        ->required();
    cmd->add_option("-p,--pages", opts->pages, "Page numbers to extract (1-based, default: all)");

    cmd->callback([opts]() {
        try {
            std::exit(run_split(*opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
