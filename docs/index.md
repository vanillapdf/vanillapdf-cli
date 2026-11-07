# vanillapdf-cli

**The command-line toolkit for professional PDF processing, validation and digital signatures.**

vanillapdf-cli inspects, edits, encrypts, signs and validates PDF documents from the command line on Linux, macOS and Windows. It is the command-line front end of the [vanillapdf](https://github.com/vanillapdf/vanillapdf) library, so it handles PDFs exactly the way applications built on the library do. Release archives are self-contained: download, extract and run.

---

## Commands at a glance

| Command | Description |
|---|---|
| [`info`](commands/info.md) | Display file metadata and page count |
| [`metadata set`](commands/metadata.md) | Write document information fields |
| [`merge`](commands/merge.md) | Combine multiple PDFs into one |
| [`split`](commands/split.md) | Extract pages into a new PDF |
| [`pages delete` / `insert`](commands/pages.md) | Delete pages, or insert pages from another PDF |
| [`rotate`](commands/rotate.md) | Rotate pages by 90 / 180 / 270 degrees |
| [`compress`](commands/compress.md) | Compress and optimise a PDF |
| [`encrypt add` / `remove`](commands/encrypt.md) | Add or remove password encryption |
| [`sign add` / `list` / `verify`](commands/sign.md) | Apply, list and verify digital signatures |
| [`validate`](commands/validate.md) | Validate PDF file structure |

## Quick start

```sh
vanillapdf-cli info report.pdf
vanillapdf-cli merge a.pdf b.pdf c.pdf -o combined.pdf
vanillapdf-cli split document.pdf -o extract.pdf -p 1-3,last
vanillapdf-cli rotate scanned.pdf -o fixed.pdf -a 90
vanillapdf-cli encrypt add doc.pdf -o secured.pdf -p "secret"
vanillapdf-cli sign add doc.pdf -o signed.pdf -k cert.p12
```

## Global flags

These flags are available on every invocation:

| Flag | Description |
|---|---|
| `--version` | Print version and exit |
| `--copyright` | Print license information and exit |
| `--about` | Print project information and links and exit |
| `-v`, `--verbose` | Show detailed error output |
| `--log-level LEVEL` | Set library log level (`off` `critical` `error` `warning` `info` `debug` `trace`) |

```{toctree}
:hidden:
:caption: Getting Started

installation
```

```{toctree}
:hidden:
:caption: Commands

commands/index
commands/info
commands/metadata
commands/merge
commands/split
commands/pages
commands/rotate
commands/compress
commands/encrypt
commands/sign
commands/validate
```

```{toctree}
:hidden:
:caption: Developer Guide

building
```
