# vanillapdf-cli

**Command-line tool for PDF manipulation — merge, split, rotate, encrypt, sign, and more.**

Built on [vanillapdf](https://vanillapdf.com), a cross-platform PDF library with a clean C API.

[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)
[![Compatibility CI](https://github.com/vanillapdf/vanillapdf-cli/actions/workflows/compat-vanillapdf-main.yml/badge.svg)](https://github.com/vanillapdf/vanillapdf-cli/actions/workflows/compat-vanillapdf-main.yml)

---

## Installation

### Homebrew (macOS / Linux)

```sh
brew install vanillapdf/tap/vanillapdf-cli
```

### Debian / Ubuntu

```sh
sudo apt install vanillapdf-cli
```

### RPM (Fedora / RHEL)

```sh
sudo dnf install vanillapdf-cli
```

### Build from source

```sh
git clone https://github.com/vanillapdf/vanillapdf-cli.git
cd vanillapdf-cli
cmake -B build          # auto-bootstraps vcpkg, fetches vanillapdf
cmake --build build
cmake --install build
```

> See [Building from source](https://vanillapdf-cli.readthedocs.io/building) for all options including static builds and system-package builds.

---

## Quick start

```sh
# Inspect a PDF
vanillapdf-cli info report.pdf

# Merge several PDFs into one
vanillapdf-cli merge chapter1.pdf chapter2.pdf chapter3.pdf -o book.pdf

# Extract pages 1, 3, and 5
vanillapdf-cli split document.pdf -o extracted.pdf -p 1 3 5

# Rotate all pages 90 degrees clockwise
vanillapdf-cli rotate document.pdf -o rotated.pdf -a 90

# Compress a PDF
vanillapdf-cli compress large.pdf -o small.pdf

# Encrypt with a password
vanillapdf-cli encrypt add document.pdf -o secured.pdf -p "my-password"

# Sign with a PKCS#12 certificate
vanillapdf-cli sign add document.pdf -o signed.pdf -k cert.p12 -p "key-password"

# List signatures in a PDF
vanillapdf-cli sign list signed.pdf

# Verify signatures (uses system trust store)
vanillapdf-cli sign verify signed.pdf
```

---

## Commands

| Command | Description |
|---|---|
| [`info`](#info) | Display file metadata and page count |
| [`merge`](#merge) | Combine multiple PDFs into one |
| [`split`](#split) | Extract pages into a new PDF |
| [`rotate`](#rotate) | Rotate pages by 90 / 180 / 270 degrees |
| [`compress`](#compress) | Compress and optimise a PDF |
| [`encrypt add`](#encrypt-add--remove) | Add password encryption |
| [`encrypt remove`](#encrypt-add--remove) | Remove encryption |
| [`sign add`](#sign-add--list--remove--verify) | Apply a digital signature |
| [`sign list`](#sign-add--list--remove--verify) | List signatures |
| [`sign remove`](#sign-add--list--remove--verify) | Remove signatures *(not yet supported)* |
| [`sign verify`](#sign-add--list--remove--verify) | Verify signatures |

### Global flags

```
--version          Show version
--copyright        Show license information
-v, --verbose      Show detailed error output
--log-level LEVEL  Library log level: off critical error warning info debug trace
```

---

### `info`

```
vanillapdf-cli info <input>
```

Prints metadata (title, author, subject, keywords, creator, producer) and page count.

```sh
vanillapdf-cli info annual-report.pdf
```

```
File:      annual-report.pdf
Pages:     42
Title:     Annual Report 2025
Author:    Jane Smith
Subject:   Financial results
Keywords:  N/A
Creator:   LibreOffice
Producer:  vanillapdf 2.2.0
```

---

### `merge`

```
vanillapdf-cli merge <input>... -o <output>
```

Appends each input document in order into a single output file.

```sh
vanillapdf-cli merge part1.pdf part2.pdf part3.pdf -o combined.pdf
```

---

### `split`

```
vanillapdf-cli split <input> -o <output> [-p <pages>...]
```

Extracts the specified pages into a new PDF. Omit `-p` to copy all pages. Pages are 1-based; out-of-range values are skipped with a warning.

```sh
# Extract pages 2, 4, and 6
vanillapdf-cli split report.pdf -o even-pages.pdf -p 2 4 6

# Copy all pages (effectively a duplicate)
vanillapdf-cli split report.pdf -o copy.pdf
```

---

### `rotate`

```
vanillapdf-cli rotate <input> -o <output> -a <angle> [-p <pages>...]
```

Rotates pages by `90`, `180`, or `270` degrees. The rotation is cumulative with any existing page rotation. Omit `-p` to rotate all pages.

```sh
# Rotate all pages 90 degrees clockwise
vanillapdf-cli rotate scanned.pdf -o fixed.pdf -a 90

# Flip only page 3 upside-down
vanillapdf-cli rotate document.pdf -o flipped.pdf -a 180 -p 3
```

---

### `compress`

```
vanillapdf-cli compress <input> -o <output>
```

Compresses and optimises the PDF using the vanillapdf library's built-in compression.

```sh
vanillapdf-cli compress bloated.pdf -o lean.pdf
```

---

### `encrypt add` / `remove`

```
vanillapdf-cli encrypt add <input> -o <output> -p <user-password> [--owner-password <password>]
vanillapdf-cli encrypt remove <input> -o <output> -p <password>
```

Encrypts using AES-128. The user password is required to open the file; the owner password (optional) is required to edit it.

```sh
# Add encryption
vanillapdf-cli encrypt add confidential.pdf -o protected.pdf -p "open123" --owner-password "edit456"

# Remove encryption
vanillapdf-cli encrypt remove protected.pdf -o unlocked.pdf -p "open123"
```

---

### `sign add` / `list` / `remove` / `verify`

```
vanillapdf-cli sign add    <input> -o <output> -k <key.p12> [-p <password>]
vanillapdf-cli sign list   <input>
vanillapdf-cli sign verify <input> [-c <cert.pem>] [--json]
vanillapdf-cli sign remove <input> -o <output>   # not yet supported
```

Signs using a PKCS#12 certificate (`.p12` / `.pfx`). The `list` subcommand shows Name, Reason, Location, and Contact Info for each signature.

```sh
# Sign a document
vanillapdf-cli sign add contract.pdf -o contract-signed.pdf -k my-cert.p12 -p "passphrase"

# Inspect signatures
vanillapdf-cli sign list contract-signed.pdf

# Verify signatures
vanillapdf-cli sign verify contract-signed.pdf
```

```
File: contract-signed.pdf
Signature #1:
    Name: Jane Smith
    Reason: Approved
    Location: London, UK
Signatures: 1
```

---

## Building from source

### Zero-configuration (auto-bootstraps vcpkg, fetches vanillapdf)

```sh
cmake -B build
cmake --build build
```

### With an existing vcpkg installation

```sh
cmake --preset vcpkg   # requires VCPKG_ROOT
cmake --build build
```

### System packages (deb / rpm / brew)

```sh
cmake --preset default  # all deps from find_package
cmake --build build
```

### Windows — static binary

```sh
cmake --preset vcpkg-static
cmake --build build
```

### CMake options

| Option | Default | Description |
|---|---|---|
| `VANILLAPDF_CLI_INTERNAL_VCPKG` | `ON` | Auto-manage deps via vcpkg |
| `VANILLAPDF_CLI_EXTERNAL_VANILLAPDF` | `OFF` | Use system-installed vanillapdf |
| `VANILLAPDF_CLI_EXTERNAL_CLI11` | `OFF` | Use system-installed CLI11 |
| `VANILLAPDF_GIT_TAG` | `main` | vanillapdf branch/tag for FetchContent |
| `VANILLAPDF_CLI_ENABLE_TESTS` | `ON` | Build integration tests |

---

## Documentation

Full documentation is available at **[vanillapdf-cli.readthedocs.io](https://vanillapdf-cli.readthedocs.io)**.

---

## License

Apache License 2.0 — see [LICENSE](LICENSE) for details.

Copyright 2026 Vanilla.PDF Labs
