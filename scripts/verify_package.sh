#!/usr/bin/env bash
# Verify a release archive the way a user receives it: unpacked on its own, with
# nothing from the build tree around it.
#
# Usage: verify_package.sh <archive> <expected-version> <sample.pdf>
set -euo pipefail

archive=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
expected_version=$2
sample_pdf=$3

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

case "$archive" in
    *.zip) (cd "$work" && cmake -E tar xf "$archive") ;;
    *)     tar -xzf "$archive" -C "$work" ;;
esac

root="$work/$(basename "$archive" | sed -E 's/\.(zip|tar\.gz)$//')"
cli="$root/bin/vanillapdf-cli"
[ -f "$cli" ] || cli="$cli.exe"

fail() {
    echo "::error::$archive: $1" >&2
    exit 1
}

# CLI11 prints the bare version; tr drops the carriage return on Windows.
version=$("$cli" --version | tr -d '\r')
[ "$version" = "$expected_version" ] ||
    fail "reports version '$version', expected '$expected_version'"

"$cli" --help > /dev/null || fail "--help failed"
"$cli" info "$sample_pdf" > /dev/null || fail "info failed on $sample_pdf"

# Only the CLI belongs in the package, not the library it links.
unexpected=$(cd "$root" && find . -type f \
    ! -path './bin/vanillapdf-cli' ! -path './bin/vanillapdf-cli.exe' \
    ! -path './share/doc/vanillapdf-cli/*')
[ -z "$unexpected" ] || fail "unexpected files in the package:
$unexpected"

# The binary links these statically, so their notices must ship with it.
notice="$root/share/doc/vanillapdf-cli/NOTICE.md"
[ -s "$notice" ] || fail "missing NOTICE.md"
for name in OpenSSL libjpeg-turbo zlib OpenJPEG spdlog fmt CLI11; do
    grep -Eq "^# $name[[:space:]]*$" "$notice" || fail "NOTICE.md has no section for $name"
done

echo "$archive: OK (version $version)"
