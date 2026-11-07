#include "commands/commands.h"
#include "utils/error.h"
#include "utils/handle_guard.h"
#include "utils/json.h"

#include <CLI/CLI.hpp>

#include <iostream>
#include <memory>
#include <string>

namespace vanillapdf {
namespace cli {

namespace {

struct ValidateOptions {
    std::string input;
    bool strict = false;
    bool json_output = false;
};

// One file structure issue, read out of its handle
struct ValidationIssue {
    FileStructureIssueSeverityType severity = FileStructureIssueSeverity_Undefined;
    FileStructureIssueCodeType code = FileStructureIssueCode_Undefined;
    std::string message;
    biguint_type object_number = 0;
    ushort_type generation_number = 0;
};

const char* severity_name(FileStructureIssueSeverityType severity) {
    switch (severity) {
        case FileStructureIssueSeverity_Info:    return "Info";
        case FileStructureIssueSeverity_Warning: return "Warning";
        case FileStructureIssueSeverity_Error:   return "Error";
        default:                                 return "Undefined";
    }
}

const char* issue_code_name(FileStructureIssueCodeType code) {
    switch (code) {
        case FileStructureIssueCode_HeaderMissing:                          return "HeaderMissing";
        case FileStructureIssueCode_HeaderVersionUndefined:                 return "HeaderVersionUndefined";
        case FileStructureIssueCode_TrailerMissingSize:                     return "TrailerMissingSize";
        case FileStructureIssueCode_TrailerMissingRoot:                     return "TrailerMissingRoot";
        case FileStructureIssueCode_TrailerSizeMismatch:                    return "TrailerSizeMismatch";
        case FileStructureIssueCode_TrailerMissing:                         return "TrailerMissing";
        case FileStructureIssueCode_XrefUsedEntryBadOffset:                 return "XrefUsedEntryBadOffset";
        case FileStructureIssueCode_XrefUsedEntryOffsetExceedsFileSize:     return "XrefUsedEntryOffsetExceedsFileSize";
        case FileStructureIssueCode_XrefCompressedEntryMissingObjectStream: return "XrefCompressedEntryMissingObjectStream";
        case FileStructureIssueCode_XrefFreeEntryInvalidNextFree:           return "XrefFreeEntryInvalidNextFree";
        case FileStructureIssueCode_TrailerRootReferenceNotInXref:          return "TrailerRootReferenceNotInXref";
        case FileStructureIssueCode_TrailerInfoReferenceNotInXref:          return "TrailerInfoReferenceNotInXref";
        case FileStructureIssueCode_XrefChainEmpty:                         return "XrefChainEmpty";
        default:                                                            return "Undefined";
    }
}

ValidationIssue read_issue(FileStructureValidationResultHandle* validation_result, size_type index) {
    FileStructureIssueGuard issue_handle;
    THROW_IF_NOT_SUCCESS(FileStructureValidationResult_GetIssueAt(validation_result, index, issue_handle.out()),
        "Failed to get issue");

    ValidationIssue issue;
    THROW_IF_NOT_SUCCESS(FileStructureIssue_GetSeverity(issue_handle, &issue.severity), "Failed to get issue severity");
    THROW_IF_NOT_SUCCESS(FileStructureIssue_GetCode(issue_handle, &issue.code), "Failed to get issue code");
    THROW_IF_NOT_SUCCESS(FileStructureIssue_GetObjectNumber(issue_handle, &issue.object_number),
        "Failed to get object number");
    THROW_IF_NOT_SUCCESS(FileStructureIssue_GetGenerationNumber(issue_handle, &issue.generation_number),
        "Failed to get generation number");

    BufferGuard message_buffer;
    THROW_IF_NOT_SUCCESS(FileStructureIssue_GetMessage(issue_handle, message_buffer.out()),
        "Failed to get issue message");

    string_type message_data = nullptr;
    size_type message_size = 0;
    THROW_IF_NOT_SUCCESS(Buffer_GetData(message_buffer, &message_data, &message_size), "Failed to read issue message");
    issue.message = std::string(message_data, message_size);

    return issue;
}

void print_issue_json(const ValidationIssue& issue) {
    auto severity = severity_name(issue.severity);
    auto code_name = issue_code_name(issue.code);
    auto code_converted = static_cast<int>(issue.code);

    std::cout << "\n    {\n";
    std::cout << "      \"severity\": " << json_escape(severity) << ",\n";
    std::cout << "      \"code\": " << code_converted << ",\n";
    std::cout << "      \"name\": " << json_escape(code_name) << ",\n";
    std::cout << "      \"message\": " << json_escape(issue.message);
    if (issue.object_number != 0) {
        std::cout << ",\n      \"object\": " << issue.object_number;
        std::cout << ",\n      \"generation\": " << issue.generation_number;
    }

    std::cout << "\n    }";
}

void print_issue_text(const ValidationIssue& issue) {
    auto severity = severity_name(issue.severity);
    auto code_name = issue_code_name(issue.code);
    auto code_converted = static_cast<int>(issue.code);

    std::cout << "[" << severity << "] " << code_name << " (" << code_converted << "): " << issue.message;
    if (issue.object_number != 0) {
        std::cout << " (object " << issue.object_number << " " << issue.generation_number << " R)";
    }

    std::cout << "\n";
}

int process_validate(const ValidateOptions& options) {
    FileGuard file;
    THROW_IF_NOT_SUCCESS(File_Open(options.input.c_str(), file.out()), "Failed to open file");
    THROW_IF_NOT_SUCCESS(File_Initialize(file), "Failed to initialize file");

    FileStructureValidationResultGuard validation_result;
    THROW_IF_NOT_SUCCESS(FileStructureValidator_Validate(file, validation_result.out()),
        "Failed to validate file structure");

    boolean_type is_valid = VANILLAPDF_RV_FALSE;
    THROW_IF_NOT_SUCCESS(FileStructureValidationResult_IsValid(validation_result, &is_valid),
        "Failed to check validity");

    size_type issue_count = 0;
    THROW_IF_NOT_SUCCESS(FileStructureValidationResult_GetIssueCount(validation_result, &issue_count),
        "Failed to get issue count");

    size_type error_count = 0;
    THROW_IF_NOT_SUCCESS(FileStructureValidationResult_GetErrorCount(validation_result, &error_count),
        "Failed to get error count");

    size_type warning_count = 0;
    THROW_IF_NOT_SUCCESS(FileStructureValidationResult_GetWarningCount(validation_result, &warning_count),
        "Failed to get warning count");

    bool valid = (is_valid != VANILLAPDF_RV_FALSE);

    if (options.json_output) {
        std::cout << "{\n";
        std::cout << "  \"file\": " << json_escape(options.input) << ",\n";
        std::cout << "  \"valid\": " << (valid ? "true" : "false") << ",\n";
        std::cout << "  \"error_count\": " << error_count << ",\n";
        std::cout << "  \"warning_count\": " << warning_count << ",\n";
        std::cout << "  \"issues\": [";

        for (size_type i = 0; i < issue_count; i++) {
            auto issue = read_issue(validation_result, i);
            if (i > 0) {
                std::cout << ",";
            }

            print_issue_json(issue);
        }

        std::cout << "\n  ]\n}\n";
    } else {
        std::cout << "File: " << options.input << "\n";
        std::cout << "Valid: " << (valid ? "yes" : "no") << "\n";
        std::cout << "Errors: " << error_count << "  Warnings: " << warning_count << "\n";

        for (size_type i = 0; i < issue_count; i++) {
            auto issue = read_issue(validation_result, i);
            print_issue_text(issue);
        }
    }

    bool has_errors = (error_count > 0);
    bool has_warnings = (warning_count > 0);
    if (has_errors || (options.strict && has_warnings)) {
        return VANILLAPDF_CLI_ERROR_FAILURE;
    }

    return VANILLAPDF_CLI_ERROR_SUCCESS;
}

} // namespace

void register_validate(CLI::App& app, int& exit_code) {
    auto options = std::make_shared<ValidateOptions>();
    auto* command = app.add_subcommand("validate", "Validate PDF file structure");

    command->add_option("input", options->input, "Input PDF file")
        ->required()
        ->check(CLI::ExistingFile);
    command->add_flag("--strict", options->strict,
        "Exit with failure if any warnings are found (default: only errors cause failure)");
    command->add_flag("--json", options->json_output, "Output results as JSON");

    command->callback([options, &exit_code]() {
        exit_code = process_validate(*options);
    });
}

} // cli
} // vanillapdf
