# vanillapdf-cli — PDF manipulation from the command line

<div align="center">

| Distribution | CI & Quality | License |
|:---:|:---:|:---:|
| [![Docs](https://readthedocs.org/projects/vanillapdf-cli/badge/?version=latest)](https://vanillapdf-cli.readthedocs.io/) | [![Compatibility](https://github.com/vanillapdf/vanillapdf-cli/actions/workflows/compat-vanillapdf-main.yml/badge.svg)](https://github.com/vanillapdf/vanillapdf-cli/actions/workflows/compat-vanillapdf-main.yml) [![Sanity check](https://github.com/vanillapdf/vanillapdf-cli/actions/workflows/sanity-check.yml/badge.svg)](https://github.com/vanillapdf/vanillapdf-cli/actions/workflows/sanity-check.yml) | [![License](https://img.shields.io/badge/license-Apache%202.0-blue)](LICENSE) |

</div>

**vanillapdf-cli** is a fast, cross-platform command-line tool for inspecting and transforming PDF documents. It's built on [vanillapdf](https://github.com/vanillapdf/vanillapdf), a modern C++17 PDF library with a clean, ABI-stable C API — so every operation runs through the same battle-tested engine, with no runtime dependencies to install.

- 📄 **Inspect & transform** — read metadata, merge, split, rotate, and compress
- 🔒 **Encrypt & decrypt** — AES-256 (or 128/RC4) with permissions and passwords
- 🔏 **Sign & verify** — apply and validate digital signatures from PKCS#12 keys
- 🩺 **Validate structure** — walk the xref/trailer and report malformed files
- ⚙️ **Scriptable** — clean stdout/stderr separation, JSON output, meaningful exit codes

## Install

### Download a release

Prebuilt archives for Linux and macOS (`.tar.gz`) and Windows (`.zip`) are attached to every [GitHub release](https://github.com/vanillapdf/vanillapdf-cli/releases). Download, extract, and run `vanillapdf-cli`.

Archives are self-contained and available for Linux x64 and ARM64 (glibc 2.34 or newer: Ubuntu 22.04+, Debian 12+, RHEL/Rocky 9+), Windows x64 and ARM64, and macOS 14+ on x64 and ARM64.

### Build from source

```sh
git clone https://github.com/vanillapdf/vanillapdf-cli.git
cd vanillapdf-cli
cmake -B build          # auto-bootstraps vcpkg, fetches vanillapdf
cmake --build build
cmake --install build
```

See [Building from source](https://vanillapdf-cli.readthedocs.io/building) for all options, including static builds and system-package builds.

### Package managers

> **Coming soon.** Homebrew, apt (Debian/Ubuntu), and dnf (Fedora/RHEL) packages are planned for a future release. For now, use a release archive or build from source.

## Quick start

| Task | Command |
|------|---------|
| Inspect a PDF | `vanillapdf-cli info report.pdf` |
| Tag metadata | `vanillapdf-cli metadata set report.pdf -o tagged.pdf --title "Q4 Report"` |
| Merge files | `vanillapdf-cli merge a.pdf b.pdf c.pdf -o book.pdf` |
| Split out pages | `vanillapdf-cli split report.pdf -o pages.pdf -p 2 4 6` |
| Encrypt (AES-256) | `vanillapdf-cli encrypt add report.pdf -o secured.pdf -p "my-password"` |
| Sign with a certificate | `vanillapdf-cli sign add report.pdf -o signed.pdf -k cert.p12` |

Each command has a dedicated section with all options in the [command reference](#commands) below.

## Commands

| Command | Description |
|---|---|
| [`info`](#info) | Display file metadata and page count |
| [`metadata set`](#metadata-set) | Write document information fields |
| [`merge`](#merge) | Combine multiple PDFs into one |
| [`split`](#split) | Extract pages into a new PDF |
| [`pages`](#pages) | Delete, reorder, or insert pages |
| [`rotate`](#rotate) | Rotate pages by 90 / 180 / 270 degrees |
| [`compress`](#compress) | Compress and optimise a PDF |
| [`encrypt add`](#encrypt-add--remove) | Add password encryption (AES-256 default) |
| [`encrypt remove`](#encrypt-add--remove) | Remove encryption |
| [`sign add`](#sign-add--list--verify) | Apply a digital signature |
| [`sign list`](#sign-add--list--verify) | List signatures |
| [`sign verify`](#sign-add--list--verify) | Verify signatures |
| [`validate`](#validate) | Validate PDF file structure |

### Global flags

```
--version          Show version
--copyright        Show license information
--about            Show project information and links (homepage, repo, sponsor)
-v, --verbose      Show detailed error output
--log-level LEVEL  Library log level: off critical error warning info debug trace
```

Errors go to `stderr`; the process exits non-zero on failure. Input files are positional arguments; output files use `-o` / `--output`. Page numbers are 1-based.

---

### `info`

```
vanillapdf-cli info <input> [--json]
```

Prints metadata (title, author, subject, keywords, creator, producer) and page count. Pass `--json` for machine-readable output.

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

```sh
vanillapdf-cli info annual-report.pdf --json
```

```json
{
  "file": "annual-report.pdf",
  "pdf_version": "1.7",
  "pages": 42,
  "title": "Annual Report 2025",
  "author": "Jane Smith",
  "subject": "Financial results",
  "keywords": null,
  "creator": "LibreOffice",
  "producer": "vanillapdf 2.2.0"
}
```

---

### `metadata set`

```
vanillapdf-cli metadata set <input> -o <output>
    [--title <t>] [--author <a>] [--subject <s>] [--keywords <k>] [--creator <c>] [-p <password>]
```

Writes the document information dictionary. Each field is three-way: **omit** it to leave the current value untouched, pass an **empty string** to remove it, or pass a **value** to set it. The information dictionary is created if the document has none.

`Producer` and `ModDate` are always stamped by the tool — a full save republishes the document — and are not user-overridable. Encrypted documents can be edited by supplying the password with `-p`.

```sh
# Set title and author, leave other fields untouched
vanillapdf-cli metadata set report.pdf -o tagged.pdf \
    --title "Annual Report 2025" --author "Jane Smith"

# Remove the subject
vanillapdf-cli metadata set report.pdf -o cleaned.pdf --subject ""
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

### `pages`

```
vanillapdf-cli pages delete  <input> -o <output> -p <pages>...
vanillapdf-cli pages reorder <input> -o <output> --order <permutation>
vanillapdf-cli pages insert  <input> -o <output> --from <other.pdf> [--at <n>]
```

Page-level editing that preserves the rest of the document. Page numbers are 1-based.

```sh
# Delete pages 2 and 5
vanillapdf-cli pages delete report.pdf -o trimmed.pdf -p 2 5

# Reorder a 3-page document (must be a full permutation)
vanillapdf-cli pages reorder report.pdf -o reordered.pdf --order 3,1,2

# Insert cover.pdf before page 1 (omit --at to append at the end)
vanillapdf-cli pages insert report.pdf -o with-cover.pdf --from cover.pdf --at 1
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
    [--algorithm aes|rc4] [--bits <n>] [--allow <perms>]
vanillapdf-cli encrypt remove <input> -o <output> -p <password>
```

Encrypts with **AES-256 by default**. The user password is required to open the file; the owner password (optional) is required to edit it.

| Option | Description |
|---|---|
| `--algorithm` | `aes` (default) or `rc4` (legacy, insecure) |
| `--bits` | Key length — AES: `128`/`256` (default `256`), RC4: `40`/`128` (default `128`) |
| `--allow` | Comma-separated permissions to grant; **omit to allow everything**. Values: `print`, `print-high`, `modify`, `copy`, `annotate`, `fill-forms`, `extract`, `assemble` (plus `all`/`none`) |

```sh
# AES-256 with a user and owner password
vanillapdf-cli encrypt add confidential.pdf -o protected.pdf -p "open123" --owner-password "edit456"

# Restrict permissions: allow printing and copying only
vanillapdf-cli encrypt add report.pdf -o restricted.pdf -p "open123" --allow print,copy

# Remove encryption
vanillapdf-cli encrypt remove protected.pdf -o unlocked.pdf -p "open123"
```

---

### `sign add` / `list` / `verify`

```
vanillapdf-cli sign add    <input> -o <output> -k <key.p12> [-p <password>]
    [--name <name>] [--reason <reason>] [--location <location>] [--no-signing-time]
vanillapdf-cli sign list   <input> [--json]
vanillapdf-cli sign verify <input> [-c <cert.pem>] [--json]
```

Signs using a PKCS#12 certificate (`.p12` / `.pfx`). `--name`, `--reason`, and `--location` are recorded in the signature (and shown by `sign list`); the signing time is set to the current time unless `--no-signing-time` is given. The `list` subcommand shows Name, Reason, Location, Contact Info, and Signing Time for each signature (with `--json` output); `verify` checks each signature against the system trust store (or a certificate passed with `-c`) and also supports `--json`.

```sh
# Sign a document with signer metadata
vanillapdf-cli sign add contract.pdf -o contract-signed.pdf -k my-cert.p12 -p "passphrase" \
    --name "Jane Smith" --reason "Approved" --location "London, UK"

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

### `validate`

```
vanillapdf-cli validate <input> [--strict] [--json]
```

Checks the structural integrity of a PDF — header, cross-reference tables, and trailer — reporting each issue as `Info`, `Warning`, or `Error`. The file is not modified. Exits non-zero when errors are found, or when warnings are found and `--strict` is set. Use `--json` for machine-readable output.

```sh
vanillapdf-cli validate report.pdf
```

```
File: report.pdf
Valid: yes
Errors: 0  Warnings: 1
[Warning] TrailerSizeMismatch (5): Trailer /Size does not match the number of xref entries
```

## Platforms

| Platform | Compilers | Architectures |
|----------|-----------|---------------|
| Windows | Visual Studio 2022+ | x64, ARM64 |
| Linux | GCC 8.1+, Clang 10+ | x64, ARM64 |
| macOS | AppleClang 15+ (Xcode 15) | x64, ARM64 |

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

## Non-Goals

vanillapdf-cli operates on **PDF document structure**. It is not a renderer: it does not rasterize pages, display PDFs, or perform OCR. For viewing or extracting rendered text, pair it with a dedicated renderer and use this tool for the structural operations around it.

Current known limitations:

- `sign remove` is not yet implemented
- No CRL/OCSP revocation checking during signature verification
- No PAdES compliance levels or RFC 3161 timestamp validation

## Versioning

vanillapdf-cli follows [Semantic Versioning](https://semver.org/). See [CHANGELOG.md](CHANGELOG.md) for the release history, and check the running version with `vanillapdf-cli --version` (which also reports the linked vanillapdf library version).

## Documentation

Full documentation is hosted on **[Read the Docs](https://vanillapdf-cli.readthedocs.io/)**.

| Guide | Description |
|-------|-------------|
| [Home](https://vanillapdf-cli.readthedocs.io/) | Overview and quick start |
| [Installation](https://vanillapdf-cli.readthedocs.io/installation) | Release archives and build from source |
| [Commands](https://vanillapdf-cli.readthedocs.io/commands/) | Full reference for every command |
| [Building from Source](https://vanillapdf-cli.readthedocs.io/building) | CMake presets and build options |

Runnable workflow scripts (batch merge, compress, sign/verify, validate) live in the [`examples/`](examples/) directory.

## Contributing

We welcome pull requests, feature proposals, and bug reports.

- [Contributing Guidelines](CONTRIBUTING.md) — build/test workflow, command pattern, DCO
- [Code of Conduct](CODE_OF_CONDUCT.md) — community standards
- [Changelog](CHANGELOG.md) — release history
- [Security Policy](SECURITY.md) — responsible disclosure (please do **not** open a public issue)

All changes require a branch and pull request; `main` is protected.

## License

vanillapdf-cli is licensed under the [Apache 2.0 License](LICENSE).

Copyright 2026 Vanilla.PDF Labs

## Contact & Support

| Channel | Link |
|---------|------|
| Email | info@vanillapdf.com |
| Website | [vanillapdf.com](https://vanillapdf.com) |
| Issues | [GitHub Issues](https://github.com/vanillapdf/vanillapdf-cli/issues) |
| Sponsor | [GitHub Sponsors](https://github.com/sponsors/vanillapdf) |
