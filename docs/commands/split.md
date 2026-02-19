# split

Extract pages from a PDF into a new file.

## Synopsis

```
vanillapdf-cli split <input> -o <output> [-p <page>...]
```

## Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Output PDF file |
| `-p`, `--pages` | No | Page numbers to extract (1-based). Omit to copy all pages. |

## Examples

```sh
# Extract pages 1, 3, and 5
vanillapdf-cli split report.pdf -o odd-pages.pdf -p 1 3 5

# Extract a range (pages 10 through 20)
vanillapdf-cli split report.pdf -o section.pdf -p 10 11 12 13 14 15 16 17 18 19 20

# Copy all pages
vanillapdf-cli split report.pdf -o copy.pdf
```

Out-of-range page numbers are skipped with a warning printed to `stderr`.
