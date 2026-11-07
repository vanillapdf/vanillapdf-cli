#include "utils/json.h"

#include <fmt/compile.h>
#include <fmt/format.h>

#include <iterator>

namespace vanillapdf {
namespace cli {

namespace {

// The short two-character JSON escape for a byte, or empty if it has none
std::string_view json_short_escape(unsigned char character) {
    switch (character) {
        case '"':  return R"(\")";
        case '\\': return R"(\\)";
        case '\b': return R"(\b)";
        case '\f': return R"(\f)";
        case '\n': return R"(\n)";
        case '\r': return R"(\r)";
        case '\t': return R"(\t)";
        default:   return std::string_view();
    }
}

} // namespace

std::string json_escape(std::string_view value) {
    // Builds into an fmt::memory_buffer (stack-backed, grows only on overflow)
    // to avoid the repeated reallocations of appending to a std::string
    fmt::memory_buffer buffer;
    buffer.push_back('"');

    for (unsigned char character : value) {
        auto short_escape = json_short_escape(character);
        if (!short_escape.empty()) {
            buffer.append(short_escape);
            continue;
        }

        if (character < 0x20) {
            auto output = std::back_inserter(buffer);
            fmt::format_to(output, FMT_COMPILE(R"(\u{:04x})"), character);
            continue;
        }

        buffer.push_back(static_cast<char>(character));
    }

    buffer.push_back('"');
    return fmt::to_string(buffer);
}

} // cli
} // vanillapdf
