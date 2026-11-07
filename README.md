# vanillapdf-cli

**The command-line toolkit for professional PDF processing, validation and digital signatures.**

<div align="center">

| Distribution | CI & Quality | License |
|:---:|:---:|:---:|
| [![Docs](https://readthedocs.org/projects/vanillapdf-cli/badge/?version=latest)](https://vanillapdf.readthedocs.io/projects/cli/) | [![Compatibility](https://github.com/vanillapdf/vanillapdf-cli/actions/workflows/compat-vanillapdf-main.yml/badge.svg)](https://github.com/vanillapdf/vanillapdf-cli/actions/workflows/compat-vanillapdf-main.yml) [![Sanity check](https://github.com/vanillapdf/vanillapdf-cli/actions/workflows/sanity-check.yml/badge.svg)](https://github.com/vanillapdf/vanillapdf-cli/actions/workflows/sanity-check.yml) | [![License](https://img.shields.io/badge/license-Apache%202.0-blue)](LICENSE) |

**[Documentation](https://vanillapdf.readthedocs.io/projects/cli/)** · **[Command reference](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/)** · **[Releases](https://github.com/vanillapdf/vanillapdf-cli/releases)**

</div>

**vanillapdf-cli** inspects, edits, encrypts, signs and validates PDF documents from the command line on Linux, macOS and Windows. It is the command-line front end of the [vanillapdf](https://github.com/vanillapdf/vanillapdf) library, so it handles PDFs exactly the way applications built on the library do. Release archives are self-contained: download, extract and run.

- 📄 **Inspect & transform** — read and write metadata, merge, split, rotate, delete and insert pages, compress
- 🔒 **Encrypt & decrypt** — AES-256 (or 128/RC4) with permissions and passwords
- 🔏 **Sign & verify** — apply and validate digital signatures from PKCS#12 keys
- 🩺 **Validate structure** — walk the xref/trailer and report malformed files
- ⚙️ **Scriptable** — clean stdout/stderr separation, JSON output, meaningful exit codes

## Contents

- [Install](#install)
- [Quick start](#quick-start)
- [Commands](#commands)
- [Limitations](#limitations)
- [Contributing](#contributing)
- [License](#license)

## Install

**Release archives** for Linux, macOS and Windows (x64 and ARM64) are attached to every [GitHub release](https://github.com/vanillapdf/vanillapdf-cli/releases). They are self-contained: download, extract, and run `vanillapdf-cli`. Linux needs glibc 2.34 or newer (Ubuntu 22.04+, Debian 12+, RHEL/Rocky 9+); macOS needs 14 or newer.

**From source**, with CMake 3.20+ and a C++17 compiler (vcpkg is bootstrapped and vanillapdf fetched automatically):

```sh
git clone https://github.com/vanillapdf/vanillapdf-cli.git
cd vanillapdf-cli
cmake -B build
cmake --build build
cmake --install build
```

**Package managers** — Homebrew, apt and dnf packages are planned for a future release.

See [Installation](https://vanillapdf.readthedocs.io/projects/cli/en/latest/installation.html) for checksum and provenance verification, and [Building from source](https://vanillapdf.readthedocs.io/projects/cli/en/latest/building.html) for presets and build options.

## Quick start

```sh
# Show metadata and page count (add --json for scripts)
vanillapdf-cli info report.pdf

# Combine documents, then pull out the first three pages and the last one
vanillapdf-cli merge a.pdf b.pdf c.pdf -o book.pdf
vanillapdf-cli split book.pdf -o summary.pdf -p 1-3,last

# Protect with a password (AES-256), then sign with a PKCS#12 certificate
vanillapdf-cli encrypt add report.pdf -o secured.pdf -p "my-password"
vanillapdf-cli sign add report.pdf -o signed.pdf -k cert.p12 -p "passphrase"

# Check the file structure; exits non-zero when the file is broken
vanillapdf-cli validate report.pdf
```

Every command prints its options with `--help`, for example `vanillapdf-cli split --help`.

## Commands

Each command links to its reference page, with every option explained and more examples.

| Command | Description |
|---|---|
| [`info`](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/info.html) | Display file metadata and page count |
| [`metadata set`](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/metadata.html) | Write document information fields |
| [`merge`](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/merge.html) | Combine multiple PDFs into one |
| [`split`](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/split.html) | Extract pages into a new PDF |
| [`pages delete` / `insert`](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/pages.html) | Delete pages, or insert pages from another PDF |
| [`rotate`](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/rotate.html) | Rotate pages by 90 / 180 / 270 degrees |
| [`compress`](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/compress.html) | Compress and optimise a PDF |
| [`encrypt add` / `remove`](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/encrypt.html) | Add or remove password encryption (AES-256 by default) |
| [`sign add` / `list` / `verify`](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/sign.html) | Apply, list and verify digital signatures |
| [`validate`](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/validate.html) | Validate PDF file structure |

Input files are positional arguments and output files use `-o` / `--output`. Commands that work on some pages take a [page range](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/#page-ranges) with `-p` / `--pages`, such as `1-3,last`, `even` or `"!first"`. Global flags (`--version`, `-v` / `--verbose`, `--log-level`) are listed on the [documentation home page](https://vanillapdf.readthedocs.io/projects/cli/).

Runnable workflow scripts (batch merge, compress, sign/verify, validate) live in [`examples/`](examples/).

## Limitations

vanillapdf-cli operates on **PDF document structure**. It is not a renderer: it does not rasterize pages, display PDFs, or perform OCR. Pair it with a renderer for viewing or text extraction, and use this tool for the structural operations around it.

Not yet supported:

- Removing signatures (`sign remove`)
- CRL/OCSP revocation checking during signature verification
- PAdES compliance levels and RFC 3161 timestamp validation

## Contributing

We welcome pull requests, feature proposals, and bug reports.

- [Contributing Guidelines](CONTRIBUTING.md) — build/test workflow, command pattern, DCO
- [Code of Conduct](CODE_OF_CONDUCT.md) — community standards
- [Changelog](CHANGELOG.md) — release history; the project follows [Semantic Versioning](https://semver.org/)
- [Security Policy](SECURITY.md) — responsible disclosure (please do **not** open a public issue)

## License

vanillapdf-cli is licensed under the [Apache 2.0 License](LICENSE). Copyright 2026 Vanilla.PDF Labs.

Questions and support: [GitHub Issues](https://github.com/vanillapdf/vanillapdf-cli/issues), info@vanillapdf.com, or [vanillapdf.com](https://vanillapdf.com). You can support the project through [GitHub Sponsors](https://github.com/sponsors/vanillapdf).
