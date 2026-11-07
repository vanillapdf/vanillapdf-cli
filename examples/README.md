# Examples

Runnable scripts showing common `vanillapdf-cli` workflows. Each is a small
POSIX shell script that assumes `vanillapdf-cli` is on your `PATH` (on Windows,
run them from Git Bash or WSL).

| Script | What it does |
|--------|--------------|
| [`merge-directory.sh`](merge-directory.sh) | Merge every PDF in a directory (sorted) into one file |
| [`batch-compress.sh`](batch-compress.sh) | Compress every PDF in a directory into an output folder |
| [`sign-and-verify.sh`](sign-and-verify.sh) | Sign a PDF with a PKCS#12 key, then verify the result |
| [`validate-all.sh`](validate-all.sh) | Validate every PDF in a directory; exit non-zero if any fail (CI-friendly) |

Make a script executable and run it with `--help` (or no arguments) to see usage:

```sh
chmod +x examples/*.sh
./examples/merge-directory.sh ./chapters merged.pdf
```

See the [command reference](https://vanillapdf.readthedocs.io/projects/cli/en/latest/commands/) for
the full set of options.
