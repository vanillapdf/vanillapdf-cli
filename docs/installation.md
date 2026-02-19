# Installation

## Homebrew (macOS / Linux)

```sh
brew install vanillapdf/tap/vanillapdf-cli
```

## Debian / Ubuntu

```sh
sudo apt install vanillapdf-cli
```

## RPM (Fedora / RHEL)

```sh
sudo dnf install vanillapdf-cli
```

## Build from source

See [Building from Source](building.md) for full details.

```sh
git clone https://github.com/vanillapdf/vanillapdf-cli.git
cd vanillapdf-cli
cmake -B build
cmake --build build
cmake --install build
```
