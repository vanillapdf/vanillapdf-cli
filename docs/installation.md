# Installation

## Download a release

Prebuilt archives for Linux (`.tar.gz`) and Windows (`.zip`) are attached to
every [GitHub release](https://github.com/vanillapdf/vanillapdf-cli/releases).
Download the archive for your platform, extract it, and run `vanillapdf-cli`.

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
