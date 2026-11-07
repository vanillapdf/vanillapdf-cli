# metadata

Read or write the document information dictionary (title, author, and related
fields).

## Synopsis

```
vanillapdf-cli metadata set <input> -o <output> [field options] [-p <password>]
```

## `set`

Writes the document information fields and saves a new file. Fields describing
the document's *content* are user-settable; the fields describing its
*production* — `Producer` and `ModDate` — are always stamped by the tool and
cannot be overridden.

### Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | PDF file to edit |
| `-o`, `--output` | Yes | Output PDF file |
| `--title` | No | Document title |
| `--author` | No | Document author |
| `--subject` | No | Document subject |
| `--keywords` | No | Document keywords |
| `--creator` | No | Application that created the original |
| `-p`, `--password` | No | Password for an encrypted input (required to re-encrypt on save) |

### Field semantics

Each field option is three-way:

- **Omitted** — the existing value is left untouched.
- **Empty string** (e.g. `--title ""`) — the key is removed from the document.
- **Any other value** — the key is set to that value.

If the document has no information dictionary, one is created.

### Examples

```sh
# Set title and author, leave everything else as-is
vanillapdf-cli metadata set report.pdf -o tagged.pdf \
    --title "Annual Report 2025" --author "Jane Smith"

# Remove the title
vanillapdf-cli metadata set tagged.pdf -o untitled.pdf --title ""

# Edit metadata on an encrypted document
vanillapdf-cli metadata set secured.pdf -o secured-tagged.pdf -p "open123" \
    --subject "Confidential"
```

## Producer and ModDate

Every write republishes the document, so the tool records itself as the
producer and sets the modification date to the current time (UTC):

```
/Producer (vanillapdf-cli 0.1.0)
/ModDate  (D:20260707120000+00'00')
```

These are stamped on every `metadata set`, regardless of the field options
given.
