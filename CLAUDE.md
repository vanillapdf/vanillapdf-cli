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

**Command pattern** (same as `vanillapdf.tools` in the library repo): each command lives in `src/commands/<name>.cpp` and exposes `register_<name>(CLI::App& app, int& exit_code)`, declared in `commands/commands.h`. Registration creates a CLI11 subcommand bound to a `<Name>Options` struct; the callback runs `process_<name>(options)` and stores its result in `exit_code`, which `main` returns. Everything except the register function sits in an anonymous namespace. Commands throw on failure; `main` catches `PdfException` / `PageRangeException` once and reports them.

**Shared utilities** (`src/utils/`, all in namespace `vanillapdf::cli`):
- `error.h` — `THROW_IF_NOT_SUCCESS(call, context)` (throws `PdfException`, recording the failed call, file and line), `PdfException`, `last_error_message()`, `report_error()` (prints to stderr; under `-v` adds the library's last error message and the failed call), and the `g_verbose` flag.
- `handle_guard.h` — `HandleGuard<Handle, ReleaseFn>` RAII wrapper for C API handles, copied from `vanillapdf.tools`: pass `guard.out()` to Create/Get functions and the guard itself where a handle is expected (implicit conversion). Aliases: `DocumentGuard`, `CatalogGuard`, `PageTreeGuard`, `FileGuard`, `BufferGuard`, and others.
- `encoding.h` — `to_string_utf8(StringObjectHandle*)` decodes a PDF text string (PDFDocEncoding / UTF-16BE / UTF-8) to UTF-8 via `TextStringEncoding_Detect`.
- `json.h` — `json_escape(string_view)` escapes a string as a JSON string literal (built with fmt).
- `page_range.h` — `resolve_page_range(range, page_count)` parses `-p` / `--pages` into a set of 1-based pages; throws `PageRangeException`.

Dependencies (`vanillapdf`, `CLI11`, `fmt`) come via `find_package`; `fmt` is used directly by the CLI. GoogleTest backs the unit tests and is only pulled in when `VANILLAPDF_CLI_ENABLE_TESTS=ON` (off by default; on in the dev/CI presets).

## Conventions

The code follows the vanillapdf coding style; see `.claude/rules/coding-style.md`. In short:

- C++17, no extensions (`CMAKE_CXX_EXTENSIONS OFF`)
- `.h` / `.cpp` files; declarations in the header, definitions in the source file (templates excepted)
- `#ifndef` include guards named `_VANILLAPDF_CLI_<FILE>_H`, closed with `#endif /* _VANILLAPDF_CLI_<FILE>_H */`, not `#pragma once`
- Packaging target: deb/rpm via CPack, dependencies come from system packages (not vendored)

## Branching

- Create branches with a `fix/` or `feature/` prefix (e.g., `fix/split-offset`, `feature/metadata-write`).
- `release/*` branches (e.g., `release/1.0`, `release/2.0`) are **restricted for maintenance**. Do not create or push to them; they are reserved for release maintainers.
