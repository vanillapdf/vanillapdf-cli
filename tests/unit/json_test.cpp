#include <ostream>
#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "utils/json.h"

using vanillapdf::cli::json_escape;

// Exact output for the characters that must be escaped.
TEST(JsonEscapeExamples, EscapesQuoteBackslashAndControls) {
    // Declared outside EXPECT_EQ: Visual Studio 2022 cannot stringize a raw string
    // that contains an escaped quote when it is a macro argument.
    const std::string quote = R"("a\"b")";
    const std::string backslash = R"("a\\b")";
    const std::string controls = R"("\t\n")";

    EXPECT_EQ(json_escape("a\"b"), quote);
    EXPECT_EQ(json_escape("a\\b"), backslash);
    EXPECT_EQ(json_escape("\t\n"), controls);
}

namespace {

// The parameter is wrapped in a struct rather than TestWithParam<std::string>:
// GCC 14 on arm64 crashes instantiating TEST_P for std::string (GCC PR116598,
// fixed in GCC 15), and a struct holding a std::string does not trigger it.
struct JsonEscapeCase {
    std::string input;
};

void PrintTo(const JsonEscapeCase& value, std::ostream* stream) {
    *stream << testing::PrintToString(value.input);
}

} // namespace

// Property: json_escape must produce a JSON string literal that a real parser
// decodes back to the original input.
class JsonEscape : public testing::TestWithParam<JsonEscapeCase> {};

TEST_P(JsonEscape, RoundTripsThroughJsonParser) {
    const std::string& input = GetParam().input;
    const std::string escaped = json_escape(input);
    const nlohmann::json parsed = nlohmann::json::parse(escaped);
    EXPECT_EQ(parsed.get<std::string>(), input);
}

INSTANTIATE_TEST_SUITE_P(
    Inputs, JsonEscape,
    testing::Values(
        JsonEscapeCase{std::string("")},
        JsonEscapeCase{std::string("Hello, world!")},
        JsonEscapeCase{std::string("digits 1234567890")},
        JsonEscapeCase{std::string("special !@#$%^&*() /")},
        JsonEscapeCase{std::string("quote \" backslash \\ mixed")},
        JsonEscapeCase{std::string("controls \b\f\n\r\t inline")},
        JsonEscapeCase{std::string("\x01\x1f", 2)},
        JsonEscapeCase{std::string("unicode caf\xc3\xa9")}));  // valid UTF-8 ("café")
