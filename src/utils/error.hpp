#ifndef VANILLAPDF_CLI_UTILS_ERROR_HPP
#define VANILLAPDF_CLI_UTILS_ERROR_HPP

#include <vanillapdf/c_vanillapdf_api.h>
#include <stdexcept>
#include <string>

namespace vanillapdf::cli {

inline bool g_verbose = false;

class pdf_error : public std::runtime_error {
public:
    explicit pdf_error(error_type code, std::string context = "")
        : std::runtime_error(
              context.empty()
                  ? "PDF error (code: " + std::to_string(code) + ")"
                  : context + " (code: " + std::to_string(code) + ")")
        , code_(code) {}

    error_type code() const noexcept { return code_; }

private:
    error_type code_;
};

inline void check(error_type result, const std::string& context = "") {
    if (result != VANILLAPDF_ERROR_SUCCESS) {
        throw pdf_error(result, context);
    }
}

inline std::string last_error_message() {
    size_type len = 0;
    check(Errors_GetLastErrorMessageLength(&len), "Failed to get error message length");
    if (len <= 1) return {};
    std::string text(len - 1, '\0');
    check(Errors_GetLastErrorMessage(text.data(), len), "Failed to get error message");
    return text;
}

} // namespace vanillapdf::cli

#endif // VANILLAPDF_CLI_UTILS_ERROR_HPP
