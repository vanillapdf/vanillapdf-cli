#include <CLI/CLI.hpp>
#include <iostream>
#include <string>

#include "commands/commands.hpp"
#include "utils/error.hpp"
#include "utils/handles.hpp"

namespace vanillapdf::cli::commands {

struct ValidateOptions {
    std::string input;
    bool strict = false;
    bool json_output = false;
};

static const char* severity_string(FileStructureIssueSeverityType s) {
    switch (s) {
    case FileStructureIssueSeverity_Info:    return "Info";
    case FileStructureIssueSeverity_Warning: return "Warning";
    case FileStructureIssueSeverity_Error:   return "Error";
    default:                                 return "Undefined";
    }
}

static const char* issue_code_name(FileStructureIssueCodeType c) {
    switch (c) {
    case FileStructureIssueCode_HeaderMissing:                        return "HeaderMissing";
    case FileStructureIssueCode_HeaderVersionUndefined:               return "HeaderVersionUndefined";
    case FileStructureIssueCode_TrailerMissingSize:                   return "TrailerMissingSize";
    case FileStructureIssueCode_TrailerMissingRoot:                   return "TrailerMissingRoot";
    case FileStructureIssueCode_TrailerSizeMismatch:                  return "TrailerSizeMismatch";
    case FileStructureIssueCode_TrailerMissing:                       return "TrailerMissing";
    case FileStructureIssueCode_XrefUsedEntryBadOffset:               return "XrefUsedEntryBadOffset";
    case FileStructureIssueCode_XrefUsedEntryOffsetExceedsFileSize:   return "XrefUsedEntryOffsetExceedsFileSize";
    case FileStructureIssueCode_XrefCompressedEntryMissingObjectStream: return "XrefCompressedEntryMissingObjectStream";
    case FileStructureIssueCode_XrefFreeEntryInvalidNextFree:         return "XrefFreeEntryInvalidNextFree";
    case FileStructureIssueCode_TrailerRootReferenceNotInXref:        return "TrailerRootReferenceNotInXref";
    case FileStructureIssueCode_TrailerInfoReferenceNotInXref:        return "TrailerInfoReferenceNotInXref";
    case FileStructureIssueCode_XrefChainEmpty:                       return "XrefChainEmpty";
    default:                                                          return "Undefined";
    }
}

static std::string issue_message(FileStructureIssueHandle* issue) {
    BufferPtr buf;
    check(FileStructureIssue_GetMessage(issue, buf.put()), "Failed to get issue message");
    string_type data = nullptr;
    size_type size = 0;
    check(Buffer_GetData(buf.get(), &data, &size), "Failed to read issue message");
    return std::string(data, size);
}

static std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    out += '"';
    for (char c : s) {
        if (c == '"')       out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else                out += c;
    }
    out += '"';
    return out;
}

static int run_validate(const ValidateOptions& opts) {
    FilePtr file;
    check(File_Open(opts.input.c_str(), file.put()), "Failed to open file");
    check(File_Initialize(file.get()), "Failed to initialize file");

    FileStructureValidationResultPtr result;
    check(FileStructureValidator_Validate(file.get(), result.put()), "Failed to validate file structure");

    boolean_type is_valid = VANILLAPDF_RV_FALSE;
    check(FileStructureValidationResult_IsValid(result.get(), &is_valid), "Failed to check validity");

    size_type issue_count = 0;
    check(FileStructureValidationResult_GetIssueCount(result.get(), &issue_count), "Failed to get issue count");

    size_type error_count = 0;
    check(FileStructureValidationResult_GetErrorCount(result.get(), &error_count), "Failed to get error count");

    size_type warning_count = 0;
    check(FileStructureValidationResult_GetWarningCount(result.get(), &warning_count), "Failed to get warning count");

    if (opts.json_output) {
        std::cout << "{\n";
        std::cout << "  \"file\": " << json_escape(opts.input) << ",\n";
        std::cout << "  \"valid\": " << (is_valid != VANILLAPDF_RV_FALSE ? "true" : "false") << ",\n";
        std::cout << "  \"error_count\": " << error_count << ",\n";
        std::cout << "  \"warning_count\": " << warning_count << ",\n";
        std::cout << "  \"issues\": [";

        for (size_type i = 0; i < issue_count; ++i) {
            FileStructureIssuePtr issue;
            check(FileStructureValidationResult_GetIssueAt(result.get(), i, issue.put()), "Failed to get issue");

            FileStructureIssueSeverityType severity = FileStructureIssueSeverity_Undefined;
            check(FileStructureIssue_GetSeverity(issue.get(), &severity), "Failed to get issue severity");

            FileStructureIssueCodeType code = FileStructureIssueCode_Undefined;
            check(FileStructureIssue_GetCode(issue.get(), &code), "Failed to get issue code");

            biguint_type obj_number = 0;
            check(FileStructureIssue_GetObjectNumber(issue.get(), &obj_number), "Failed to get object number");

            ushort_type gen_number = 0;
            check(FileStructureIssue_GetGenerationNumber(issue.get(), &gen_number), "Failed to get generation number");

            if (i > 0) std::cout << ",";
            std::cout << "\n    {\n";
            std::cout << "      \"severity\": " << json_escape(severity_string(severity)) << ",\n";
            std::cout << "      \"code\": " << static_cast<int>(code) << ",\n";
            std::cout << "      \"name\": " << json_escape(issue_code_name(code)) << ",\n";
            std::cout << "      \"message\": " << json_escape(issue_message(issue.get()));
            if (obj_number != 0) {
                std::cout << ",\n      \"object\": " << obj_number;
                std::cout << ",\n      \"generation\": " << gen_number;
            }
            std::cout << "\n    }";
        }

        std::cout << "\n  ]\n}\n";
    } else {
        std::cout << "File: " << opts.input << "\n";
        std::cout << "Valid: " << (is_valid != VANILLAPDF_RV_FALSE ? "yes" : "no") << "\n";
        std::cout << "Errors: " << error_count << "  Warnings: " << warning_count << "\n";

        for (size_type i = 0; i < issue_count; ++i) {
            FileStructureIssuePtr issue;
            check(FileStructureValidationResult_GetIssueAt(result.get(), i, issue.put()), "Failed to get issue");

            FileStructureIssueSeverityType severity = FileStructureIssueSeverity_Undefined;
            check(FileStructureIssue_GetSeverity(issue.get(), &severity), "Failed to get issue severity");

            FileStructureIssueCodeType code = FileStructureIssueCode_Undefined;
            check(FileStructureIssue_GetCode(issue.get(), &code), "Failed to get issue code");

            biguint_type obj_number = 0;
            check(FileStructureIssue_GetObjectNumber(issue.get(), &obj_number), "Failed to get object number");

            ushort_type gen_number = 0;
            check(FileStructureIssue_GetGenerationNumber(issue.get(), &gen_number), "Failed to get generation number");

            std::cout << "[" << severity_string(severity) << "] "
                      << issue_code_name(code) << " (" << static_cast<int>(code) << "): "
                      << issue_message(issue.get());
            if (obj_number != 0) {
                std::cout << " (object " << obj_number << " " << gen_number << " R)";
            }
            std::cout << "\n";
        }
    }

    bool has_errors = (error_count > 0);
    bool has_warnings = (warning_count > 0);
    return (has_errors || (opts.strict && has_warnings)) ? 1 : 0;
}

void register_validate(CLI::App& app) {
    auto opts = std::make_shared<ValidateOptions>();
    auto* cmd = app.add_subcommand("validate", "Validate PDF file structure");
    cmd->add_option("input", opts->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    cmd->add_flag("--strict", opts->strict,
                  "Exit with failure if any warnings are found (default: only errors cause failure)");
    cmd->add_flag("--json", opts->json_output, "Output results as JSON");

    cmd->callback([opts]() {
        try {
            std::exit(run_validate(*opts));
        } catch (const pdf_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::exit(1);
        }
    });
}

} // namespace vanillapdf::cli::commands
