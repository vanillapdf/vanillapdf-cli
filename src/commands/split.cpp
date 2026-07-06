#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <set>
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
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    CatalogPtr catalog;
    check(Document_GetCatalog(document.get(), catalog.put()), "Failed to get catalog");

    PageTreePtr pages;
    check(Catalog_GetPages(catalog.get(), pages.put()), "Failed to get page tree");

    size_type total_pages = 0;
    check(PageTree_GetPageCount(pages.get(), &total_pages), "Failed to get page count");

    // The 1-based pages to keep. Omitting -p keeps everything (a plain copy).
    std::set<size_type> keep;
    if (opts.pages.empty()) {
        for (size_type i = 1; i <= total_pages; ++i) {
            keep.insert(i);
        }
    } else {
        for (int page_num : opts.pages) {
            if (page_num < 1 || static_cast<size_type>(page_num) > total_pages) {
                std::cerr << "Warning: page " << page_num << " out of range (1-" << total_pages << "), skipping\n";
                continue;
            }
            keep.insert(static_cast<size_type>(page_num));
        }
    }

    if (keep.empty()) {
        throw pdf_error(VANILLAPDF_ERROR_PARAMETER_VALUE, "No pages to extract");
    }

    // Extract by removing the pages we are not keeping (highest first so earlier
    // indices stay valid). Editing in place keeps the rest of the document and
    // avoids the blank page that Document_Create would seed.
    for (size_type i = total_pages; i > 0; --i) {
        if (keep.find(i) == keep.end()) {
            check(PageTree_RemovePage(pages.get(), i), "Failed to remove page");
        }
    }

    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save output");
    std::cerr << "Split to: " << opts.output << " (" << keep.size() << " pages)\n";

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
