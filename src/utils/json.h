#ifndef _VANILLAPDF_CLI_JSON_H
#define _VANILLAPDF_CLI_JSON_H

#include <string>
#include <string_view>

namespace vanillapdf {
namespace cli {

// Escape a string as a JSON string literal, including the surrounding quotes.
// Control characters without a short escape (< 0x20) are emitted as \u00XX,
// so the result is always valid JSON.
std::string json_escape(std::string_view value);

} // cli
} // vanillapdf

#endif /* _VANILLAPDF_CLI_JSON_H */
