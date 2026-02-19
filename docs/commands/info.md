# info

Display metadata and page count for a PDF file.

## Synopsis

```
vanillapdf-cli info <input>
```

## Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | PDF file to inspect |

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
