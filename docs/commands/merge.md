# merge

Combine multiple PDF files into a single output file.

## Synopsis

```
vanillapdf-cli merge <input>... -o <output>
```

## Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes (one or more) | Input PDF files |
| `-o`, `--output` | Yes | Output PDF file |

## Examples

```sh
# Merge three chapters into a book
vanillapdf-cli merge chapter1.pdf chapter2.pdf chapter3.pdf -o book.pdf

# Combine a cover page with the document body
vanillapdf-cli merge cover.pdf body.pdf -o final.pdf
```

Pages are appended in the order the input files are given.
