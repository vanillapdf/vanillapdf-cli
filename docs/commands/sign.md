# sign

Manage digital signatures in a PDF.

---

## add {#add}

Apply a digital signature using a PKCS#12 certificate.

### Synopsis

```
vanillapdf-cli sign add <input> -o <output> -k <key-file> [-p <password>]
```

### Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Source PDF file |
| `-o`, `--output` | Yes | Signed output file |
| `-k`, `--key` | Yes | PKCS#12 key file (`.p12` or `.pfx`) |
| `-p`, `--password` | No | Key file password |

### Example

```sh
vanillapdf-cli sign add contract.pdf -o contract-signed.pdf \
    -k my-certificate.p12 -p "passphrase"
```

---

## list {#list}

List all digital signatures embedded in a PDF.

### Synopsis

```
vanillapdf-cli sign list <input>
```

### Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | PDF file to inspect |

### Example

```sh
vanillapdf-cli sign list contract-signed.pdf
```

```
File: contract-signed.pdf
Signature #1:
    Name: Jane Smith
    Reason: Approved
    Location: London, UK
    Contact: jane@example.com
Signatures: 1
```

Fields not present in the signature (Reason, Location, Contact) are omitted from the output.

---

## remove {#remove}

!!! warning "Not yet supported"
    Signature removal is not yet implemented in the vanillapdf library.
