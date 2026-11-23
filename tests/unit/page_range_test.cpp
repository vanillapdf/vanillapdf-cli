#include <cstddef>
#include <ostream>
#include <set>
#include <string>

#include <gtest/gtest.h>

#include "utils/page_range.h"

using vanillapdf::cli::PageRangeException;
using vanillapdf::cli::resolve_page_range;

using Pages = std::set<std::size_t>;

TEST(PageRange, SinglePages) {
    EXPECT_EQ(resolve_page_range("2", 5), Pages({2}));
    EXPECT_EQ(resolve_page_range("3,1,2", 5), Pages({1, 2, 3}));
}

TEST(PageRange, Ranges) {
    EXPECT_EQ(resolve_page_range("2-4", 5), Pages({2, 3, 4}));
    EXPECT_EQ(resolve_page_range("3-3", 5), Pages({3}));
    EXPECT_EQ(resolve_page_range("1-2,4-5", 5), Pages({1, 2, 4, 5}));
}

TEST(PageRange, ReversedRangeSelectsTheSamePages) {
    EXPECT_EQ(resolve_page_range("4-2", 5), resolve_page_range("2-4", 5));
    EXPECT_EQ(resolve_page_range("last-first", 3), Pages({1, 2, 3}));
}

TEST(PageRange, FirstAndLastKeywords) {
    EXPECT_EQ(resolve_page_range("first", 5), Pages({1}));
    EXPECT_EQ(resolve_page_range("last", 5), Pages({5}));
    EXPECT_EQ(resolve_page_range("3-last", 5), Pages({3, 4, 5}));
}

TEST(PageRange, BeginAndEndAreAliases) {
    EXPECT_EQ(resolve_page_range("begin", 5), resolve_page_range("first", 5));
    EXPECT_EQ(resolve_page_range("end", 5), resolve_page_range("last", 5));
    EXPECT_EQ(resolve_page_range("begin-end", 4), Pages({1, 2, 3, 4}));
}

TEST(PageRange, OddAndEven) {
    EXPECT_EQ(resolve_page_range("odd", 5), Pages({1, 3, 5}));
    EXPECT_EQ(resolve_page_range("even", 5), Pages({2, 4}));
    EXPECT_EQ(resolve_page_range("even", 1), Pages());
}

TEST(PageRange, ExclusionsFilterInclusions) {
    EXPECT_EQ(resolve_page_range("1-5,!3", 5), Pages({1, 2, 4, 5}));
    EXPECT_EQ(resolve_page_range("!3,1-5", 5), Pages({1, 2, 4, 5}));  // order doesn't matter
    EXPECT_EQ(resolve_page_range("odd,!1", 5), Pages({3, 5}));
    EXPECT_EQ(resolve_page_range("1-5,!2-4", 5), Pages({1, 5}));
    EXPECT_EQ(resolve_page_range("1-5,!even", 5), Pages({1, 3, 5}));
}

TEST(PageRange, OnlyExclusionsApplyToAllPages) {
    EXPECT_EQ(resolve_page_range("!1", 4), Pages({2, 3, 4}));
    EXPECT_EQ(resolve_page_range("!first,!last", 4), Pages({2, 3}));
    EXPECT_EQ(resolve_page_range("!odd", 4), Pages({2, 4}));
}

TEST(PageRange, DuplicatesAreMerged) {
    EXPECT_EQ(resolve_page_range("1,1", 3), Pages({1}));
    EXPECT_EQ(resolve_page_range("1-2,2-3", 3), Pages({1, 2, 3}));
}

TEST(PageRange, WhitespaceAroundEntriesIsIgnored) {
    EXPECT_EQ(resolve_page_range(" 1 , 3 - 4 , ! 4 ", 5), Pages({1, 3}));
}

TEST(PageRange, OutOfRangePagesAreErrors) {
    EXPECT_THROW(resolve_page_range("0", 3), PageRangeException);
    EXPECT_THROW(resolve_page_range("4", 3), PageRangeException);
    EXPECT_THROW(resolve_page_range("2-4", 3), PageRangeException);
    EXPECT_THROW(resolve_page_range("last", 0), PageRangeException);
}

namespace {

// The parameter is wrapped in a struct rather than TestWithParam<std::string>:
// GCC 14 on arm64 crashes instantiating TEST_P for std::string (GCC PR116598,
// fixed in GCC 15), and a struct holding a std::string does not trigger it.
struct MalformedRangeCase {
    std::string range;
};

void PrintTo(const MalformedRangeCase& value, std::ostream* stream) {
    *stream << testing::PrintToString(value.range);
}

} // namespace

// Property: every malformed range is rejected with PageRangeException.
class PageRangeMalformed : public testing::TestWithParam<MalformedRangeCase> {};

TEST_P(PageRangeMalformed, Throws) {
    EXPECT_THROW(resolve_page_range(GetParam().range, 10), PageRangeException);
}

INSTANTIATE_TEST_SUITE_P(
    Ranges, PageRangeMalformed,
    testing::Values(
        MalformedRangeCase{""},
        MalformedRangeCase{","},
        MalformedRangeCase{"1,"},
        MalformedRangeCase{"1,,2"},
        MalformedRangeCase{"-3"},
        MalformedRangeCase{"3-"},
        MalformedRangeCase{"1-2-3"},
        MalformedRangeCase{"!"},
        MalformedRangeCase{"!!1"},
        MalformedRangeCase{"+1"},
        MalformedRangeCase{"1.5"},
        MalformedRangeCase{"abc"},
        MalformedRangeCase{"First"},
        MalformedRangeCase{"odd-even"},
        MalformedRangeCase{"99999999999999999999999"}));
