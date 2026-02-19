# compress

Compress and optimise a PDF file.

## Synopsis

```
vanillapdf-cli compress <input> -o <output>
```

## Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Output PDF file |

## Example

```sh
vanillapdf-cli compress large.pdf -o small.pdf
```

Applies vanillapdf's built-in compression when saving. Results vary depending on how the original file was created and whether its content streams are already compressed.
