# Installation

## Download a release

Prebuilt archives for Linux and macOS (`.tar.gz`) and Windows (`.zip`) are
attached to every
[GitHub release](https://github.com/vanillapdf/vanillapdf-cli/releases).
Download the archive for your platform, extract it, and run `vanillapdf-cli`.

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

!!! warning "Unsigned binaries"
    Release binaries are **not code-signed or notarized**. Your operating
    system may warn you before running them:

    - **Windows** — Defender SmartScreen may show
      *"Windows protected your PC"*. Click **More info → Run anyway**.
    - **macOS** — Gatekeeper may block the binary. Run
      `xattr -d com.apple.quarantine ./vanillapdf-cli` after extracting, or
      right-click the binary and choose **Open**.

    This is expected for unsigned software. Signing is planned for a future
    release.

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

!!! note "Coming soon"
    Homebrew, apt (Debian/Ubuntu), and dnf (Fedora/RHEL) packages are planned
    for a future release. For now, use a release archive or build from source.
