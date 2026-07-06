# Changelog

All notable changes to **vanillapdf-cli** are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- `info` — display file metadata (title, author, subject, keywords, creator,
  producer) and page count, with UTF-8 text-string decoding.
- `merge` — combine multiple PDFs into a single document.
- `split` — extract selected pages (1-based) into a new PDF.
- `rotate` — rotate pages by 90 / 180 / 270 degrees, cumulative with existing
  rotation.
- `compress` — compress and optimise a PDF using vanillapdf's built-in
  compression.
- `encrypt add` / `encrypt remove` — add or remove password encryption with
  user and optional owner passwords. Supports `--algorithm` (AES or legacy RC4),
  `--bits` (AES 128/256, default 256; RC4 40/128), and `--allow` to grant
  specific document permissions (print, copy, modify, etc.).
- `sign add` / `sign list` / `sign verify` — apply, list, and verify digital
  signatures using PKCS#12 certificates. `sign verify` supports `--json` output.
- `validate` — validate PDF file structure (header, cross-reference tables,
  trailer), reporting Info / Warning / Error issues, with `--strict` and
  `--json` options.
- Global flags: `--version`, `--copyright`, `-v` / `--verbose`, and
  `--log-level`.
- Packaging: Homebrew formula, Debian/RPM packages via CPack, and a
  tag-driven release workflow.

### Not yet supported

- `sign remove` — placeholder subcommand; removing signatures is not yet
  implemented.

[Unreleased]: https://github.com/vanillapdf/vanillapdf-cli/commits/main
