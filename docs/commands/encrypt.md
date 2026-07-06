# encrypt

Add or remove password encryption on a PDF.

---

## add {#add}

Encrypt a PDF with a user password (required to open) and an optional owner password (required to edit).

### Synopsis

```
vanillapdf-cli encrypt add <input> -o <output> -p <user-password> [--owner-password <password>]
    [--algorithm aes|rc4] [--bits <n>] [--allow <perms>]
```

### Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Encrypted output file |
| `-p`, `--user-password` | Yes | Password required to open the document |
| `--owner-password` | No | Password required to edit the document |
| `--algorithm` | No | Cipher: `aes` (default) or `rc4` (legacy, insecure) |
| `--bits` | No | Key length. AES: `128`/`256` (default `256`); RC4: `40`/`128` (default `128`) |
| `--allow` | No | Comma-separated permissions to grant; omit to allow everything |

### Permissions

`--allow` accepts any combination of: `print`, `print-high`, `modify`, `copy`,
`annotate`, `fill-forms`, `extract`, `assemble` (plus `all` and `none`). Any
permission not listed is denied. When `--allow` is omitted, all permissions are
granted.

### Example

```sh
# AES-256 with a user password only (default)
vanillapdf-cli encrypt add report.pdf -o report-secured.pdf -p "open123"

# AES-128 with both user and owner passwords
vanillapdf-cli encrypt add report.pdf -o report-secured.pdf \
    -p "open123" --owner-password "edit456" --bits 128

# Grant printing and copying only
vanillapdf-cli encrypt add report.pdf -o report-restricted.pdf \
    -p "open123" --allow print,copy
```

!!! note
    Encryption uses AES-256 by default. RC4 is supported for compatibility with
    legacy readers but is insecure and should be avoided.

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
