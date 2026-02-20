#ifndef VANILLAPDF_CLI_UTILS_ENCODING_HPP
#define VANILLAPDF_CLI_UTILS_ENCODING_HPP

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

#include "utils/handles.hpp"

namespace vpdf {

inline char32_t read_utf16be_codepoint(std::string_view data, size_t& i) {
    uint16_t hi = (static_cast<uint8_t>(data[i]) << 8) | static_cast<uint8_t>(data[i + 1]);
    i += 2;

    if (hi < 0xD800 || hi > 0xDFFF) {
        return hi; // BMP character, no surrogate
    }

    if (hi > 0xDBFF) {
        return 0xFFFD; // unpaired low surrogate
    }

    // High surrogate — expect a following low surrogate
    if (i + 1 >= data.size()) {
        return 0xFFFD; // truncated
    }

    uint16_t lo = (static_cast<uint8_t>(data[i]) << 8) | static_cast<uint8_t>(data[i + 1]);
    if (lo < 0xDC00 || lo > 0xDFFF) {
        return 0xFFFD; // unpaired high surrogate
    }

    i += 2;
    return 0x10000 + ((static_cast<char32_t>(hi - 0xD800) << 10) | (lo - 0xDC00));
}

inline void encode_utf8(char32_t cp, std::string& out) {
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0x10FFFF) {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

inline std::string to_string_utf8(StringObjectHandle* handle) {
    BufferPtr buf;
    check(StringObject_GetValue(handle, buf.put()));
    string_type raw = nullptr;
    size_type raw_size = 0;
    check(Buffer_GetData(buf.get(), &raw, &raw_size));

    TextStringEncodingType encoding = TextStringEncodingType_Undefined;
    check(TextStringEncoding_Detect(raw, raw_size, &encoding));

    std::string_view data(raw, raw_size);
    std::string out;

    switch (encoding) {
        case TextStringEncodingType_PDFDocEncoding: {
            out.reserve(data.size());
            for (size_t i = 0; i < data.size(); ++i) {
                uint32_t cp = 0;
                check(PDFDocEncoding_ToUnicode(static_cast<uint8_t>(data[i]), &cp));
                encode_utf8(static_cast<char32_t>(cp), out);
            }
            break;
        }
        case TextStringEncodingType_UTF16BE: {
            std::string_view payload = data.substr(2); // skip BOM
            out.reserve(payload.size());
            size_t i = 0;
            while (i + 1 < payload.size()) {
                encode_utf8(read_utf16be_codepoint(payload, i), out);
            }
            break;
        }
        case TextStringEncodingType_UTF8: {
            out = std::string(data.substr(3)); // strip 3-byte BOM
            break;
        }
        default:
            throw std::logic_error("Unhandled TextStringEncodingType value");
    }

    return out;
}

} // namespace vpdf

#endif // VANILLAPDF_CLI_UTILS_ENCODING_HPP
