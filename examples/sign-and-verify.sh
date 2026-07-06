#!/bin/sh
# Sign a PDF with a PKCS#12 certificate, then verify the resulting signature.
#
# Usage: ./sign-and-verify.sh <input.pdf> <key.p12> [key-password]
set -eu

if [ "$#" -lt 2 ] || [ "$1" = "--help" ]; then
    echo "Usage: $0 <input.pdf> <key.p12> [key-password]" >&2
    exit 1
fi

input="$1"
key="$2"
password="${3:-}"
signed="${input%.pdf}-signed.pdf"

echo "Signing $input -> $signed ..."
if [ -n "$password" ]; then
    vanillapdf-cli sign add "$input" -o "$signed" -k "$key" -p "$password"
else
    vanillapdf-cli sign add "$input" -o "$signed" -k "$key"
fi

echo
echo "Listing signatures in $signed ..."
vanillapdf-cli sign list "$signed"

echo
echo "Verifying $signed ..."
vanillapdf-cli sign verify "$signed"
