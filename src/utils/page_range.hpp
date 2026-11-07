#ifndef VANILLAPDF_CLI_UTILS_PAGE_RANGE_HPP
#define VANILLAPDF_CLI_UTILS_PAGE_RANGE_HPP

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <vector>

#include <fmt/format.h>

namespace vanillapdf::cli {

// A malformed page range, or one that names a page outside the document.
class page_range_error : public std::invalid_argument {
public:
    using std::invalid_argument::invalid_argument;
};

namespace detail {

inline std::string_view trim(std::string_view s) {
    const auto first = s.find_first_not_of(" \t");
    if (first == std::string_view::npos) {
        return {};
    }
    const auto last = s.find_last_not_of(" \t");
    return s.substr(first, last - first + 1);
}

// A single page: a 1-based number, first/begin, or last/end.
inline std::size_t resolve_page(std::string_view token, std::string_view item, std::size_t total) {
    std::size_t page = 0;
    if (token == "first" || token == "begin") {
        page = 1;
    } else if (token == "last" || token == "end") {
        page = total;
    } else {
        const char* end = token.data() + token.size();
        const auto [ptr, ec] = std::from_chars(token.data(), end, page);
        if (token.empty() || ec != std::errc() || ptr != end) {
            throw page_range_error(fmt::format("invalid page '{}'", item));
        }
    }

    if (page < 1 || page > total) {
        throw page_range_error(
            fmt::format("page '{}' is out of range (document has {} pages)", token, total));
    }
    return page;
}

// Append the pages one item denotes (without its leading '!') to out.
inline void expand_item(std::string_view term, std::string_view item, std::size_t total,
                        std::vector<std::size_t>& out) {
    if (term == "odd" || term == "even") {
        for (std::size_t page = (term == "odd") ? 1 : 2; page <= total; page += 2) {
            out.push_back(page);
        }
        return;
    }

    const auto dash = term.find('-');
    if (dash == std::string_view::npos) {
        out.push_back(resolve_page(term, item, total));
        return;
    }

    const std::size_t from = resolve_page(trim(term.substr(0, dash)), item, total);
    const std::size_t to = resolve_page(trim(term.substr(dash + 1)), item, total);
    if (from <= to) {
        for (std::size_t page = from; page <= to; ++page) {
            out.push_back(page);
        }
    } else {
        for (std::size_t page = from; page >= to; --page) {
            out.push_back(page);
        }
    }
}

} // namespace detail

// Resolve a page-range spec against a document of `total` pages into 1-based
// page numbers, in the order written. The spec is a comma-separated list of:
//
//   7            a single page
//   2-5          an inclusive range; descending when reversed (5-2, last-first)
//   first, last  the first / last page, usable anywhere a number is
//                (begin and end are aliases)
//   odd, even    every odd / even page
//   !item        exclude the pages of item from the result
//
// A spec of only exclusions applies them to every page ("!1" = all but the
// first). Duplicates are kept; callers that operate on a set of pages dedupe.
// Throws page_range_error on bad syntax or a page outside 1..total.
inline std::vector<std::size_t> resolve_page_range(std::string_view spec, std::size_t total) {
    std::vector<std::size_t> included;
    std::vector<std::size_t> excluded;
    bool has_inclusion = false;

    std::size_t start = 0;
    while (true) {
        const auto comma = spec.find(',', start);
        const auto item = detail::trim(spec.substr(start, comma == std::string_view::npos
                                                              ? std::string_view::npos
                                                              : comma - start));
        if (item.empty()) {
            throw page_range_error(fmt::format("empty entry in page range '{}'", spec));
        }

        if (item.front() == '!') {
            detail::expand_item(detail::trim(item.substr(1)), item, total, excluded);
        } else {
            has_inclusion = true;
            detail::expand_item(item, item, total, included);
        }

        if (comma == std::string_view::npos) {
            break;
        }
        start = comma + 1;
    }

    if (!has_inclusion) {
        for (std::size_t page = 1; page <= total; ++page) {
            included.push_back(page);
        }
    }

    if (!excluded.empty()) {
        std::sort(excluded.begin(), excluded.end());
        included.erase(std::remove_if(included.begin(), included.end(),
                                      [&excluded](std::size_t page) {
                                          return std::binary_search(excluded.begin(),
                                                                    excluded.end(), page);
                                      }),
                       included.end());
    }

    return included;
}

} // namespace vanillapdf::cli

#endif // VANILLAPDF_CLI_UTILS_PAGE_RANGE_HPP
