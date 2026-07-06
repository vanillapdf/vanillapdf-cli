# info

Display metadata and page count for a PDF file.

## Synopsis

```
vanillapdf-cli info <input> [--json]
```

## Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | PDF file to inspect |
| `--json` | No | Emit results as JSON |

## Example

```sh
vanillapdf-cli info annual-report.pdf
```

```
File:        annual-report.pdf
PDF Version: 1.7
Pages:       42
Title:       Annual Report 2025
Author:    Jane Smith
Subject:   Financial results
Keywords:  N/A
Creator:   LibreOffice
Producer:  vanillapdf 2.2.0
```

Fields not present in the document are shown as `N/A`.

## JSON output

```sh
vanillapdf-cli info annual-report.pdf --json
```

```json
{
  "file": "annual-report.pdf",
  "pdf_version": "1.7",
  "pages": 42,
  "title": "Annual Report 2025",
  "author": "Jane Smith",
  "subject": "Financial results",
  "keywords": null,
  "creator": "LibreOffice",
  "producer": "vanillapdf 2.2.0"
}
```

Fields (and `pdf_version`) that are absent or unparseable are emitted as `null`,
so the object always has the same keys.
