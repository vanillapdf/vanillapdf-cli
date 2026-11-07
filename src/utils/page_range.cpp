#include "utils/page_range.hpp"

#include <algorithm>
#include <charconv>
#include <system_error>

#include <fmt/format.h>

namespace vanillapdf::cli {

namespace {

const std::string_view WHITESPACE = " \t";

std::string_view trim(std::string_view value) {

    auto first_position = value.find_first_not_of(WHITESPACE);
    if (first_position == std::string_view::npos) {
        return std::string_view();
    }

    auto last_position = value.find_last_not_of(WHITESPACE);
    auto trimmed_length = last_position - first_position + 1;
    return value.substr(first_position, trimmed_length);
}

// A single page: a 1-based number, first/begin, or last/end. The entry is the
// whole comma-separated entry the page came from, used in error messages.
std::size_t resolve_page(std::string_view page_text, std::string_view entry, std::size_t page_count) {

    std::size_t page_number = 0;

    if (page_text == "first" || page_text == "begin") {
        page_number = 1;
    } else if (page_text == "last" || page_text == "end") {
        page_number = page_count;
    } else {
        auto text_begin = page_text.data();
        auto text_end = page_text.data() + page_text.size();
        auto parse_result = std::from_chars(text_begin, text_end, page_number);

        bool parsed = (parse_result.ec == std::errc()) && (parse_result.ptr == text_end);
        if (page_text.empty() || !parsed) {
            throw page_range_error(fmt::format("invalid page '{}'", entry));
        }
    }

    if (page_number < 1 || page_number > page_count) {
        throw page_range_error(fmt::format("page '{}' is out of range (document has {} pages)",
            page_text, page_count));
    }

    return page_number;
}

// Add the pages one entry denotes (without its leading '!') to pages.
void add_entry_pages(std::string_view entry_body, std::string_view entry, std::size_t page_count,
    std::set<std::size_t>& pages) {

    if (entry_body == "odd" || entry_body == "even") {
        std::size_t page_number = (entry_body == "odd") ? 1 : 2;
        while (page_number <= page_count) {
            pages.insert(page_number);
            page_number += 2;
        }

        return;
    }

    auto dash_position = entry_body.find('-');
    if (dash_position == std::string_view::npos) {
        auto page_number = resolve_page(entry_body, entry, page_count);
        pages.insert(page_number);
        return;
    }

    auto from_text = trim(entry_body.substr(0, dash_position));
    auto to_text = trim(entry_body.substr(dash_position + 1));
    auto from_page = resolve_page(from_text, entry, page_count);
    auto to_page = resolve_page(to_text, entry, page_count);

    // A reversed range selects the same pages
    auto first_page = std::min(from_page, to_page);
    auto last_page = std::max(from_page, to_page);
    for (auto page_number = first_page; page_number <= last_page; page_number += 1) {
        pages.insert(page_number);
    }
}

} // namespace

std::set<std::size_t> resolve_page_range(std::string_view range, std::size_t page_count) {

    std::set<std::size_t> included_pages;
    std::set<std::size_t> excluded_pages;
    bool has_inclusion = false;

    std::size_t entry_start = 0;
    while (true) {
        auto comma_position = range.find(',', entry_start);
        auto entry_length = (comma_position == std::string_view::npos)
            ? std::string_view::npos
            : comma_position - entry_start;

        auto raw_entry = range.substr(entry_start, entry_length);
        auto entry = trim(raw_entry);
        if (entry.empty()) {
            throw page_range_error(fmt::format("empty entry in page range '{}'", range));
        }

        if (entry.front() == '!') {
            auto entry_body = trim(entry.substr(1));
            add_entry_pages(entry_body, entry, page_count, excluded_pages);
        } else {
            has_inclusion = true;
            add_entry_pages(entry, entry, page_count, included_pages);
        }

        if (comma_position == std::string_view::npos) {
            break;
        }

        entry_start = comma_position + 1;
    }

    // A range of only exclusions starts from every page
    if (!has_inclusion) {
        for (std::size_t page_number = 1; page_number <= page_count; page_number += 1) {
            included_pages.insert(page_number);
        }
    }

    for (auto excluded_page : excluded_pages) {
        included_pages.erase(excluded_page);
    }

    return included_pages;
}

} // namespace vanillapdf::cli
