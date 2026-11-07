#include "utils/encoding.h"
#include "utils/error.h"
#include "utils/handle_guard.h"

#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace vanillapdf {
namespace cli {

namespace {

const char32_t REPLACEMENT_CHARACTER = 0xFFFD;

uint16_t read_utf16be_unit(std::string_view data, size_t position) {
    auto high_byte = static_cast<uint8_t>(data[position]);
    auto low_byte = static_cast<uint8_t>(data[position + 1]);
    return static_cast<uint16_t>((high_byte << 8) | low_byte);
}

// Read one code point starting at position and advance position past it
char32_t read_utf16be_code_point(std::string_view data, size_t& position) {
    auto high_surrogate = read_utf16be_unit(data, position);
    position += 2;

    // BMP character, no surrogate
    if (high_surrogate < 0xD800 || high_surrogate > 0xDFFF) {
        return high_surrogate;
    }

    // Unpaired low surrogate, or a high surrogate truncated at the end
    if (high_surrogate > 0xDBFF || position + 1 >= data.size()) {
        return REPLACEMENT_CHARACTER;
    }

    auto low_surrogate = read_utf16be_unit(data, position);
    if (low_surrogate < 0xDC00 || low_surrogate > 0xDFFF) {
        return REPLACEMENT_CHARACTER;
    }

    position += 2;

    auto high_bits = static_cast<char32_t>(high_surrogate - 0xD800) << 10;
    auto low_bits = static_cast<char32_t>(low_surrogate - 0xDC00);
    return 0x10000 + (high_bits | low_bits);
}

void encode_utf8(char32_t code_point, std::string& output) {
    if (code_point <= 0x7F) {
        output.push_back(static_cast<char>(code_point));
    } else if (code_point <= 0x7FF) {
        output.push_back(static_cast<char>(0xC0 | (code_point >> 6)));
        output.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
    } else if (code_point <= 0xFFFF) {
        output.push_back(static_cast<char>(0xE0 | (code_point >> 12)));
        output.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
    } else if (code_point <= 0x10FFFF) {
        output.push_back(static_cast<char>(0xF0 | (code_point >> 18)));
        output.push_back(static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
    }
}

} // namespace

std::string to_string_utf8(StringObjectHandle* string_object) {
    BufferGuard buffer;
    THROW_IF_NOT_SUCCESS(StringObject_GetValue(string_object, buffer.out()), "Failed to get string value");

    string_type raw_data = nullptr;
    size_type raw_size = 0;
    THROW_IF_NOT_SUCCESS(Buffer_GetData(buffer, &raw_data, &raw_size), "Failed to read string data");

    TextStringEncodingType encoding = TextStringEncodingType_Undefined;
    THROW_IF_NOT_SUCCESS(TextStringEncoding_Detect(raw_data, raw_size, &encoding), "Failed to detect string encoding");

    std::string_view data(raw_data, raw_size);
    std::string result;

    switch (encoding) {
        case TextStringEncodingType_PDFDocEncoding: {
            result.reserve(data.size());
            for (size_t i = 0; i < data.size(); i++) {
                auto encoded_byte = static_cast<uint8_t>(data[i]);
                uint32_t code_point = 0;
                THROW_IF_NOT_SUCCESS(TextStringEncoding_PDFDocEncodingByteToUnicode(encoded_byte, &code_point),
                    "Failed to decode PDFDocEncoding character");

                auto code_point_converted = static_cast<char32_t>(code_point);
                encode_utf8(code_point_converted, result);
            }

            break;
        }
        case TextStringEncodingType_UTF16BE: {
            // Skip the 2-byte BOM
            auto payload = data.substr(2);
            result.reserve(payload.size());

            size_t position = 0;
            while (position + 1 < payload.size()) {
                auto code_point = read_utf16be_code_point(payload, position);
                encode_utf8(code_point, result);
            }

            break;
        }
        case TextStringEncodingType_UTF8: {
            // Strip the 3-byte BOM
            auto payload = data.substr(3);
            result = std::string(payload);
            break;
        }
        default:
            throw std::logic_error("Unhandled TextStringEncodingType value");
    }

    return result;
}

} // cli
} // vanillapdf
