# vanillapdf-cli

**Command-line tool for PDF manipulation — merge, split, rotate, encrypt, sign, and more.**

Built on [vanillapdf](https://vanillapdf.com), a cross-platform PDF library with a clean C API.

---

## Commands at a glance

| Command | Description |
|---|---|
| [`info`](commands/info.md) | Display file metadata and page count |
| [`merge`](commands/merge.md) | Combine multiple PDFs into one |
| [`split`](commands/split.md) | Extract pages into a new PDF |
| [`rotate`](commands/rotate.md) | Rotate pages by 90 / 180 / 270 degrees |
| [`compress`](commands/compress.md) | Compress and optimise a PDF |
| [`encrypt add`](commands/encrypt.md) | Add password encryption |
| [`encrypt remove`](commands/encrypt.md) | Remove encryption |
| [`sign add`](commands/sign.md) | Apply a digital signature |
| [`sign list`](commands/sign.md) | List signatures in a PDF |

## Quick start

```sh
vanillapdf-cli info report.pdf
vanillapdf-cli merge a.pdf b.pdf c.pdf -o combined.pdf
vanillapdf-cli split document.pdf -o extract.pdf -p 1 3 5
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
| `-v`, `--verbose` | Show detailed error output |
| `--log-level LEVEL` | Set library log level (`off` `critical` `error` `warning` `info` `debug` `trace`) |
