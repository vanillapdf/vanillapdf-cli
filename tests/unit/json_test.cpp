#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "utils/json.h"

using vanillapdf::cli::json_escape;

// Exact output for the characters that must be escaped.
TEST(JsonEscapeExamples, EscapesQuoteBackslashAndControls) {
    EXPECT_EQ(json_escape("a\"b"), R"("a\"b")");
    EXPECT_EQ(json_escape("a\\b"), R"("a\\b")");
    EXPECT_EQ(json_escape("\t\n"), R"("\t\n")");
}

// Property: json_escape must produce a JSON string literal that a real parser
// decodes back to the original input.
class JsonEscape : public testing::TestWithParam<std::string> {};

TEST_P(JsonEscape, RoundTripsThroughJsonParser) {
    const std::string& input = GetParam();
    const std::string escaped = json_escape(input);
    const nlohmann::json parsed = nlohmann::json::parse(escaped);
    EXPECT_EQ(parsed.get<std::string>(), input);
}

INSTANTIATE_TEST_SUITE_P(
    Inputs, JsonEscape,
    testing::Values(
        std::string(""),
        std::string("Hello, world!"),
        std::string("digits 1234567890"),
        std::string("special !@#$%^&*() /"),
        std::string("quote \" backslash \\ mixed"),
        std::string("controls \b\f\n\r\t inline"),
        std::string("\x01\x1f", 2),
        std::string("unicode caf\xc3\xa9")));  // valid UTF-8 ("café")
