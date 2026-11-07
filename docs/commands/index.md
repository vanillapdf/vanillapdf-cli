# Commands

| Command | Description |
|---|---|
| [`info`](info.md) | Display file metadata and page count |
| [`metadata set`](metadata.md) | Write document information fields |
| [`merge`](merge.md) | Combine multiple PDFs into one |
| [`split`](split.md) | Extract pages into a new PDF |
| [`pages`](pages.md) | Delete or insert pages |
| [`rotate`](rotate.md) | Rotate pages by 90 / 180 / 270 degrees |
| [`compress`](compress.md) | Compress and optimise a PDF |
| [`encrypt add`](encrypt.md#add) | Add password encryption (AES-256 by default) |
| [`encrypt remove`](encrypt.md#remove) | Remove encryption |
| [`sign add`](sign.md#add) | Apply a digital signature (PKCS#12) |
| [`sign list`](sign.md#list) | List signatures in a PDF |
| [`validate`](validate.md) | Validate PDF file structure |

## Conventions

- **Input files** are positional arguments and must exist on disk.
- **Output files** use `-o` / `--output` and are always required for write operations.
- **Pages** are selected with a [page range](#page-ranges), numbered from 1.
- Non-zero exit code on error; messages go to `stderr`.

## Page ranges

Commands that work on some of a document's pages (`split`, `rotate`,
`pages delete`) take a page range with `-p` / `--pages`: one argument made of
comma-separated entries.

| Entry | Selects | Example (10-page document) |
|---|---|---|
| `N` | Page *N* | `3` → 3 |
| `A-B` | Pages *A* through *B*, inclusive (`4-2` is the same as `2-4`) | `2-4` → 2, 3, 4 |
| `first`, `last` | The first / last page; usable anywhere a number is | `8-last` → 8, 9, 10 |
| `begin`, `end` | Aliases for `first` and `last` | `begin-3` → 1, 2, 3 |
| `odd`, `even` | Every odd / even page | `even` → 2, 4, 6, 8, 10 |
| `!entry` | Removes the pages of *entry* from the selection | `1-5,!3` → 1, 2, 4, 5 |

Combining entries:

- A range selects a *set* of pages: `1-3,7,last` → 1, 2, 3, 7, 10. The order
  of entries and repeated pages do not matter — `last,1-3,2` selects the same
  pages, and every command processes them in document order.
- Exclusions apply after everything else, wherever they appear: `!3,1-5` is the
  same as `1-5,!3`.
- A range made only of exclusions starts from every page: `!1` → all pages
  except the first; `!first,!last` → everything but the cover and back page.
- Spaces around entries are ignored, but quote the argument if it contains
  spaces or a `!` (some shells treat `!` specially): `-p "1-3, !2"`.

A page outside the document (for example `12` in a 10-page file) or a malformed
entry is an error. The command stops without writing any output.

```sh
vanillapdf-cli split   report.pdf -o summary.pdf -p 1-3,last
vanillapdf-cli rotate  scan.pdf   -o fixed.pdf   -a 180 -p even
vanillapdf-cli pages delete report.pdf -o no-cover.pdf -p first
vanillapdf-cli split   report.pdf -o body.pdf    -p "!first,!last"
```
