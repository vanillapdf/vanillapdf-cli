#include "utils/error.h"

#include <fmt/core.h>

#include <string_view>

namespace vanillapdf {
namespace cli {

bool g_verbose = false;

namespace {

std::string format_error_message(error_type code, const std::string& context) {
    if (context.empty()) {
        return fmt::format("PDF error (code: {})", code);
    }

    return fmt::format("{} (code: {})", context, code);
}

// Shorten a compiler-provided source path to the part from the src directory on,
// so verbose output does not leak the build machine's directory layout
std::string_view source_path(std::string_view file) {
    auto unix_position = file.rfind("src/");
    auto windows_position = file.rfind("src\\");

    if (unix_position != std::string_view::npos) {
        return file.substr(unix_position);
    }

    if (windows_position != std::string_view::npos) {
        return file.substr(windows_position);
    }

    return file;
}

} // namespace

PdfException::PdfException(error_type code, const std::string& context)
    : std::runtime_error(format_error_message(code, context)), m_code(code) {
}

PdfException::PdfException(error_type code, const std::string& context, const char* call, const char* file,
    int line)
    : std::runtime_error(format_error_message(code, context)), m_code(code), m_call(call), m_file(file),
    m_line(line) {
}

error_type PdfException::code() const noexcept {
    return m_code;
}

const std::string& PdfException::call() const noexcept {
    return m_call;
}

const std::string& PdfException::file() const noexcept {
    return m_file;
}

int PdfException::line() const noexcept {
    return m_line;
}

std::string last_error_message() {
    size_type message_length = 0;
    error_type length_result = Errors_GetLastErrorMessageLength(&message_length);

    // The length includes the terminating null character
    if (length_result != VANILLAPDF_ERROR_SUCCESS || message_length <= 1) {
        return std::string();
    }

    std::string message(message_length - 1, '\0');
    error_type message_result = Errors_GetLastErrorMessage(message.data(), message_length);
    if (message_result != VANILLAPDF_ERROR_SUCCESS) {
        return std::string();
    }

    return message;
}

void report_error(const PdfException& ex) {
    if (!g_verbose) {
        fmt::print(stderr, "Error: {}\n", ex.what());
        return;
    }

    auto detail = last_error_message();
    if (detail.empty()) {
        fmt::print(stderr, "Error: {}\n", ex.what());
    } else {
        fmt::print(stderr, "Error: {} ({})\n", ex.what(), detail);
    }

    if (!ex.call().empty()) {
        auto file = source_path(ex.file());
        fmt::print(stderr, "  {}\n  {{ {}:{} }}\n", ex.call(), file, ex.line());
    }
}

void report_error(const std::exception& ex) {
    fmt::print(stderr, "Error: {}\n", ex.what());
}

} // cli
} // vanillapdf
