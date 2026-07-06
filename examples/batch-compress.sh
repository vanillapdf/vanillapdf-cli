#!/bin/sh
# Compress every PDF in a directory, writing results to an output directory.
#
# Usage: ./batch-compress.sh <input-dir> <output-dir>
set -eu

if [ "$#" -ne 2 ] || [ "$1" = "--help" ]; then
    echo "Usage: $0 <input-dir> <output-dir>" >&2
    exit 1
fi

input_dir="$1"
output_dir="$2"
mkdir -p "$output_dir"

found=0
for pdf in "$input_dir"/*.pdf; do
    [ -e "$pdf" ] || break
    found=1
    name="$(basename "$pdf")"
    echo "Compressing $name ..."
    vanillapdf-cli compress "$pdf" -o "$output_dir/$name"
done

if [ "$found" -eq 0 ]; then
    echo "No PDF files found in $input_dir" >&2
    exit 1
fi

echo "Done. Compressed files are in $output_dir"
