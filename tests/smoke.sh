#!/bin/sh
set -eu

root_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

make -C "$root_dir" userspace
cc -std=c17 -Wall -Wextra -Wpedantic -Werror \
    -I"$root_dir/include/uapi" "$root_dir/tests/uapi_layout.c" -o "$tmp/uapi-layout"
"$tmp/uapi-layout"
"$root_dir/userspace/swctl" --help >/dev/null
"$root_dir/userspace/swctl" unload-check --device "$tmp/no-device"

echo "KDL userspace/UAPI smoke: PASS (module not loaded)"
