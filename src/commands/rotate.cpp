#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "commands/commands.hpp"
#include "utils/handles.hpp"

namespace vpdf::commands {

struct RotateOptions {
    std::string input;
    std::string output;
    int angle = 0;
    std::vector<int> pages;
};

static void rotate_page(PageTreeHandle* page_tree, size_type page_idx, int angle) {
    PageObjectPtr page;
    check(PageTree_GetPage(page_tree, page_idx, page.put()), "Failed to get page");

    DictionaryObjectPtr dict;
    check(PageObject_GetBaseObject(page.get(), dict.put()), "Failed to get page dictionary");

    NameObjectPtr rotate_name;
    check(NameObject_CreateFromDecodedString("Rotate", rotate_name.put()),
          "Failed to create Rotate name");

    int current_rotation = 0;

    ObjectPtr existing;
    boolean_type found = VANILLAPDF_RV_FALSE;
    error_type find_err = DictionaryObject_TryFind(dict.get(), rotate_name.get(), &found, existing.put());
    check(find_err, "Failed to search for Rotate entry");

    if (found == VANILLAPDF_RV_TRUE) {
        IntegerObjectPtr int_obj;
        check(IntegerObject_FromObject(existing.get(), int_obj.put()), "Failed to read Rotate value");
        bigint_type val = 0;
        check(IntegerObject_GetIntegerValue(int_obj.get(), &val), "Failed to get integer value");
        current_rotation = static_cast<int>(val);
    }

    int new_rotation = (current_rotation + angle) % 360;

    IntegerObjectPtr new_int;
    check(IntegerObject_Create(new_int.put()), "Failed to create integer object");
    check(IntegerObject_SetIntegerValue(new_int.get(), static_cast<bigint_type>(new_rotation)),
          "Failed to set rotation value");

    ObjectPtr new_obj;
    check(IntegerObject_ToObject(new_int.get(), new_obj.put()), "Failed to convert to object");

    check(DictionaryObject_Insert(dict.get(), rotate_name.get(), new_obj.get(), VANILLAPDF_RV_TRUE),
          "Failed to set Rotate entry");
}

static int run_rotate(const RotateOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    CatalogPtr catalog;
    check(Document_GetCatalog(document.get(), catalog.put()), "Failed to get catalog");

    PageTreePtr page_tree;
    check(Catalog_GetPages(catalog.get(), page_tree.put()), "Failed to get page tree");

    size_type total_pages = 0;
    check(PageTree_GetPageCount(page_tree.get(), &total_pages), "Failed to get page count");

    if (opts.pages.empty()) {
        for (size_type i = 0; i < total_pages; ++i) {
            rotate_page(page_tree.get(), i, opts.angle);
        }
        std::cerr << "Rotated all " << total_pages << " pages by " << opts.angle << " degrees\n";
    } else {
        for (int page_num : opts.pages) {
            auto idx = static_cast<size_type>(page_num - 1);
            if (idx >= total_pages) {
                std::cerr << "Warning: page " << page_num << " out of range (1-" << total_pages << "), skipping\n";
                continue;
            }
            rotate_page(page_tree.get(), idx, opts.angle);
        }
        std::cerr << "Rotated " << opts.pages.size() << " pages by " << opts.angle << " degrees\n";
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
