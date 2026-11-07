# split

Extract pages from a PDF into a new file.

## Synopsis

```
vanillapdf-cli split <input> -o <output> [-p <range>]
```

## Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Output PDF file |
| `-p`, `--pages` | No | Pages to extract, as a [page range](index.md#page-ranges), e.g. `1-3,7,last`. Omit to copy all pages. |

The extracted pages keep their original order: `-p 3,1` and `-p 1,3` produce
the same file.

## Examples

```sh
# Extract pages 1, 3, and 5
vanillapdf-cli split report.pdf -o selection.pdf -p 1,3,5

# Extract a range (pages 10 through 20)
vanillapdf-cli split report.pdf -o section.pdf -p 10-20

# Everything from page 5 to the end
vanillapdf-cli split report.pdf -o rest.pdf -p 5-last

# Every odd page
vanillapdf-cli split report.pdf -o odd-pages.pdf -p odd

# Everything except the cover page
vanillapdf-cli split report.pdf -o no-cover.pdf -p "!first"

# Copy all pages
vanillapdf-cli split report.pdf -o copy.pdf
```

A page outside the document or a malformed range is an error, and no output
file is written.
