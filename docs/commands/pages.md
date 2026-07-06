# pages

Page-level editing: delete, reorder, or insert pages. Unlike `split` (which
extracts pages into a fresh document), `pages` edits the document in place and
preserves the rest of its structure. All page numbers are **1-based**.

---

## delete {#delete}

Delete one or more pages.

```
vanillapdf-cli pages delete <input> -o <output> -p <pages>...
```

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Output PDF file |
| `-p`, `--pages` | Yes | Page numbers to delete (1-based) |

Deleting every page is rejected. Out-of-range page numbers are skipped with a warning.

```sh
vanillapdf-cli pages delete report.pdf -o trimmed.pdf -p 2 5
```

---

## reorder {#reorder}

Reorder the pages of a document.

```
vanillapdf-cli pages reorder <input> -o <output> --order <permutation>
```

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Output PDF file |
| `--order` | Yes | Comma-separated new order — a permutation of all pages |

`--order` must list every page exactly once (e.g. `3,1,2` for a 3-page
document); anything else is rejected.

```sh
vanillapdf-cli pages reorder report.pdf -o reordered.pdf --order 3,1,2
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
