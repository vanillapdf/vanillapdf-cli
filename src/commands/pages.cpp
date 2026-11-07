#include "commands/commands.h"
#include "utils/error.h"
#include "utils/handle_guard.h"
#include "utils/page_range.h"

#include <CLI/CLI.hpp>

#include <iostream>
#include <memory>
#include <string>

namespace vanillapdf {
namespace cli {

namespace {

struct PagesDeleteOptions {
    std::string input;
    std::string output;
    std::string pages;
};

struct PagesInsertOptions {
    std::string input;
    std::string output;
    std::string from;
    int at = 0;  // 1-based position to insert before; <= 0 or past the end appends
};

// Open a document and hand back its page tree; the document and catalog are
// returned too so the caller keeps them alive for as long as the tree is used
void open_page_tree(const std::string& path, DocumentGuard& document, CatalogGuard& catalog,
    PageTreeGuard& page_tree) {
    THROW_IF_NOT_SUCCESS(Document_Open(path.c_str(), document.out()), "Failed to open document");
    THROW_IF_NOT_SUCCESS(Document_GetCatalog(document, catalog.out()), "Failed to get catalog");
    THROW_IF_NOT_SUCCESS(Catalog_GetPages(catalog, page_tree.out()), "Failed to get page tree");
}

int process_pages_delete(const PagesDeleteOptions& options) {
    DocumentGuard document;
    CatalogGuard catalog;
    PageTreeGuard page_tree;
    open_page_tree(options.input, document, catalog, page_tree);

    size_type page_count = 0;
    THROW_IF_NOT_SUCCESS(PageTree_GetPageCount(page_tree, &page_count), "Failed to get page count");

    // Page numbers in the PageTree API are 1-based
    auto deleted_pages = resolve_page_range(options.pages, page_count);

    if (deleted_pages.empty()) {
        throw PdfException(VANILLAPDF_ERROR_PARAMETER_VALUE, "No valid pages to delete");
    }

    if (deleted_pages.size() >= page_count) {
        throw PdfException(VANILLAPDF_ERROR_PARAMETER_VALUE, "Cannot delete all pages");
    }

    // Remove from the highest page number down so earlier ones stay valid
    for (auto iterator = deleted_pages.rbegin(); iterator != deleted_pages.rend(); iterator++) {
        auto page_number_converted = static_cast<size_type>(*iterator);
        THROW_IF_NOT_SUCCESS(PageTree_RemovePage(page_tree, page_number_converted), "Failed to remove page");
    }

    THROW_IF_NOT_SUCCESS(Document_Save(document, options.output.c_str()), "Failed to save output");

    auto remaining_page_count = page_count - deleted_pages.size();
    std::cerr << "Wrote: " << options.output << " (" << remaining_page_count << " pages)\n";

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

int process_pages_insert(const PagesInsertOptions& options) {
    DocumentGuard document;
    CatalogGuard catalog;
    PageTreeGuard page_tree;
    open_page_tree(options.input, document, catalog, page_tree);

    size_type page_count = 0;
    THROW_IF_NOT_SUCCESS(PageTree_GetPageCount(page_tree, &page_count), "Failed to get page count");

    // Insert before 1-based page --at; out of range (or omitted) appends at the end
    bool position_in_range = (options.at >= 1) && (static_cast<size_type>(options.at) <= page_count);
    bool append_at_end = !position_in_range;
    size_type insert_position = append_at_end ? 0 : static_cast<size_type>(options.at);

    DocumentGuard source_document;
    CatalogGuard source_catalog;
    PageTreeGuard source_page_tree;
    open_page_tree(options.from, source_document, source_catalog, source_page_tree);

    size_type source_page_count = 0;
    THROW_IF_NOT_SUCCESS(PageTree_GetPageCount(source_page_tree, &source_page_count),
        "Failed to get source page count");

    // Page numbers are 1-based; keep the inserted pages in their source order
    for (size_type page_number = 1; page_number <= source_page_count; page_number++) {
        PageObjectGuard page;
        THROW_IF_NOT_SUCCESS(PageTree_GetPage(source_page_tree, page_number, page.out()), "Failed to get source page");

        if (append_at_end) {
            THROW_IF_NOT_SUCCESS(PageTree_AppendPage(page_tree, page), "Failed to append page");
        } else {
            auto target_position = insert_position + (page_number - 1);
            THROW_IF_NOT_SUCCESS(PageTree_InsertPage(page_tree, target_position, page), "Failed to insert page");
        }
    }

    THROW_IF_NOT_SUCCESS(Document_Save(document, options.output.c_str()), "Failed to save output");

    auto total_page_count = page_count + source_page_count;
    std::cerr << "Wrote: " << options.output << " (" << total_page_count << " pages)\n";

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

} // namespace

void register_pages(CLI::App& app, int& exit_code) {
    auto* command = app.add_subcommand("pages", "Delete or insert pages");
    command->require_subcommand(1);

    // pages delete
    auto delete_options = std::make_shared<PagesDeleteOptions>();
    auto* delete_command = command->add_subcommand("delete", "Delete pages from a PDF document");

    delete_command->add_option("input", delete_options->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    delete_command->add_option("-o,--output", delete_options->output, "Output PDF file")
        ->required();
    delete_command->add_option("-p,--pages", delete_options->pages, "Pages to delete, e.g. 1-3,7,last or even")
        ->required();

    delete_command->callback([delete_options, &exit_code]() {
        exit_code = process_pages_delete(*delete_options);
    });

    // pages insert
    auto insert_options = std::make_shared<PagesInsertOptions>();
    auto* insert_command = command->add_subcommand("insert", "Insert pages from another PDF document");

    insert_command->add_option("input", insert_options->input, "Target PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    insert_command->add_option("-o,--output", insert_options->output, "Output PDF file")
        ->required();
    insert_command->add_option("--from", insert_options->from, "PDF whose pages are inserted")
        ->required()
        ->check(CLI::ExistingFile);
    insert_command->add_option("--at", insert_options->at,
        "1-based position to insert before (default: append at the end)");

    insert_command->callback([insert_options, &exit_code]() {
        exit_code = process_pages_insert(*insert_options);
    });
}

} // cli
} // vanillapdf
