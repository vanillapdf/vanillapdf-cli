#ifndef VANILLAPDF_CLI_UTILS_ERROR_HPP
#define VANILLAPDF_CLI_UTILS_ERROR_HPP

#include <vanillapdf/c_vanillapdf_api.h>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace vpdf {

inline bool g_verbose = false;

class pdf_error : public std::runtime_error {
public:
    explicit pdf_error(error_type code, std::string context = "")
        : std::runtime_error(
              context.empty()
                  ? "PDF error (code: " + std::to_string(code) + ")"
                  : context + " (code: " + std::to_string(code) + ")")
        , code_(code) {}

    error_type code() const noexcept { return code_; }

private:
    error_type code_;
};

inline void check(error_type result, const std::string& context = "") {
    if (result != VANILLAPDF_ERROR_SUCCESS) {
        throw pdf_error(result, context);
    }
}

template <typename T, error_type (CALLING_CONVENTION *ReleaseFn)(T*)>
class Handle {
public:
    Handle() = default;
    explicit Handle(T* ptr) : ptr_(ptr) {}

    ~Handle() {
        if (ptr_) {
            ReleaseFn(ptr_);
        }
    }

    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

    Handle(Handle&& other) noexcept : ptr_(other.ptr_) { other.ptr_ = nullptr; }

    Handle& operator=(Handle&& other) noexcept {
        if (this != &other) {
            if (ptr_) ReleaseFn(ptr_);
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    T* get() const noexcept { return ptr_; }
    T** put() noexcept { return &ptr_; }

    T* release() noexcept {
        T* p = ptr_;
        ptr_ = nullptr;
        return p;
    }

    explicit operator bool() const noexcept { return ptr_ != nullptr; }

private:
    T* ptr_ = nullptr;
};

// Semantic handles
using DocumentPtr = Handle<DocumentHandle, Document_Release>;
using CatalogPtr = Handle<CatalogHandle, Catalog_Release>;
using PageTreePtr = Handle<PageTreeHandle, PageTree_Release>;
using PageObjectPtr = Handle<PageObjectHandle, PageObject_Release>;
using DocumentInfoPtr = Handle<DocumentInfoHandle, DocumentInfo_Release>;
using InteractiveFormPtr = Handle<InteractiveFormHandle, InteractiveForm_Release>;
using FieldCollectionPtr = Handle<FieldCollectionHandle, FieldCollection_Release>;
using FieldPtr = Handle<FieldHandle, Field_Release>;
using SignatureFieldPtr = Handle<SignatureFieldHandle, SignatureField_Release>;
using DigitalSignaturePtr = Handle<DigitalSignatureHandle, DigitalSignature_Release>;
using DocumentSignatureSettingsPtr = Handle<DocumentSignatureSettingsHandle, DocumentSignatureSettings_Release>;
using DocumentEncryptionSettingsPtr = Handle<DocumentEncryptionSettingsHandle, DocumentEncryptionSettings_Release>;

// Syntax handles
using HexadecimalStringObjectPtr = Handle<HexadecimalStringObjectHandle, HexadecimalStringObject_Release>;
using DictionaryObjectPtr = Handle<DictionaryObjectHandle, DictionaryObject_Release>;
using NameObjectPtr = Handle<NameObjectHandle, NameObject_Release>;
using IntegerObjectPtr = Handle<IntegerObjectHandle, IntegerObject_Release>;
using ObjectPtr = Handle<ObjectHandle, Object_Release>;
using StringObjectPtr = Handle<StringObjectHandle, StringObject_Release>;
using LiteralStringObjectPtr = Handle<LiteralStringObjectHandle, LiteralStringObject_Release>;

// Signature verification handles
using ByteRangeCollectionPtr = Handle<ByteRangeCollectionHandle, ByteRangeCollection_Release>;
using ByteRangePtr = Handle<ByteRangeHandle, ByteRange_Release>;
using TrustedCertificateStorePtr = Handle<TrustedCertificateStoreHandle, TrustedCertificateStore_Release>;
using SignatureVerificationSettingsPtr = Handle<SignatureVerificationSettingsHandle, SignatureVerificationSettings_Release>;
using SignatureVerificationResultPtr = Handle<SignatureVerificationResultHandle, SignatureVerificationResult_Release>;

// Utility handles
using FilePtr = Handle<FileHandle, File_Release>;
using BufferPtr = Handle<BufferHandle, Buffer_Release>;
using PKCS12KeyPtr = Handle<PKCS12KeyHandle, PKCS12Key_Release>;
using SigningKeyPtr = Handle<SigningKeyHandle, SigningKey_Release>;

inline std::string last_error_message() {
    size_type len = 0;
    check(Errors_GetLastErrorMessageLength(&len), "Failed to get error message length");
    if (len <= 1) return {};
    std::string text(len - 1, '\0');
    check(Errors_GetLastErrorMessage(text.data(), len), "Failed to get error message");
    return text;
}

inline std::string to_string(StringObjectHandle* handle) {
    BufferPtr buf;
    check(StringObject_GetValue(handle, buf.put()));
    string_type data = nullptr;
    size_type size = 0;
    check(Buffer_GetData(buf.get(), &data, &size));
    return std::string(data, size);
}

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

#endif // VANILLAPDF_CLI_UTILS_ERROR_HPP
