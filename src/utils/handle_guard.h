#ifndef _VANILLAPDF_CLI_HANDLE_GUARD_H
#define _VANILLAPDF_CLI_HANDLE_GUARD_H

#include <vanillapdf/c_vanillapdf_api.h>

namespace vanillapdf {
namespace cli {

/**
 * RAII wrapper for C API handle lifetime management.
 *
 * Automatically calls the release function when the guard goes out of scope,
 * so an exception thrown by THROW_IF_NOT_SUCCESS frees every handle acquired so far.
 *
 * Usage:
 *   DocumentGuard document;
 *   THROW_IF_NOT_SUCCESS(Document_Open(path, document.out()), "Failed to open document");
 *   THROW_IF_NOT_SUCCESS(Document_Save(document, destination), "Failed to save document");
 *   // document is automatically released at scope exit
 */
template<typename Handle, error_type (CALLING_CONVENTION *ReleaseFn)(Handle*)>
class HandleGuard {
public:
    HandleGuard() = default;

    explicit HandleGuard(Handle* handle) : m_handle(handle) {
    }

    ~HandleGuard() {
        reset();
    }

    // Move-only semantics to prevent double-release
    HandleGuard(HandleGuard&& other) noexcept : m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }

    HandleGuard& operator=(HandleGuard&& other) noexcept {
        if (this != &other) {
            reset();
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }

        return *this;
    }

    HandleGuard(const HandleGuard&) = delete;
    HandleGuard& operator=(const HandleGuard&) = delete;

    /**
     * Returns a pointer to the internal handle pointer.
     * Used with C API Create/Get functions: Create(guard.out()).
     * A handle already owned by the guard is released first, so a guard
     * declared outside a loop can be refilled on every iteration.
     */
    Handle** out() {
        reset();
        return &m_handle;
    }

    /**
     * Returns the raw handle pointer.
     */
    Handle* get() const {
        return m_handle;
    }

    /**
     * Implicit conversion to raw handle pointer.
     * Allows passing directly to C API functions.
     */
    operator Handle*() const {
        return m_handle;
    }

    /**
     * Explicit bool conversion for null checks.
     */
    explicit operator bool() const {
        return m_handle != nullptr;
    }

    /**
     * Releases the owned handle now. The release result is deliberately
     * ignored, as there is nothing a caller could do about it.
     */
    void reset() {
        if (m_handle != nullptr) {
            ReleaseFn(m_handle);
            m_handle = nullptr;
        }
    }

private:
    Handle* m_handle = nullptr;
};

// Files and documents
using FileGuard = HandleGuard<FileHandle, File_Release>;
using DocumentGuard = HandleGuard<DocumentHandle, Document_Release>;
using CatalogGuard = HandleGuard<CatalogHandle, Catalog_Release>;
using PageTreeGuard = HandleGuard<PageTreeHandle, PageTree_Release>;
using PageObjectGuard = HandleGuard<PageObjectHandle, PageObject_Release>;
using DocumentInfoGuard = HandleGuard<DocumentInfoHandle, DocumentInfo_Release>;

// Cross-reference tables
using XrefChainGuard = HandleGuard<XrefChainHandle, XrefChain_Release>;
using XrefChainIteratorGuard = HandleGuard<XrefChainIteratorHandle, XrefChainIterator_Release>;
using XrefGuard = HandleGuard<XrefHandle, Xref_Release>;
using XrefUsedEntryGuard = HandleGuard<XrefUsedEntryHandle, XrefUsedEntry_Release>;

// Objects
using ObjectGuard = HandleGuard<ObjectHandle, Object_Release>;
using NameObjectGuard = HandleGuard<NameObjectHandle, NameObject_Release>;
using IntegerObjectGuard = HandleGuard<IntegerObjectHandle, IntegerObject_Release>;
using DictionaryObjectGuard = HandleGuard<DictionaryObjectHandle, DictionaryObject_Release>;
using StringObjectGuard = HandleGuard<StringObjectHandle, StringObject_Release>;
using LiteralStringObjectGuard = HandleGuard<LiteralStringObjectHandle, LiteralStringObject_Release>;
using HexadecimalStringObjectGuard = HandleGuard<HexadecimalStringObjectHandle, HexadecimalStringObject_Release>;
using IndirectReferenceObjectGuard = HandleGuard<IndirectReferenceObjectHandle, IndirectReferenceObject_Release>;

// Buffers
using BufferGuard = HandleGuard<BufferHandle, Buffer_Release>;

// Interactive forms and signatures
using InteractiveFormGuard = HandleGuard<InteractiveFormHandle, InteractiveForm_Release>;
using FieldCollectionGuard = HandleGuard<FieldCollectionHandle, FieldCollection_Release>;
using FieldGuard = HandleGuard<FieldHandle, Field_Release>;
using SignatureFieldGuard = HandleGuard<SignatureFieldHandle, SignatureField_Release>;
using DigitalSignatureGuard = HandleGuard<DigitalSignatureHandle, DigitalSignature_Release>;
using ByteRangeCollectionGuard = HandleGuard<ByteRangeCollectionHandle, ByteRangeCollection_Release>;
using ByteRangeGuard = HandleGuard<ByteRangeHandle, ByteRange_Release>;
using TrustedCertificateStoreGuard = HandleGuard<TrustedCertificateStoreHandle, TrustedCertificateStore_Release>;
using SignatureVerificationSettingsGuard = HandleGuard<SignatureVerificationSettingsHandle, SignatureVerificationSettings_Release>;
using SignatureVerificationResultGuard = HandleGuard<SignatureVerificationResultHandle, SignatureVerificationResult_Release>;
using DocumentSignatureSettingsGuard = HandleGuard<DocumentSignatureSettingsHandle, DocumentSignatureSettings_Release>;
using PKCS12KeyGuard = HandleGuard<PKCS12KeyHandle, PKCS12Key_Release>;
using SigningKeyGuard = HandleGuard<SigningKeyHandle, SigningKey_Release>;
using DocumentEncryptionSettingsGuard = HandleGuard<DocumentEncryptionSettingsHandle, DocumentEncryptionSettings_Release>;

// Validation and utilities
using FileStructureValidationResultGuard = HandleGuard<FileStructureValidationResultHandle, FileStructureValidationResult_Release>;
using FileStructureIssueGuard = HandleGuard<FileStructureIssueHandle, FileStructureIssue_Release>;
using DateGuard = HandleGuard<DateHandle, Date_Release>;

} // cli
} // vanillapdf

#endif /* _VANILLAPDF_CLI_HANDLE_GUARD_H */
