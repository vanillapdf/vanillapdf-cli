#include <CLI/CLI.hpp>
#include <iostream>
#include <map>
#include <memory>

#include "commands/commands.hpp"
#include "utils/error.hpp"

namespace vpdf::commands {

enum class TextEncoding { Utf8, Utf16 };

struct InfoOptions {
    std::string input;
    TextEncoding encoding = TextEncoding::Utf8;
};

static void print_optional_field(DocumentInfoHandle* info, const char* label,
                                 error_type (*getter)(DocumentInfoHandle*, PdfTextStringHandle**),
                                 TextEncoding encoding) {
    PdfTextStringPtr text;
    error_type err = getter(info, text.put());
    if (err == VANILLAPDF_ERROR_SUCCESS) {
        std::cout << label << ": ";
        switch (encoding) {
            case TextEncoding::Utf8:
                std::cout << to_string_utf8(text.get());
                break;
            case TextEncoding::Utf16: {
                // UTF-16BE with BOM — write raw bytes for binary-safe piping
                auto raw = to_string_utf16(text.get());
                std::cout.write(raw.data(), static_cast<std::streamsize>(raw.size()));
                break;
            }
            default:
                throw std::logic_error("Unhandled TextEncoding value");
        }
        std::cout << "\n";
    } else if (err == VANILLAPDF_ERROR_OBJECT_MISSING) {
        std::cout << label << ": N/A\n";
    } else {
        std::cout << label << ": <unable to parse>";
        if (vpdf::g_verbose) {
            std::cout << " (" << vpdf::last_error_message() << ")";
        }
        std::cout << "\n";
    }
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

    std::cout << "File: " << opts.input << "\n";
    auto ver_it = version_names.find(pdf_version);
    if (ver_it != version_names.end()) {
        std::cout << "PDF Version: " << ver_it->second << "\n";
    }
    std::cout << "Pages: " << page_count << "\n";

    DocumentInfoPtr info;
    if (Document_GetDocumentInfo(document.get(), info.put()) == VANILLAPDF_ERROR_SUCCESS) {
        print_optional_field(info.get(), "Title",    DocumentInfo_GetTitleText,    opts.encoding);
        print_optional_field(info.get(), "Author",   DocumentInfo_GetAuthorText,   opts.encoding);
        print_optional_field(info.get(), "Subject",  DocumentInfo_GetSubjectText,  opts.encoding);
        print_optional_field(info.get(), "Keywords", DocumentInfo_GetKeywordsText, opts.encoding);
        print_optional_field(info.get(), "Creator",  DocumentInfo_GetCreatorText,  opts.encoding);
        print_optional_field(info.get(), "Producer", DocumentInfo_GetProducerText, opts.encoding);
    }

    return 0;
}

void register_info(CLI::App& app) {
    auto opts = std::make_shared<InfoOptions>();

    static const std::map<std::string, TextEncoding> encoding_map = {
        {"utf8",  TextEncoding::Utf8},
        {"utf16", TextEncoding::Utf16},
    };

    auto* cmd = app.add_subcommand("info", "Display PDF file information");
    cmd->add_option("input", opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    cmd->add_option("--encoding", opts->encoding,
                    "Output encoding for text fields: utf8 (default) or utf16 (UTF-16BE with BOM)")
        ->transform(CLI::CheckedTransformer(encoding_map, CLI::ignore_case));

    cmd->callback([opts]() {
        try {
            std::exit(run_info(*opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vpdf::commands
