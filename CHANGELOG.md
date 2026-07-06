# Changelog

All notable changes to **vanillapdf-cli** are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0] - 2026-07-07

### Added

- `info` — display file metadata (title, author, subject, keywords, creator,
  producer) and page count, with UTF-8 text-string decoding and optional
  `--json` output.
- `metadata set` — write document information fields (title, author, subject,
  keywords, creator); an empty value removes a field, and the information
  dictionary is created when absent. `Producer` and `ModDate` are always
  stamped, and encrypted documents can be edited with `-p` / `--password`.
- `merge` — combine multiple PDFs into a single document.
- `split` — extract selected pages (1-based) into a new PDF.
- `pages` — page-level editing: `delete`, `reorder`, and `insert` pages,
  preserving the rest of the document.
- `rotate` — rotate pages by 90 / 180 / 270 degrees, cumulative with existing
  rotation.
- `compress` — compress and optimise a PDF using vanillapdf's built-in
  compression.
- `encrypt add` / `encrypt remove` — add or remove password encryption with
  user and optional owner passwords. Supports `--algorithm` (AES or legacy RC4),
  `--bits` (AES 128/256, default 256; RC4 40/128), and `--allow` to grant
  specific document permissions (print, copy, modify, etc.).
- `sign add` / `sign list` / `sign verify` — apply, list, and verify digital
  signatures using PKCS#12 certificates. `sign add` records `--name`, `--reason`,
  and `--location`, and stamps the signing time (unless `--no-signing-time`).
  `sign list` shows those fields and the signing time and supports `--json`;
  `sign verify` also supports `--json`. Listing tolerates a malformed signature
  (reported as an error entry) instead of aborting.
- `validate` — validate PDF file structure (header, cross-reference tables,
  trailer), reporting Info / Warning / Error issues, with `--strict` and
  `--json` options.
- Global flags: `--version`, `--copyright`, `-v` / `--verbose`, and
  `--log-level`.
- Distribution: prebuilt archives from a tag-driven release workflow — Linux
  x64 and Windows x64, plus macOS x64 and arm64 — with build-provenance and
  SBOM attestation. A Homebrew formula template and Debian/RPM CPack
  configuration are included as scaffolding for future package-manager channels.

### Not yet supported

- Removing signatures (`sign remove`) — the vanillapdf library exposes no
  signature-removal API yet, so the command is not offered.

[Unreleased]: https://github.com/vanillapdf/vanillapdf-cli/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/vanillapdf/vanillapdf-cli/releases/tag/v0.1.0
