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

**Shared utilities** (`src/utils/`, all in namespace `vanillapdf::cli`):
- `error.hpp` — `check(error_type, context)` (throws `pdf_error` on non-success codes), `last_error_message()`, `report_error(pdf_error)` (prints to stderr; appends vanillapdf's last error under `-v`), and the `g_verbose` flag.
- `handles.hpp` — `Handle<T, ReleaseFn>` RAII wrapper for vanillapdf C API opaque handles (use `.get()`, `.put()` for out-params, `.release()`), plus type aliases: `DocumentPtr`, `CatalogPtr`, `PageTreePtr`, `PageObjectPtr`, `FilePtr`, `BufferPtr`, `DocumentInfoPtr`, `PKCS12KeyPtr`, `SigningKeyPtr`, `StringObjectPtr`, `FileStructureValidationResultPtr`, `FileStructureIssuePtr`, and others.
- `encoding.hpp` — `to_string_utf8(StringObjectHandle*)` decodes a PDF text string (PDFDocEncoding / UTF-16BE / UTF-8) to UTF-8 via `TextStringEncoding_Detect`.
- `json.hpp` — `json_escape(string_view)` escapes a string as a JSON string literal (built with fmt).

Dependencies (`vanillapdf`, `CLI11`, `fmt`) come via `find_package`; `fmt` is used directly by the CLI. GoogleTest backs the unit tests and is only pulled in when `VANILLAPDF_CLI_ENABLE_TESTS=ON` (off by default; on in the dev/CI presets).

## Conventions

- C++17, no extensions (`CMAKE_CXX_EXTENSIONS OFF`)
- Use `#ifndef`/`#define` include guards, not `#pragma once`
- Guard naming: `VANILLAPDF_CLI_<PATH>_<FILE>_HPP` (e.g., `VANILLAPDF_CLI_UTILS_ERROR_HPP`)
- Packaging target: deb/rpm via CPack, dependencies come from system packages (not vendored)
