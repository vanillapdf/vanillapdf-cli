#ifndef VANILLAPDF_CLI_UTILS_HANDLES_HPP
#define VANILLAPDF_CLI_UTILS_HANDLES_HPP

#include <vanillapdf/c_vanillapdf_api.h>

#include "utils/error.hpp"

namespace vanillapdf::cli {

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

} // namespace vanillapdf::cli

#endif // VANILLAPDF_CLI_UTILS_HANDLES_HPP
