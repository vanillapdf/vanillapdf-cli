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
vanillapdf-cli sign list <input> [--json]
```

### Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | PDF file to inspect |
| `--json` | No | Emit results as JSON |

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
    Signing Time: 2026-05-14T09:30:00+01:00
Signatures: 1
```

For each signature the Name, Reason, Location, Contact, and Signing Time (`/M`,
formatted as ISO 8601) are shown; fields not present are omitted. A signature
that cannot be read is reported as an error entry and does not abort the listing.

### JSON output

```sh
vanillapdf-cli sign list contract-signed.pdf --json
```

```json
{
  "file": "contract-signed.pdf",
  "signatures": [
    {
      "index": 1,
      "name": "Jane Smith",
      "reason": "Approved",
      "location": "London, UK",
      "contact": "jane@example.com",
      "signing_time": "2026-05-14T09:30:00+01:00",
      "error": null
    }
  ]
}
```

Absent fields are `null`, so every signature object has the same keys. If a
signature could not be read, `error` holds the reason and the other fields are
`null`.

---

## remove {#remove}

!!! warning "Not yet supported"
    Signature removal is not yet implemented in the vanillapdf library.

---

## verify {#verify}

Verify the digital signatures in a PDF using system or custom trusted certificates.

### Synopsis

```
vanillapdf-cli sign verify <input> [options]
```

### Arguments

| Argument | Required | Description |
|---|---|---|
| `input` | Yes | Signed PDF file to verify |
| `-c`, `--trusted-cert` | No | PEM certificate file to add to the trust store (repeatable) |
| `--no-system-certs` | No | Do not load system default trusted certificates |
| `--skip-cert-validation` | No | Skip certificate chain validation (cryptographic check only) |
| `--check-signing-time` | No | Verify certificate validity at the time of signing |
| `--allow-weak` | No | Allow weak digest algorithms (MD5, SHA-1, RSA < 2048) |
| `--json` | No | Output results as JSON |

### Example

```sh
vanillapdf-cli sign verify contract-signed.pdf
```

```
File: contract-signed.pdf
Signature #1:
  Status: Valid
  Signature valid: yes
  Document intact: yes
  Certificate trusted: yes
  Signer: Jane Smith
```

With a custom trust store and JSON output:

```sh
vanillapdf-cli sign verify contract-signed.pdf \
    --no-system-certs -c my-ca.pem --json
```

```json
{
  "file": "contract-signed.pdf",
  "signatures": [
    {
      "index": 1,
      "status": "CertificateUntrusted",
      "signature_valid": true,
      "document_intact": true,
      "certificate_trusted": false,
      "signer": "Jane Smith"
    }
  ]
}
```
