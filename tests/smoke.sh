#!/bin/sh
set -eu

root_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
compiler=${CC:-cc}
test_cflags=${CFLAGS:-"-O2 -std=c17 -Wall -Wextra -Wpedantic -Werror"}

make -C "$root_dir" userspace
"$compiler" -std=c17 -Wall -Wextra -Wpedantic -Werror \
    -I"$root_dir/include/uapi" "$root_dir/tests/uapi_layout.c" -o "$tmp/uapi-layout"
"$tmp/uapi-layout"
if "$compiler" -m32 -std=c17 -Wall -Wextra -Wpedantic -Werror \
    -I"$root_dir/include/uapi" -c "$root_dir/tests/uapi_layout.c" -o "$tmp/uapi-layout-32.o"; then
    echo "KDL UAPI layout: 32-bit compile passed"
else
    echo "KDL UAPI layout: 32-bit compiler unavailable; skipped"
fi
# CFLAGS is intentionally split into compiler arguments for sanitizer runs.
# shellcheck disable=SC2086
"$compiler" $test_cflags \
    "$root_dir/tests/json_output_test.c" -o "$tmp/json-output-test"
"$tmp/json-output-test"
"$compiler" $test_cflags -DKDL_SWCTL_NO_MAIN -Dpoll=sw_test_poll -Dread=sw_test_read \
    -I"$root_dir/userspace/include" -I"$root_dir/include/uapi" \
    -c "$root_dir/userspace/src/swctl.c" -o "$tmp/swctl-listen-test.o"
"$compiler" $test_cflags \
    -I"$root_dir/userspace/include" -I"$root_dir/include/uapi" \
    "$root_dir/tests/listen_client_test.c" "$tmp/swctl-listen-test.o" \
    -o "$tmp/listen-client-test"
"$tmp/listen-client-test"
"$root_dir/userspace/swctl" --help >/dev/null
"$root_dir/userspace/swctl" unload-check --device "$tmp/no-device"
set +e
"$root_dir/userspace/swctl" listen --count 1x --device "$tmp/no-device" >"$tmp/bad-count.out" 2>&1
bad_rc=$?
set -e
[ "$bad_rc" -eq 2 ]
grep -q 'invalid listen options' "$tmp/bad-count.out"
set +e
"$root_dir/userspace/swctl" listen --count 0 --device "$tmp/no-device" >"$tmp/zero-count.out" 2>&1
bad_rc=$?
set -e
[ "$bad_rc" -eq 2 ]
grep -q 'invalid listen options' "$tmp/zero-count.out"
set +e
"$root_dir/userspace/swctl" listen --timeout-ms 2147483648 --device "$tmp/no-device" >"$tmp/large-timeout.out" 2>&1
bad_rc=$?
set -e
[ "$bad_rc" -eq 2 ]
grep -q 'invalid listen options' "$tmp/large-timeout.out"
set +e
"$root_dir/userspace/swctl" listen --count --device "$tmp/no-device" >"$tmp/missing-count.out" 2>&1
bad_rc=$?
set -e
[ "$bad_rc" -eq 2 ]
grep -q 'invalid listen options' "$tmp/missing-count.out"
set +e
"$root_dir/userspace/swctl" listen --count 1 --count 2 --device "$tmp/no-device" >"$tmp/duplicate-count.out" 2>&1
bad_rc=$?
set -e
[ "$bad_rc" -eq 2 ]
grep -q 'invalid listen options' "$tmp/duplicate-count.out"
set +e
"$root_dir/userspace/swctl" listen --unknown --device "$tmp/no-device" >"$tmp/unknown-option.out" 2>&1
bad_rc=$?
set -e
[ "$bad_rc" -eq 2 ]
grep -q 'invalid listen options' "$tmp/unknown-option.out"
set +e
SECUREWATCH_ALLOW_LOAD= "$root_dir/scripts/vm-test.sh" >"$tmp/vm-guard.out" 2>&1
guard_rc=$?
set -e
[ "$guard_rc" -eq 2 ]
grep -q 'Refusing to load kernel code' "$tmp/vm-guard.out"
set +e
SECUREWATCH_ALLOW_LOAD=YES SECUREWATCH_VM_SNAPSHOT_READY= \
    "$root_dir/scripts/vm-test.sh" >"$tmp/vm-snapshot-guard.out" 2>&1
guard_rc=$?
set -e
[ "$guard_rc" -eq 2 ]
grep -q 'confirm a disposable VM snapshot' "$tmp/vm-snapshot-guard.out"
set +e
SECUREWATCH_ALLOW_LOAD=YES SECUREWATCH_VM_SNAPSHOT_READY=YES \
SECUREWATCH_RECOVERY_CONSOLE_READY= \
    "$root_dir/scripts/vm-test.sh" >"$tmp/vm-console-guard.out" 2>&1
guard_rc=$?
set -e
[ "$guard_rc" -eq 2 ]
grep -q 'confirm a recovery console' "$tmp/vm-console-guard.out"

echo "KDL userspace/UAPI smoke: PASS (module not loaded)"
