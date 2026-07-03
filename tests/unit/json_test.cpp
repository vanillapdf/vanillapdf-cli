#include <string>
#include <string_view>
#include <vector>

#include <fmt/core.h>
#include <gtest/gtest.h>

#include "utils/json.hpp"

using vanillapdf::cli::json_escape;

namespace {

constexpr char BS = static_cast<char>(0x5c);  // backslash
constexpr char QT = static_cast<char>(0x22);  // double quote

// Wrap inner content in the surrounding quotes json_escape adds.
std::string quoted(std::string_view inner) {
    return fmt::format("{0}{1}{0}", QT, inner);
}

struct Case {
    std::string name;
    std::string input;
    std::string expected;
};

const std::vector<Case>& cases() {
    static const std::vector<Case> data = {
        {"Plain", "plain", quoted("plain")},
        {"Empty", "", quoted("")},
        {"DoubleQuote", fmt::format("a{}b", QT), quoted(fmt::format("a{}{}b", BS, QT))},
        {"Backslash", fmt::format("a{}b", BS), quoted(fmt::format("a{}{}b", BS, BS))},
        {"NamedControls",
         fmt::format("{}{}{}{}{}", char(0x08), char(0x0c), char(0x0a), char(0x0d), char(0x09)),
         quoted(fmt::format("{0}b{0}f{0}n{0}r{0}t", BS))},
        {"OtherControlsAsUnicode",
         fmt::format("{}{}", char(0x01), char(0x1f)),
         quoted(fmt::format("{0}u{1:04x}{0}u{2:04x}", BS, 0x01, 0x1f))},
        {"EmbeddedNul",
         std::string(1, char(0x00)),
         quoted(fmt::format("{}u{:04x}", BS, 0x00))},
        {"HighBytesPassThrough",  // UTF-8 "é" survives untouched
         fmt::format("{}{}", char(0xc3), char(0xa9)),
         quoted(fmt::format("{}{}", char(0xc3), char(0xa9)))},
    };
    return data;
}

} // namespace

class JsonEscape : public testing::TestWithParam<Case> {};

TEST_P(JsonEscape, ProducesExpectedLiteral) {
    const Case& c = GetParam();
    EXPECT_EQ(json_escape(c.input), c.expected);
}

INSTANTIATE_TEST_SUITE_P(
    Cases, JsonEscape, testing::ValuesIn(cases()),
    [](const testing::TestParamInfo<Case>& info) { return info.param.name; });
