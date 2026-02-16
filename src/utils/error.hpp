#ifndef VANILLAPDF_CLI_UTILS_ERROR_HPP
#define VANILLAPDF_CLI_UTILS_ERROR_HPP

#include <vanillapdf/c_vanillapdf_api.h>
#include <stdexcept>
#include <string>
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
using DictionaryObjectPtr = Handle<DictionaryObjectHandle, DictionaryObject_Release>;
using NameObjectPtr = Handle<NameObjectHandle, NameObject_Release>;
using IntegerObjectPtr = Handle<IntegerObjectHandle, IntegerObject_Release>;
using ObjectPtr = Handle<ObjectHandle, Object_Release>;
using StringObjectPtr = Handle<StringObjectHandle, StringObject_Release>;
using LiteralStringObjectPtr = Handle<LiteralStringObjectHandle, LiteralStringObject_Release>;

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

} // namespace vpdf

#endif // VANILLAPDF_CLI_UTILS_ERROR_HPP
