# encrypt

Add or remove password encryption on a PDF.

---

## add {#add}

Encrypt a PDF with a user password (required to open) and an optional owner password (required to edit).

### Synopsis

```
vanillapdf-cli encrypt add <input> -o <output> -p <user-password> [--owner-password <password>]
```

### Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Encrypted output file |
| `-p`, `--user-password` | Yes | Password required to open the document |
| `--owner-password` | No | Password required to edit the document |

### Example

```sh
# Encrypt with a user password only
vanillapdf-cli encrypt add report.pdf -o report-secured.pdf -p "open123"

# Encrypt with both user and owner passwords
vanillapdf-cli encrypt add report.pdf -o report-secured.pdf \
    -p "open123" --owner-password "edit456"
```

!!! note
    Encryption uses AES with a 128-bit key.

---

## remove {#remove}

Remove encryption from a password-protected PDF.

### Synopsis

```
vanillapdf-cli encrypt remove <input> -o <output> -p <password>
```

### Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Encrypted source PDF file |
| `-o`, `--output` | Yes | Decrypted output file |
| `-p`, `--password` | Yes | Document password |

### Example

```sh
vanillapdf-cli encrypt remove report-secured.pdf -o report-plain.pdf -p "open123"
```
