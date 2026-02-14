# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```bash
cmake --preset default      # configure (system packages)
cmake --preset vcpkg         # configure (vcpkg, needs VCPKG_ROOT)
cmake --build build          # build
cmake --install build        # install
cpack --config build/CPackConfig.cmake  # package (deb/rpm)
```

Dependencies: `vanillapdf` (libvanillapdf) and `CLI11`, resolved via `find_package()`. For development, use the `vcpkg` preset with `VCPKG_ROOT` set.

## Architecture

C++17 CLI tool built on the [vanillapdf](https://github.com/vanillapdf/vanillapdf) C API (`vanillapdf/c_vanillapdf_api.h`) and [CLI11](https://github.com/CLIUtils/CLI11) for argument parsing. Binary output name is `vanillapdf-cli`.

**Command pattern**: Each command lives in `src/commands/<name>.cpp` and exposes a `register_<name>(CLI::App&)` function (declared in `commands.hpp`). Registration creates a CLI11 subcommand with options and a callback. `main.cpp` calls all register functions.

**`src/utils/error.hpp`**: Core utilities shared across commands:
- `vpdf::check(error_type, context)` — throws `vpdf::pdf_error` on non-success vanillapdf error codes
- `vpdf::Handle<T, ReleaseFn>` — RAII wrapper for vanillapdf C API opaque handles (use `.get()`, `.put()` for out-params, `.release()`)
- Type aliases: `DocumentPtr`, `CatalogPtr`, `PageTreePtr`, `PageObjectPtr`

## Conventions

- C++17, no extensions (`CMAKE_CXX_EXTENSIONS OFF`)
- Use `#ifndef`/`#define` include guards, not `#pragma once`
- Guard naming: `VANILLAPDF_CLI_<PATH>_<FILE>_HPP` (e.g., `VANILLAPDF_CLI_UTILS_ERROR_HPP`)
- Packaging target: deb/rpm via CPack, dependencies come from system packages (not vendored)
