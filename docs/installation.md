# Installation

## Download a release

Prebuilt archives for Linux and macOS (`.tar.gz`) and Windows (`.zip`) are
attached to every
[GitHub release](https://github.com/vanillapdf/vanillapdf-cli/releases).
Download the archive for your platform, extract it, and run `vanillapdf-cli`.

| Platform | Architectures | Minimum version |
|----------|---------------|-----------------|
| Linux | x64, arm64 | glibc 2.34: Ubuntu 22.04, Debian 12, RHEL/Rocky 9 |
| Windows | x64, arm64 | |
| macOS | x64, arm64 | macOS 14 |

Older Linux distributions (Ubuntu 20.04, RHEL/Rocky 8) are not supported by the
prebuilt archives. Build from source there.

## Verify your download

Each archive is published with a matching `.sha256` file. Verify the checksum
before extracting:

```sh
# Linux / macOS
sha256sum -c vanillapdf-cli-<version>-<platform>.tar.gz.sha256
```

```powershell
# Windows (PowerShell)
$expected = (Get-Content vanillapdf-cli-<version>-windows-x64.zip.sha256).Split()[0]
(Get-FileHash vanillapdf-cli-<version>-windows-x64.zip -Algorithm SHA256).Hash -eq $expected
```

For a stronger guarantee, releases also carry **build provenance
attestations**. If you have the [GitHub CLI](https://cli.github.com/), verify
that an archive was built by this repository's release workflow:

```sh
gh attestation verify vanillapdf-cli-<version>-<platform>.zip \
  --repo vanillapdf/vanillapdf-cli
```

````{admonition} macOS Gatekeeper
:class: note
Release binaries are not notarized yet.
If you download the archive with a browser, macOS blocks the binary on first run
with *"cannot be opened because the developer cannot be verified."*

Downloads with `curl` or the GitHub CLI are not affected:

```sh
gh release download --repo vanillapdf/vanillapdf-cli --pattern '*macos-arm64*'
```

If the binary is already blocked, clear the quarantine flag:

```sh
xattr -d com.apple.quarantine ./vanillapdf-cli
```

or allow it in **System Settings → Privacy & Security → Open Anyway**.
````

## Build from source

See [Building from Source](building.md) for full details.

```sh
git clone https://github.com/vanillapdf/vanillapdf-cli.git
cd vanillapdf-cli
cmake -B build
cmake --build build
cmake --install build
```

## Package managers

```{admonition} Coming soon
:class: note
Homebrew, apt (Debian/Ubuntu), and dnf (Fedora/RHEL) packages are planned
for a future release. For now, use a release archive or build from source.
```
