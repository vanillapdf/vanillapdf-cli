# pages

Page-level editing: delete or insert pages. Unlike `split` (which
extracts pages into a fresh document), `pages` edits the document in place and
preserves the rest of its structure. All page numbers are **1-based**.

---

## delete {#delete}

Delete one or more pages.

```
vanillapdf-cli pages delete <input> -o <output> -p <range>
```

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Output PDF file |
| `-p`, `--pages` | Yes | Pages to delete, as a [page range](index.md#page-ranges), e.g. `2,5` or `first` |

Deleting every page is rejected. A page outside the document or a malformed
range is an error, and no output file is written.

```sh
# Delete pages 2 and 5
vanillapdf-cli pages delete report.pdf -o trimmed.pdf -p 2,5

# Drop the cover page
vanillapdf-cli pages delete report.pdf -o no-cover.pdf -p first

# Keep only page 3 (delete everything else)
vanillapdf-cli pages delete report.pdf -o page3.pdf -p "!3"
```

---

## insert {#insert}

Insert the pages of another PDF into this one.

```
vanillapdf-cli pages insert <input> -o <output> --from <other.pdf> [--at <n>]
```

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Target PDF file |
| `-o`, `--output` | Yes | Output PDF file |
| `--from` | Yes | PDF whose pages are inserted |
| `--at` | No | 1-based position to insert before (default: append at the end) |

```sh
# Insert cover.pdf before page 1
vanillapdf-cli pages insert report.pdf -o with-cover.pdf --from cover.pdf --at 1

# Append appendix.pdf at the end
vanillapdf-cli pages insert report.pdf -o combined.pdf --from appendix.pdf
```
