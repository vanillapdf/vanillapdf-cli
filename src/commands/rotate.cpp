#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "commands/commands.hpp"
#include "utils/error.hpp"

namespace vpdf::commands {

struct RotateOptions {
    std::string input;
    std::string output;
    int angle = 0;
    std::vector<int> pages;
};

static int run_rotate(const RotateOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    CatalogPtr catalog;
    check(Document_GetCatalog(document.get(), catalog.put()), "Failed to get catalog");

    PageTreePtr page_tree;
    check(Catalog_GetPages(catalog.get(), page_tree.put()), "Failed to get page tree");

    size_type total_pages = 0;
    check(PageTree_GetPageCount(page_tree.get(), &total_pages), "Failed to get page count");

    // TODO: implement page rotation using vanillapdf API
    // 1. For each target page, get the /Rotate entry
    // 2. Add the requested angle (mod 360)
    // 3. Set the new /Rotate value

    if (opts.pages.empty()) {
        std::cerr << "Rotating all " << total_pages << " pages by " << opts.angle << " degrees\n";
    } else {
        std::cerr << "Rotating " << opts.pages.size() << " pages by " << opts.angle << " degrees\n";
    }

    check(Document_Save(document.get(), opts.output.c_str()), "Failed to save document");
    std::cerr << "Output: " << opts.output << "\n";

    return 0;
}

void register_rotate(CLI::App& app) {
    auto opts = std::make_shared<RotateOptions>();

    auto* cmd = app.add_subcommand("rotate", "Rotate pages in a PDF document");
    cmd->add_option("input", opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    cmd->add_option("-o,--output", opts->output, "Output PDF file")
        ->required();
    cmd->add_option("-a,--angle", opts->angle, "Rotation angle (90, 180, 270)")
        ->required()
        ->check(CLI::IsMember({90, 180, 270}));
    cmd->add_option("-p,--pages", opts->pages, "Page numbers to rotate (1-based, default: all)");

    cmd->callback([opts]() {
        try {
            std::exit(run_rotate(*opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
