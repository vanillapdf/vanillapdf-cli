# validate

Validate the structural integrity of a PDF file.

The validator inspects the file header, cross-reference tables, and trailer,
reporting any issues it finds as **Info**, **Warning**, or **Error**. It does
*not* alter the file.

## Synopsis

```
vanillapdf-cli validate <input> [--strict] [--json]
```

## Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | PDF file to validate |
| `--strict` | No | Exit with failure if any warnings are found (by default only errors cause failure) |
| `--json` | No | Emit results as JSON |

## Example

```sh
vanillapdf-cli validate report.pdf
```

```
File: report.pdf
Valid: yes
Errors: 0  Warnings: 1
[Warning] TrailerSizeMismatch (5): Trailer /Size does not match the number of xref entries
```

## JSON output

```sh
vanillapdf-cli validate report.pdf --json
```

```json
{
  "file": "report.pdf",
  "valid": true,
  "error_count": 0,
  "warning_count": 1,
  "issues": [
    {
      "severity": "Warning",
      "code": 5,
      "name": "TrailerSizeMismatch",
      "message": "Trailer /Size does not match the number of xref entries"
    }
  ]
}
```

Each issue carries a `severity`, a numeric `code` with its symbolic `name`, and a
`message`. When an issue is tied to a specific indirect object, the entry also
includes its `object` and `generation` numbers.

## Exit codes

| Condition | Exit code |
|---|---|
| No errors (and no warnings, or `--strict` not set) | `0` |
| One or more errors | `1` |
| Warnings present with `--strict` | `1` |
