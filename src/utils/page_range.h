#ifndef _VANILLAPDF_CLI_PAGE_RANGE_H
#define _VANILLAPDF_CLI_PAGE_RANGE_H

#include <cstddef>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>

namespace vanillapdf {
namespace cli {

// A malformed page range, or one that names a page outside the document
class PageRangeException : public std::invalid_argument {
public:
    explicit PageRangeException(const std::string& message);
};

// Resolve a page range against a document of `page_count` pages into the set
// of selected 1-based page numbers. The range is a comma-separated list of:
//
//   7            a single page
//   2-5          an inclusive range (5-2 selects the same pages)
//   first, last  the first / last page, usable anywhere a number is
//                (begin and end are aliases)
//   odd, even    every odd / even page
//   !entry       exclude the pages of entry from the selection
//
// A range of only exclusions applies them to every page ("!1" = all but the
// first). Throws PageRangeException on bad syntax or a page outside 1..page_count.
std::set<std::size_t> resolve_page_range(std::string_view range, std::size_t page_count);

} // cli
} // vanillapdf

#endif /* _VANILLAPDF_CLI_PAGE_RANGE_H */
