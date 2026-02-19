# attachments

Manage file attachments embedded in a PDF.

!!! warning "Not yet supported"
    Attachment support is not yet implemented in the vanillapdf library.
    The `add`, `list`, and `remove` subcommands are defined but return an error when invoked.

---

## add {#add}

Embed a file as an attachment.

### Synopsis

```
vanillapdf-cli attachments add <input> -o <output> -f <file> [-d <description>]
```

### Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Output PDF file |
| `-f`, `--file` | Yes | File to attach |
| `-d`, `--description` | No | Attachment description |

---

## list {#list}

List all file attachments in a PDF.

### Synopsis

```
vanillapdf-cli attachments list <input>
```

---

## remove {#remove}

Remove a named attachment from a PDF.

### Synopsis

```
vanillapdf-cli attachments remove <input> -o <output> -n <name>
```

### Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Output PDF file |
| `-n`, `--name` | Yes | Name of the attachment to remove |
