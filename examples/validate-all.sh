#!/bin/sh
# Validate the structure of every PDF in a directory.
# Exits non-zero if any file fails validation - suitable for CI gating.
#
# Usage: ./validate-all.sh <input-dir> [--strict]
set -eu

if [ "$#" -lt 1 ] || [ "$1" = "--help" ]; then
    echo "Usage: $0 <input-dir> [--strict]" >&2
    exit 1
fi

input_dir="$1"
strict="${2:-}"

failures=0
found=0
for pdf in "$input_dir"/*.pdf; do
    [ -e "$pdf" ] || break
    found=1
    if vanillapdf-cli validate "$pdf" $strict; then
        echo "OK:   $pdf"
    else
        echo "FAIL: $pdf" >&2
        failures=$((failures + 1))
    fi
done

if [ "$found" -eq 0 ]; then
    echo "No PDF files found in $input_dir" >&2
    exit 1
fi

echo
echo "$failures file(s) failed validation."
[ "$failures" -eq 0 ]
