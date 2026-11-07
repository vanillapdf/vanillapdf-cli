#ifndef _VANILLAPDF_CLI_ERROR_H
#define _VANILLAPDF_CLI_ERROR_H

#include <vanillapdf/c_vanillapdf_api.h>

#include <exception>
#include <stdexcept>
#include <string>

namespace vanillapdf {
namespace cli {

// Set by -v / --verbose: errors also print the failed call and the library's
// last error message
extern bool g_verbose;

// A vanillapdf C API call that failed, or a command that cannot proceed
class PdfException : public std::runtime_error {
public:
    explicit PdfException(error_type code, const std::string& context = std::string());

    // Raised by THROW_IF_NOT_SUCCESS, which also records the failed call
    PdfException(error_type code, const std::string& context, const char* call, const char* file, int line);

    error_type code() const noexcept;
    const std::string& call() const noexcept;
    const std::string& file() const noexcept;
    int line() const noexcept;

private:
    error_type m_code = VANILLAPDF_ERROR_SUCCESS;
    std::string m_call;
    std::string m_file;
    int m_line = 0;
};

// The library's last error message, or an empty string when there is none
std::string last_error_message();

// Print an error to stderr. For a PdfException in verbose mode, the failed call
// and the library's last error message are printed too, when available.
void report_error(const PdfException& ex);
void report_error(const std::exception& ex);

} // cli
} // vanillapdf

// Every handle is owned by a HandleGuard, so throwing here releases everything
// acquired so far. The exception is reported once, by main.
#define THROW_IF_NOT_SUCCESS(fn, context) \
do { \
    error_type vanillapdf_cli_result = (fn); \
    if (VANILLAPDF_ERROR_SUCCESS != vanillapdf_cli_result) { \
        throw vanillapdf::cli::PdfException(vanillapdf_cli_result, (context), #fn, __FILE__, __LINE__); \
    } \
} while (0)

#endif /* _VANILLAPDF_CLI_ERROR_H */
