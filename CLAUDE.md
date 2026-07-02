# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```bash
cmake --preset default      # configure (system packages)
cmake --preset vcpkg         # configure (vcpkg, needs VCPKG_ROOT)
cmake --preset vcpkg-dev     # configure (vcpkg + vanillapdf from source via FetchContent)
cmake --build build          # build
ctest --test-dir build -C Debug --output-on-failure  # test (Windows)
ctest --test-dir build --output-on-failure            # test (Linux/Mac)
cmake --install build        # install
cpack --config build/CPackConfig.cmake  # package (deb/rpm)
```

Dependencies: `vanillapdf` (libvanillapdf) and `CLI11`, resolved via `find_package()`. For development, use the `vcpkg` preset with `VCPKG_ROOT` set. Use the `vcpkg-dev` preset to build against vanillapdf from source (controlled by `VANILLAPDF_GIT_TAG`).

## Packaging

**deb/rpm**: CPack targets, system package dependencies declared in `CMakeLists.txt`.

**Homebrew**: `homebrew/vanillapdf-cli.rb.in` is a template with `@PLACEHOLDER@` variables substituted at release time. The formula depends on `vanillapdf` (the Homebrew formula) and `cli11`; no extra cmake flags are needed since vcpkg is not activated (no toolchain file) and FetchContent is not triggered (`VANILLAPDF_GIT_TAG` defaults to empty).

## Architecture

C++17 CLI tool built on the [vanillapdf](https://github.com/vanillapdf/vanillapdf) C API (`vanillapdf/c_vanillapdf_api.h`) and [CLI11](https://github.com/CLIUtils/CLI11) for argument parsing. Binary output name is `vanillapdf-cli`.

**Command pattern**: Each command lives in `src/commands/<name>.cpp` and exposes a `register_<name>(CLI::App&)` function (declared in `commands.hpp`). Registration creates a CLI11 subcommand with options and a callback. `main.cpp` calls all register functions.

**`src/utils/error.hpp`**: Core utilities shared across commands:
- `vanillapdf::cli::check(error_type, context)` — throws `vanillapdf::cli::pdf_error` on non-success vanillapdf error codes
- `vanillapdf::cli::Handle<T, ReleaseFn>` — RAII wrapper for vanillapdf C API opaque handles (use `.get()`, `.put()` for out-params, `.release()`)
- `vanillapdf::cli::to_string(StringObjectHandle*)` — extracts `std::string` from a vanillapdf string object
- Type aliases for RAII handles: `DocumentPtr`, `CatalogPtr`, `PageTreePtr`, `PageObjectPtr`, `FilePtr`, `BufferPtr`, `DocumentInfoPtr`, `DocumentEncryptionSettingsPtr`, `DocumentSignatureSettingsPtr`, `PKCS12KeyPtr`, `SigningKeyPtr`, `InteractiveFormPtr`, `FieldCollectionPtr`, `FieldPtr`, `SignatureFieldPtr`, `DigitalSignaturePtr`, `DictionaryObjectPtr`, `NameObjectPtr`, `IntegerObjectPtr`, `ObjectPtr`, `StringObjectPtr`, `LiteralStringObjectPtr`

## Conventions

- C++17, no extensions (`CMAKE_CXX_EXTENSIONS OFF`)
- Use `#ifndef`/`#define` include guards, not `#pragma once`
- Guard naming: `VANILLAPDF_CLI_<PATH>_<FILE>_HPP` (e.g., `VANILLAPDF_CLI_UTILS_ERROR_HPP`)
- Packaging target: deb/rpm via CPack, dependencies come from system packages (not vendored)
