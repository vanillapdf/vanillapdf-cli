#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "commands/commands.hpp"
#include "utils/handles.hpp"
#include "utils/page_range.hpp"

namespace vanillapdf::cli::commands {

struct PagesDeleteOptions {
    std::string input;
    std::string output;
    std::string pages;
};

struct PagesReorderOptions {
    std::string input;
    std::string output;
    std::vector<int> order;
};

struct PagesInsertOptions {
    std::string input;
    std::string output;
    std::string from;
    int at = 0;  // 1-based position to insert before; <= 0 or past the end appends
};

// Open a document and hand back its page tree; the document and catalog are
// returned too so the caller keeps them alive for as long as the tree is used.
static void open_page_tree(const std::string& path, DocumentPtr& document,
                           CatalogPtr& catalog, PageTreePtr& pages) {
    check(Document_Open(path.c_str(), document.put()), "Failed to open document");
    check(Document_GetCatalog(document.get(), catalog.put()), "Failed to get catalog");
    check(Catalog_GetPages(catalog.get(), pages.put()), "Failed to get page tree");
}

static int run_pages_delete(const PagesDeleteOptions& opts) {
    DocumentPtr document;
    CatalogPtr catalog;
    PageTreePtr pages;
    open_page_tree(opts.input, document, catalog, pages);

    size_type total = 0;
    check(PageTree_GetPageCount(pages.get(), &total), "Failed to get page count");

    // Page indices in the PageTree API are 1-based.
    std::set<size_type> to_remove;
    for (std::size_t page_num : resolve_page_range(opts.pages, total)) {
        to_remove.insert(static_cast<size_type>(page_num));
    }

    if (to_remove.empty()) {
        throw pdf_error(VANILLAPDF_ERROR_PARAMETER_VALUE, "No valid pages to delete");
    }
    if (to_remove.size() >= total) {
        throw pdf_error(VANILLAPDF_ERROR_PARAMETER_VALUE, "Cannot delete all pages");
    }

    // Remove from the highest page number down so earlier ones stay valid.
    for (auto it = to_remove.rbegin(); it != to_remove.rend(); ++it) {
        check(PageTree_RemovePage(pages.get(), *it), "Failed to remove page");
    }

    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save output");
    std::cerr << "Wrote: " << opts.output << " (" << (total - to_remove.size()) << " pages)\n";
    return 0;
}

static int run_pages_reorder(const PagesReorderOptions& opts) {
    DocumentPtr document;
    CatalogPtr catalog;
    PageTreePtr pages;
    open_page_tree(opts.input, document, catalog, pages);

    size_type total = 0;
    check(PageTree_GetPageCount(pages.get(), &total), "Failed to get page count");

    // --order must be a permutation of 1..total (every page exactly once).
    if (opts.order.size() != total) {
        throw pdf_error(VANILLAPDF_ERROR_PARAMETER_VALUE,
                        "--order must list all " + std::to_string(total) + " pages exactly once");
    }
    std::set<int> seen;
    for (int page_num : opts.order) {
        if (page_num < 1 || static_cast<size_type>(page_num) > total || !seen.insert(page_num).second) {
            throw pdf_error(VANILLAPDF_ERROR_PARAMETER_VALUE,
                            "--order must be a permutation of 1.." + std::to_string(total));
        }
    }

    // Take a reference to each page in the desired order (this keeps the page
    // objects alive across the removal below), then rebuild the tree.
    // Page indices are 1-based. Take a reference to each page in the desired
    // order (keeping the page objects alive across the removal below).
    std::vector<PageObjectPtr> ordered;
    ordered.reserve(total);
    for (int page_num : opts.order) {
        PageObjectPtr page;
        check(PageTree_GetPage(pages.get(), static_cast<size_type>(page_num), page.put()),
              "Failed to get page");
        ordered.push_back(std::move(page));
    }

    for (size_type i = total; i > 0; --i) {
        check(PageTree_RemovePage(pages.get(), i), "Failed to remove page");
    }
    for (auto& page : ordered) {
        check(PageTree_AppendPage(pages.get(), page.get()), "Failed to append page");
    }

    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save output");
    std::cerr << "Wrote: " << opts.output << "\n";
    return 0;
}

static int run_pages_insert(const PagesInsertOptions& opts) {
    DocumentPtr document;
    CatalogPtr catalog;
    PageTreePtr pages;
    open_page_tree(opts.input, document, catalog, pages);

    size_type total = 0;
    check(PageTree_GetPageCount(pages.get(), &total), "Failed to get page count");

    // Insert before 1-based page --at; out of range (or omitted) appends at the end.
    const bool append_at_end = !(opts.at >= 1 && static_cast<size_type>(opts.at) <= total);
    const size_type insert_at = append_at_end ? 0 : static_cast<size_type>(opts.at);

    DocumentPtr from_document;
    CatalogPtr from_catalog;
    PageTreePtr from_pages;
    open_page_tree(opts.from, from_document, from_catalog, from_pages);

    size_type from_total = 0;
    check(PageTree_GetPageCount(from_pages.get(), &from_total), "Failed to get source page count");

    // Page indices are 1-based; keep the inserted pages in their source order.
    for (size_type i = 1; i <= from_total; ++i) {
        PageObjectPtr page;
        check(PageTree_GetPage(from_pages.get(), i, page.put()), "Failed to get source page");
        if (append_at_end) {
            check(PageTree_AppendPage(pages.get(), page.get()), "Failed to append page");
        } else {
            check(PageTree_InsertPage(pages.get(), insert_at + (i - 1), page.get()),
                  "Failed to insert page");
        }
    }

    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save output");
    std::cerr << "Wrote: " << opts.output << " (" << (total + from_total) << " pages)\n";
    return 0;
}

void register_pages(CLI::App& app) {
    auto* cmd = app.add_subcommand("pages", "Delete, reorder, or insert pages");
    cmd->require_subcommand(1);

    // pages delete
    auto del_opts = std::make_shared<PagesDeleteOptions>();
    auto* del_cmd = cmd->add_subcommand("delete", "Delete pages from a PDF document");
    del_cmd->add_option("input", del_opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    del_cmd->add_option("-o,--output", del_opts->output, "Output PDF file")
        ->required();
    del_cmd->add_option("-p,--pages", del_opts->pages, "Pages to delete, e.g. 1-3,7,last or even")
        ->required();
    del_cmd->callback([del_opts]() {
        try {
            std::exit(run_pages_delete(*del_opts));
        } catch (const pdf_error& e) {
            report_error(e);
            std::exit(1);
        } catch (const page_range_error& e) {
            report_error(e);
            std::exit(1);
        }
    });

    // pages reorder
    auto reo_opts = std::make_shared<PagesReorderOptions>();
    auto* reo_cmd = cmd->add_subcommand("reorder", "Reorder the pages of a PDF document");
    reo_cmd->add_option("input", reo_opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    reo_cmd->add_option("-o,--output", reo_opts->output, "Output PDF file")
        ->required();
    reo_cmd->add_option("--order", reo_opts->order,
                        "New page order as a comma-separated permutation, e.g. 3,1,2")
        ->required()
        ->delimiter(',');
    reo_cmd->callback([reo_opts]() {
        try {
            std::exit(run_pages_reorder(*reo_opts));
        } catch (const pdf_error& e) {
            report_error(e);
            std::exit(1);
        }
    });

    // pages insert
    auto ins_opts = std::make_shared<PagesInsertOptions>();
    auto* ins_cmd = cmd->add_subcommand("insert", "Insert pages from another PDF document");
    ins_cmd->add_option("input", ins_opts->input, "Target PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    ins_cmd->add_option("-o,--output", ins_opts->output, "Output PDF file")
        ->required();
    ins_cmd->add_option("--from", ins_opts->from, "PDF whose pages are inserted")
        ->required()
        ->check(CLI::ExistingFile);
    ins_cmd->add_option("--at", ins_opts->at,
                        "1-based position to insert before (default: append at the end)");
    ins_cmd->callback([ins_opts]() {
        try {
            std::exit(run_pages_insert(*ins_opts));
        } catch (const pdf_error& e) {
            report_error(e);
            std::exit(1);
        }
    });
}

} // namespace vanillapdf::cli::commands
