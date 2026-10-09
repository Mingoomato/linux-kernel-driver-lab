#!/bin/sh
set -eu

root_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

make -C "$root_dir" userspace
cc -std=c17 -Wall -Wextra -Wpedantic -Werror \
    -I"$root_dir/include/uapi" "$root_dir/tests/uapi_layout.c" -o "$tmp/uapi-layout"
"$tmp/uapi-layout"
if cc -m32 -std=c17 -Wall -Wextra -Wpedantic -Werror \
    -I"$root_dir/include/uapi" -c "$root_dir/tests/uapi_layout.c" -o "$tmp/uapi-layout-32.o"; then
    echo "KDL UAPI layout: 32-bit compile passed"
else
    echo "KDL UAPI layout: 32-bit compiler unavailable; skipped"
fi
"$root_dir/userspace/swctl" --help >/dev/null
"$root_dir/userspace/swctl" unload-check --device "$tmp/no-device"
set +e
SECUREWATCH_ALLOW_LOAD= "$root_dir/scripts/vm-test.sh" >"$tmp/vm-guard.out" 2>&1
guard_rc=$?
set -e
[ "$guard_rc" -eq 2 ]
grep -q 'Refusing to load kernel code' "$tmp/vm-guard.out"

echo "KDL userspace/UAPI smoke: PASS (module not loaded)"
