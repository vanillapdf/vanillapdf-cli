# Commands

| Command | Description |
|---|---|
| [`info`](info.md) | Display file metadata and page count |
| [`merge`](merge.md) | Combine multiple PDFs into one |
| [`split`](split.md) | Extract pages into a new PDF |
| [`rotate`](rotate.md) | Rotate pages by 90 / 180 / 270 degrees |
| [`compress`](compress.md) | Compress and optimise a PDF |
| [`encrypt add`](encrypt.md#add) | Add password encryption (AES 128-bit) |
| [`encrypt remove`](encrypt.md#remove) | Remove encryption |
| [`sign add`](sign.md#add) | Apply a digital signature (PKCS#12) |
| [`sign list`](sign.md#list) | List signatures in a PDF |
| [`validate`](validate.md) | Validate PDF file structure |

## Conventions

- **Input files** are positional arguments and must exist on disk.
- **Output files** use `-o` / `--output` and are always required for write operations.
- **Page numbers** are 1-based. Out-of-range values are skipped with a warning.
- Non-zero exit code on error; messages go to `stderr`.
