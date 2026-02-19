# Building from Source

## Prerequisites

- CMake 3.20+
- A C++17 compiler (MSVC 2019+, GCC 10+, Clang 12+)
- Git

All other dependencies (vanillapdf, CLI11, and vanillapdf's transitive dependencies) are handled automatically.

## Quick build (zero configuration)

Cloning and building with no extra setup. vcpkg is bootstrapped automatically and vanillapdf is fetched from source:

```sh
git clone https://github.com/vanillapdf/vanillapdf-cli.git
cd vanillapdf-cli
cmake -B build
cmake --build build
cmake --install build
```

## CMake presets

| Preset | Description |
|---|---|
| *(none)* | Auto-bootstraps vcpkg, builds vanillapdf from `main` |
| `default` | All deps from system (`find_package`). For Homebrew / deb / rpm. |
| `vcpkg` | Uses an existing vcpkg installation via `VCPKG_ROOT` |
| `vcpkg-dev` | Auto-bootstraps vcpkg, builds vanillapdf from `main` (explicit) |
| `vcpkg-static` | Windows only — static binary via vcpkg |

```sh
cmake --preset vcpkg-dev   # equivalent to cmake -B build with defaults
cmake --preset default     # system packages only, no vcpkg
cmake --preset vcpkg       # VCPKG_ROOT must be set
```

## CMake options

| Option | Default | Description |
|---|---|---|
| `VANILLAPDF_CLI_INTERNAL_VCPKG` | `ON` | Auto-manage deps via vcpkg |
| `VANILLAPDF_CLI_EXTERNAL_VANILLAPDF` | `OFF` | Use system-installed vanillapdf |
| `VANILLAPDF_CLI_EXTERNAL_CLI11` | `OFF` | Use system-installed CLI11 |
| `VANILLAPDF_GIT_TAG` | `main` | vanillapdf branch / tag for FetchContent |
| `VANILLAPDF_CLI_ENABLE_TESTS` | `ON` | Build integration tests |

## Running tests

```sh
# Windows
ctest --test-dir build -C Debug --output-on-failure

# Linux / macOS
ctest --test-dir build --output-on-failure
```

## Packaging

```sh
cmake --install build              # install to system
cpack --config build/CPackConfig.cmake   # build deb / rpm package
```
