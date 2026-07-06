#!/bin/sh
# Merge every PDF in a directory (sorted by name) into a single output file.
#
# Usage: ./merge-directory.sh <input-dir> <output.pdf>
set -eu

if [ "$#" -ne 2 ] || [ "$1" = "--help" ]; then
    echo "Usage: $0 <input-dir> <output.pdf>" >&2
    exit 1
fi

input_dir="$1"
output="$2"

# Collect PDFs in sorted order into the positional parameters.
set --
for pdf in "$input_dir"/*.pdf; do
    [ -e "$pdf" ] || { echo "No PDF files found in $input_dir" >&2; exit 1; }
    set -- "$@" "$pdf"
done

echo "Merging $# file(s) into $output ..."
vanillapdf-cli merge "$@" -o "$output"
echo "Done: $output"
