#ifndef VANILLAPDF_CLI_UTILS_JSON_HPP
#define VANILLAPDF_CLI_UTILS_JSON_HPP

#include <iterator>
#include <string>
#include <string_view>

#include <fmt/compile.h>
#include <fmt/format.h>

namespace vanillapdf::cli {

// The short two-character JSON escape for a byte, or empty if it has none.
inline std::string_view json_short_escape(unsigned char c) {
    switch (c) {
        case '"':  return R"(\")";
        case '\\': return R"(\\)";
        case '\b': return R"(\b)";
        case '\f': return R"(\f)";
        case '\n': return R"(\n)";
        case '\r': return R"(\r)";
        case '\t': return R"(\t)";
        default:   return {};
    }
}

// Escape a string as a JSON string literal, including the surrounding quotes.
// Builds into an fmt::memory_buffer (stack-backed, grows only on overflow) to
// avoid the repeated reallocations of appending to a std::string. Remaining
// control characters (< 0x20) are emitted as \u00XX so the result is always
// valid JSON.
inline std::string json_escape(std::string_view s) {
    fmt::memory_buffer buf;
    buf.push_back('"');
    for (unsigned char c : s) {
        if (std::string_view esc = json_short_escape(c); !esc.empty()) {
            buf.append(esc);
            continue;
        }
        if (c < 0x20) {
            fmt::format_to(std::back_inserter(buf), FMT_COMPILE(R"(\u{:04x})"), c);
            continue;
        }
        buf.push_back(static_cast<char>(c));
    }
    buf.push_back('"');
    return fmt::to_string(buf);
}

} // namespace vanillapdf::cli

#endif // VANILLAPDF_CLI_UTILS_JSON_HPP
