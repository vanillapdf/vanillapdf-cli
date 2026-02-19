# rotate

Rotate pages in a PDF by a given angle.

## Synopsis

```
vanillapdf-cli rotate <input> -o <output> -a <angle> [-p <page>...]
```

## Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Output PDF file |
| `-a`, `--angle` | Yes | Rotation angle: `90`, `180`, or `270` |
| `-p`, `--pages` | No | Pages to rotate (1-based). Omit to rotate all pages. |

## Examples

```sh
# Fix a document scanned sideways (rotate all pages 90° clockwise)
vanillapdf-cli rotate scanned.pdf -o fixed.pdf -a 90

# Flip a single page upside-down
vanillapdf-cli rotate document.pdf -o flipped.pdf -a 180 -p 3

# Rotate the first and last pages 270°
vanillapdf-cli rotate document.pdf -o adjusted.pdf -a 270 -p 1 10
```

!!! note
    Rotation is **cumulative** with any existing page rotation stored in the PDF.
    A page already rotated 90° that receives another 90° rotation will end up at 180°.

Out-of-range page numbers are skipped with a warning printed to `stderr`.
