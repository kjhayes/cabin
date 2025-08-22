#! /usr/bin/bash

#!/usr/bin/env bash
set -eu

root_dir=/home/kevin/kanawha-dev/sysroot/
img_file=./root.ext2

# Create a test directory to convert to ext2.
mkdir -p "$root_dir"

# Create a 32M ext2 without sudo.
# If 32M is not enough for the contents of the directory,
# it will fail.
rm -f "$img_file"
mke2fs \
  -L '' \
  -N 0 \
  -O ^64bit \
  -d "$root_dir" \
  -m 5 \
  -r 1 \
  -t ext2 \
  "$img_file" \
  64M \

