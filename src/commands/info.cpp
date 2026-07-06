#include <CLI/CLI.hpp>
#include <iostream>
#include <map>
#include <memory>

#include "commands/commands.hpp"
#include "utils/encoding.hpp"
#include "utils/json.hpp"

namespace vanillapdf::cli::commands {

struct InfoOptions {
    std::string input;
    bool json_output = false;
};

// The document information dictionary fields, with their human-readable label
// and JSON key, rendered in this order by both output modes.
struct InfoField {
    const char* label;
    const char* key;
    error_type (*getter)(DocumentInfoHandle*, StringObjectHandle**);
};

static const InfoField info_fields[] = {
    {"Title",    "title",    DocumentInfo_GetTitle},
    {"Author",   "author",   DocumentInfo_GetAuthor},
    {"Subject",  "subject",  DocumentInfo_GetSubject},
    {"Keywords", "keywords", DocumentInfo_GetKeywords},
    {"Creator",  "creator",  DocumentInfo_GetCreator},
    {"Producer", "producer", DocumentInfo_GetProducer},
};

static void print_optional_field(DocumentInfoHandle* info, const InfoField& field) {
    StringObjectPtr text;
    error_type err = field.getter(info, text.put());
    if (err == VANILLAPDF_ERROR_SUCCESS) {
        std::cout << field.label << ": " << to_string_utf8(text.get()) << "\n";
    } else if (err == VANILLAPDF_ERROR_OBJECT_MISSING) {
        std::cout << field.label << ": N/A\n";
    } else {
        std::cout << field.label << ": <unable to parse>";
        if (vanillapdf::cli::g_verbose) {
            std::cout << " (" << vanillapdf::cli::last_error_message() << ")";
        }
        std::cout << "\n";
    }
}

static void print_info_text(const std::string& file, const char* version,
                            size_type pages, DocumentInfoHandle* info) {
    std::cout << "File: " << file << "\n";
    if (version) {
        std::cout << "PDF Version: " << version << "\n";
    }
    std::cout << "Pages: " << pages << "\n";
    if (info) {
        for (const auto& field : info_fields) {
            print_optional_field(info, field);
        }
    }
}

static void print_info_json(const std::string& file, const char* version,
                            size_type pages, DocumentInfoHandle* info) {
    std::cout << "{\n";
    std::cout << "  \"file\": " << json_escape(file) << ",\n";
    std::cout << "  \"pdf_version\": ";
    if (version) {
        std::cout << json_escape(version);
    } else {
        std::cout << "null";
    }
    std::cout << ",\n";
    std::cout << "  \"pages\": " << pages;

    // A missing or unparseable field is emitted as null so the shape is stable.
    for (const auto& field : info_fields) {
        std::cout << ",\n  " << json_escape(field.key) << ": ";
        StringObjectPtr text;
        if (info && field.getter(info, text.put()) == VANILLAPDF_ERROR_SUCCESS) {
            std::cout << json_escape(to_string_utf8(text.get()));
        } else {
            std::cout << "null";
        }
    }

    std::cout << "\n}\n";
}

static int run_info(const InfoOptions& opts) {
    DocumentPtr document;
    check(Document_Open(opts.input.c_str(), document.put()), "Failed to open document");

    CatalogPtr catalog;
    check(Document_GetCatalog(document.get(), catalog.put()), "Failed to get catalog");

    PageTreePtr page_tree;
    check(Catalog_GetPages(catalog.get(), page_tree.put()), "Failed to get page tree");

    size_type page_count = 0;
    check(PageTree_GetPageCount(page_tree.get(), &page_count), "Failed to get page count");

    // PDF version from file header
    FilePtr file;
    PDFVersion pdf_version = PDFVersion_Undefined;
    if (File_Open(opts.input.c_str(), file.put()) == VANILLAPDF_ERROR_SUCCESS) {
        File_GetVersion(file.get(), &pdf_version);
    }

    static const std::map<PDFVersion, const char*> version_names = {
        {PDFVersion_10, "1.0"}, {PDFVersion_11, "1.1"}, {PDFVersion_12, "1.2"},
        {PDFVersion_13, "1.3"}, {PDFVersion_14, "1.4"}, {PDFVersion_15, "1.5"},
        {PDFVersion_16, "1.6"}, {PDFVersion_17, "1.7"}, {PDFVersion_20, "2.0"},
    };
    auto ver_it = version_names.find(pdf_version);
    const char* version_str = (ver_it != version_names.end()) ? ver_it->second : nullptr;

    DocumentInfoPtr info;
    DocumentInfoHandle* info_handle = nullptr;
    if (Document_GetDocumentInfo(document.get(), info.put()) == VANILLAPDF_ERROR_SUCCESS) {
        info_handle = info.get();
    }

    if (opts.json_output) {
        print_info_json(opts.input, version_str, page_count, info_handle);
    } else {
        print_info_text(opts.input, version_str, page_count, info_handle);
    }

    return 0;
}

void register_info(CLI::App& app) {
    auto opts = std::make_shared<InfoOptions>();

    auto* cmd = app.add_subcommand("info", "Display PDF file information");
    cmd->add_option("input", opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    cmd->add_flag("--json", opts->json_output, "Output results as JSON");

    cmd->callback([opts]() {
        try {
            std::exit(run_info(*opts));
        } catch (const pdf_error& e) {
            report_error(e);
            std::exit(1);
        }
    });
}

} // namespace vanillapdf::cli::commands
