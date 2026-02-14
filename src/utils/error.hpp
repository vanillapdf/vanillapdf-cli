#ifndef VANILLAPDF_CLI_UTILS_ERROR_HPP
#define VANILLAPDF_CLI_UTILS_ERROR_HPP

#include <vanillapdf/c_vanillapdf_api.h>
#include <stdexcept>
#include <string>
#include <utility>

namespace vpdf {

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

template <typename T, error_type (*ReleaseFn)(T*)>
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

using DocumentPtr = Handle<DocumentHandle, Document_Release>;
using CatalogPtr = Handle<CatalogHandle, Catalog_Release>;
using PageTreePtr = Handle<PageTreeHandle, PageTree_Release>;
using PageObjectPtr = Handle<PageObjectHandle, PageObject_Release>;

} // namespace vpdf

#endif // VANILLAPDF_CLI_UTILS_ERROR_HPP
