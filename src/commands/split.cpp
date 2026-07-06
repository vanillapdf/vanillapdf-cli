#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "commands/commands.hpp"
#include "utils/handles.hpp"

namespace vanillapdf::cli::commands {

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

    // Document_Create seeds the new document with a blank page; record how many
    // pages precede the ones we append so they can be dropped afterwards.
    size_type seeded_pages = 0;
    check(PageTree_GetPageCount(out_pages.get(), &seeded_pages), "Failed to get output page count");

    // Page indices in the PageTree API are 1-based.
    size_type extracted = 0;
    if (opts.pages.empty()) {
        for (size_type i = 1; i <= total_pages; ++i) {
            PageObjectPtr page;
            check(PageTree_GetPage(in_pages.get(), i, page.put()), "Failed to get page");
            check(PageTree_AppendPage(out_pages.get(), page.get()), "Failed to append page");
            ++extracted;
        }
    } else {
        for (int page_num : opts.pages) {
            if (page_num < 1 || static_cast<size_type>(page_num) > total_pages) {
                std::cerr << "Warning: page " << page_num << " out of range (1-" << total_pages << "), skipping\n";
                continue;
            }
            PageObjectPtr page;
            check(PageTree_GetPage(in_pages.get(), static_cast<size_type>(page_num), page.put()),
                  "Failed to get page");
            check(PageTree_AppendPage(out_pages.get(), page.get()), "Failed to append page");
            ++extracted;
        }
    }

    if (extracted == 0) {
        throw pdf_error(VANILLAPDF_ERROR_PARAMETER_VALUE, "No pages to extract");
    }

    // Drop the seeded blank page(s) that preceded the extracted pages.
    for (size_type i = seeded_pages; i > 0; --i) {
        check(PageTree_RemovePage(out_pages.get(), i), "Failed to remove placeholder page");
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
            report_error(e);
            std::exit(1);
        }
    });
}

} // namespace vanillapdf::cli::commands
