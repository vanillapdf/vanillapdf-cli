#include "commands/commands.h"
#include "utils/error.h"
#include "utils/handle_guard.h"
#include "utils/page_range.h"

#include <CLI/CLI.hpp>

#include <iostream>
#include <memory>
#include <set>
#include <string>

namespace vanillapdf {
namespace cli {

namespace {

struct SplitOptions {
    std::string input;
    std::string output;
    std::string pages;
};

int process_split(const SplitOptions& options) {
    DocumentGuard document;
    THROW_IF_NOT_SUCCESS(Document_Open(options.input.c_str(), document.out()), "Failed to open document");

    CatalogGuard catalog;
    THROW_IF_NOT_SUCCESS(Document_GetCatalog(document, catalog.out()), "Failed to get catalog");

    PageTreeGuard page_tree;
    THROW_IF_NOT_SUCCESS(Catalog_GetPages(catalog, page_tree.out()), "Failed to get page tree");

    size_type page_count = 0;
    THROW_IF_NOT_SUCCESS(PageTree_GetPageCount(page_tree, &page_count), "Failed to get page count");

    // The 1-based pages to keep, in document order. Omitting -p keeps everything
    // (a plain copy).
    std::set<std::size_t> kept_pages;
    if (options.pages.empty()) {
        for (std::size_t page_number = 1; page_number <= page_count; page_number++) {
            kept_pages.insert(page_number);
        }
    } else {
        kept_pages = resolve_page_range(options.pages, page_count);
    }

    if (kept_pages.empty()) {
        throw PdfException(VANILLAPDF_ERROR_PARAMETER_VALUE, "No pages to extract");
    }

    // Extract by removing the pages we are not keeping (highest first so earlier
    // indices stay valid). Editing in place keeps the rest of the document and
    // avoids the blank page that Document_Create would seed.
    for (size_type page_number = page_count; page_number > 0; page_number--) {
        if (kept_pages.count(page_number) == 0) {
            THROW_IF_NOT_SUCCESS(PageTree_RemovePage(page_tree, page_number), "Failed to remove page");
        }
    }

    THROW_IF_NOT_SUCCESS(Document_Save(document, options.output.c_str()), "Failed to save output");
    std::cerr << "Split to: " << options.output << " (" << kept_pages.size() << " pages)\n";

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

} // namespace

void register_split(CLI::App& app, int& exit_code) {
    auto options = std::make_shared<SplitOptions>();
    auto* command = app.add_subcommand("split", "Split pages from a PDF document");

    command->add_option("input", options->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    command->add_option("-o,--output", options->output, "Output PDF file")
        ->required();
    command->add_option("-p,--pages", options->pages,
        "Pages to extract, e.g. 1-3,7,last or odd,!1 (default: all); kept in document order");

    command->callback([options, &exit_code]() {
        exit_code = process_split(*options);
    });
}

} // cli
} // vanillapdf
