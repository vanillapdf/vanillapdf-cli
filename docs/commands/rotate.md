# rotate

Rotate pages in a PDF by a given angle.

## Synopsis

```
vanillapdf-cli rotate <input> -o <output> -a <angle> [-p <range>]
```

## Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Output PDF file |
| `-a`, `--angle` | Yes | Clockwise rotation angle: `90`, `180`, or `270` |
| `-p`, `--pages` | No | Pages to rotate, as a [page range](index.md#page-ranges), e.g. `1-3,last`. Omit to rotate all pages. |

A page listed more than once (for example `-p 1-3,2`) is rotated only once.

## Examples

```sh
# Fix a document scanned sideways (rotate all pages 90° clockwise)
vanillapdf-cli rotate scanned.pdf -o fixed.pdf -a 90

# Flip a single page upside-down
vanillapdf-cli rotate document.pdf -o flipped.pdf -a 180 -p 3

# Rotate the first and last pages 270°
vanillapdf-cli rotate document.pdf -o adjusted.pdf -a 270 -p first,last

# Duplex scan where every back side came out upside-down
vanillapdf-cli rotate duplex.pdf -o fixed.pdf -a 180 -p even
```

```{note}
Rotation is **cumulative** with any existing page rotation stored in the PDF.
A page already rotated 90° that receives another 90° rotation will end up at 180°.
```

A page outside the document or a malformed range is an error, and no output
file is written.
