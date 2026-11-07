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

struct RotateOptions {
    std::string input;
    std::string output;
    int angle = 0;
    std::string pages;
};

void rotate_page(PageTreeHandle* page_tree, size_type page_number, int angle) {
    PageObjectGuard page;
    THROW_IF_NOT_SUCCESS(PageTree_GetPage(page_tree, page_number, page.out()), "Failed to get page");

    DictionaryObjectGuard page_dictionary;
    THROW_IF_NOT_SUCCESS(PageObject_GetBaseObject(page, page_dictionary.out()), "Failed to get page dictionary");

    NameObjectGuard rotate_name;
    THROW_IF_NOT_SUCCESS(NameObject_CreateFromDecodedString("Rotate", rotate_name.out()),
        "Failed to create Rotate name");

    boolean_type rotate_found = VANILLAPDF_RV_FALSE;
    ObjectGuard existing_rotate;
    THROW_IF_NOT_SUCCESS(DictionaryObject_TryFind(page_dictionary, rotate_name, &rotate_found, existing_rotate.out()),
        "Failed to search for Rotate entry");

    int current_rotation = 0;
    if (rotate_found == VANILLAPDF_RV_TRUE) {
        IntegerObjectGuard existing_integer;
        THROW_IF_NOT_SUCCESS(IntegerObject_FromObject(existing_rotate, existing_integer.out()),
            "Failed to read Rotate value");

        bigint_type existing_value = 0;
        THROW_IF_NOT_SUCCESS(IntegerObject_GetIntegerValue(existing_integer, &existing_value),
            "Failed to get integer value");
        current_rotation = static_cast<int>(existing_value);
    }

    int new_rotation = (current_rotation + angle) % 360;
    auto new_rotation_converted = static_cast<bigint_type>(new_rotation);

    IntegerObjectGuard new_integer;
    THROW_IF_NOT_SUCCESS(IntegerObject_Create(new_integer.out()), "Failed to create integer object");
    THROW_IF_NOT_SUCCESS(IntegerObject_SetIntegerValue(new_integer, new_rotation_converted),
        "Failed to set rotation value");

    ObjectGuard new_rotate;
    THROW_IF_NOT_SUCCESS(IntegerObject_ToObject(new_integer, new_rotate.out()), "Failed to convert to object");

    THROW_IF_NOT_SUCCESS(DictionaryObject_Insert(page_dictionary, rotate_name, new_rotate, VANILLAPDF_RV_TRUE),
        "Failed to set Rotate entry");
}

int process_rotate(const RotateOptions& options) {
    DocumentGuard document;
    THROW_IF_NOT_SUCCESS(Document_Open(options.input.c_str(), document.out()), "Failed to open document");

    CatalogGuard catalog;
    THROW_IF_NOT_SUCCESS(Document_GetCatalog(document, catalog.out()), "Failed to get catalog");

    PageTreeGuard page_tree;
    THROW_IF_NOT_SUCCESS(Catalog_GetPages(catalog, page_tree.out()), "Failed to get page tree");

    size_type page_count = 0;
    THROW_IF_NOT_SUCCESS(PageTree_GetPageCount(page_tree, &page_count), "Failed to get page count");

    // Page numbers in the PageTree API are 1-based
    if (options.pages.empty()) {
        for (size_type page_number = 1; page_number <= page_count; page_number++) {
            rotate_page(page_tree, page_number, options.angle);
        }

        std::cerr << "Rotated all " << page_count << " pages by " << options.angle << " degrees\n";
    } else {
        // A set, so a page listed twice (e.g. "1-3,2") is still rotated once
        auto rotated_pages = resolve_page_range(options.pages, page_count);
        for (auto page_number : rotated_pages) {
            auto page_number_converted = static_cast<size_type>(page_number);
            rotate_page(page_tree, page_number_converted, options.angle);
        }

        std::cerr << "Rotated " << rotated_pages.size() << " pages by " << options.angle << " degrees\n";
    }

    THROW_IF_NOT_SUCCESS(Document_Save(document, options.output.c_str()), "Failed to save document");
    std::cerr << "Output: " << options.output << "\n";

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

} // namespace

void register_rotate(CLI::App& app, int& exit_code) {
    auto options = std::make_shared<RotateOptions>();
    auto* command = app.add_subcommand("rotate", "Rotate pages in a PDF document");

    command->add_option("input", options->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    command->add_option("-o,--output", options->output, "Output PDF file")
        ->required();
    command->add_option("-a,--angle", options->angle, "Rotation angle (90, 180, 270)")
        ->required()
        ->check(CLI::IsMember({90, 180, 270}));
    command->add_option("-p,--pages", options->pages,
        "Pages to rotate, e.g. 1-3,7,last or even (default: all)");

    command->callback([options, &exit_code]() {
        exit_code = process_rotate(*options);
    });
}

} // cli
} // vanillapdf
