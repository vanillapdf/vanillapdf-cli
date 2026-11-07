#include "commands/commands.h"
#include "utils/encoding.h"
#include "utils/error.h"
#include "utils/handle_guard.h"
#include "utils/json.h"

#include <CLI/CLI.hpp>

#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace vanillapdf {
namespace cli {

namespace {

struct InfoOptions {
    std::string input;
    bool json_output = false;
};

// The document information dictionary fields, with their human-readable label
// and JSON key, rendered in this order by both output modes
struct InfoField {
    const char* label;
    const char* key;
    error_type (*getter)(DocumentInfoHandle*, StringObjectHandle**);
};

const InfoField INFO_FIELDS[] = {
    {"Title",    "title",    DocumentInfo_GetTitle},
    {"Author",   "author",   DocumentInfo_GetAuthor},
    {"Subject",  "subject",  DocumentInfo_GetSubject},
    {"Keywords", "keywords", DocumentInfo_GetKeywords},
    {"Creator",  "creator",  DocumentInfo_GetCreator},
    {"Producer", "producer", DocumentInfo_GetProducer},
};

const std::map<PDFVersion, const char*> PDF_VERSION_NAMES = {
    {PDFVersion_10, "1.0"}, {PDFVersion_11, "1.1"}, {PDFVersion_12, "1.2"},
    {PDFVersion_13, "1.3"}, {PDFVersion_14, "1.4"}, {PDFVersion_15, "1.5"},
    {PDFVersion_16, "1.6"}, {PDFVersion_17, "1.7"}, {PDFVersion_20, "2.0"},
};

void print_optional_field(DocumentInfoHandle* document_info, const InfoField& field) {
    StringObjectGuard text;
    error_type result = field.getter(document_info, text.out());

    if (result == VANILLAPDF_ERROR_SUCCESS) {
        auto value = to_string_utf8(text);
        std::cout << field.label << ": " << value << "\n";
        return;
    }

    if (result == VANILLAPDF_ERROR_OBJECT_MISSING) {
        std::cout << field.label << ": N/A\n";
        return;
    }

    std::cout << field.label << ": <unable to parse>";
    if (g_verbose) {
        auto detail = last_error_message();
        std::cout << " (" << detail << ")";
    }

    std::cout << "\n";
}

void print_info_text(const std::string& file, const char* version, size_type page_count,
    DocumentInfoHandle* document_info) {
    std::cout << "File: " << file << "\n";
    if (version != nullptr) {
        std::cout << "PDF Version: " << version << "\n";
    }

    std::cout << "Pages: " << page_count << "\n";
    if (document_info != nullptr) {
        for (const auto& field : INFO_FIELDS) {
            print_optional_field(document_info, field);
        }
    }
}

void print_info_json(const std::string& file, const char* version, size_type page_count,
    DocumentInfoHandle* document_info) {
    std::cout << "{\n";
    std::cout << "  \"file\": " << json_escape(file) << ",\n";
    std::cout << "  \"pdf_version\": ";
    if (version != nullptr) {
        std::cout << json_escape(version);
    } else {
        std::cout << "null";
    }

    std::cout << ",\n";
    std::cout << "  \"pages\": " << page_count;

    // A missing or unparseable field is emitted as null so the shape is stable
    for (const auto& field : INFO_FIELDS) {
        std::cout << ",\n  " << json_escape(field.key) << ": ";

        if (document_info == nullptr) {
            std::cout << "null";
            continue;
        }

        StringObjectGuard text;
        error_type result = field.getter(document_info, text.out());
        if (result != VANILLAPDF_ERROR_SUCCESS) {
            std::cout << "null";
            continue;
        }

        auto value = to_string_utf8(text);
        std::cout << json_escape(value);
    }

    std::cout << "\n}\n";
}

int process_info(const InfoOptions& options) {
    DocumentGuard document;
    THROW_IF_NOT_SUCCESS(Document_Open(options.input.c_str(), document.out()), "Failed to open document");

    CatalogGuard catalog;
    THROW_IF_NOT_SUCCESS(Document_GetCatalog(document, catalog.out()), "Failed to get catalog");

    PageTreeGuard page_tree;
    THROW_IF_NOT_SUCCESS(Catalog_GetPages(catalog, page_tree.out()), "Failed to get page tree");

    size_type page_count = 0;
    THROW_IF_NOT_SUCCESS(PageTree_GetPageCount(page_tree, &page_count), "Failed to get page count");

    // PDF version from the file header
    FileGuard file;
    PDFVersion pdf_version = PDFVersion_Undefined;
    error_type file_result = File_Open(options.input.c_str(), file.out());
    if (file_result == VANILLAPDF_ERROR_SUCCESS) {
        File_GetVersion(file, &pdf_version);
    }

    const char* version_name = nullptr;
    auto version_iterator = PDF_VERSION_NAMES.find(pdf_version);
    if (version_iterator != PDF_VERSION_NAMES.end()) {
        version_name = version_iterator->second;
    }

    DocumentInfoGuard document_info;
    error_type info_result = Document_GetDocumentInfo(document, document_info.out());
    if (info_result != VANILLAPDF_ERROR_SUCCESS) {
        document_info.reset();
    }

    if (options.json_output) {
        print_info_json(options.input, version_name, page_count, document_info);
    } else {
        print_info_text(options.input, version_name, page_count, document_info);
    }

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

} // namespace

void register_info(CLI::App& app, int& exit_code) {
    auto options = std::make_shared<InfoOptions>();
    auto* command = app.add_subcommand("info", "Display PDF file information");

    command->add_option("input", options->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    command->add_flag("--json", options->json_output, "Output results as JSON");

    command->callback([options, &exit_code]() {
        exit_code = process_info(*options);
    });
}

} // cli
} // vanillapdf
