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
