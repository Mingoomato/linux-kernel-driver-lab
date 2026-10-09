#!/bin/sh
set -eu

if [ "${SECUREWATCH_ALLOW_LOAD:-}" != "YES" ]; then
    echo "Refusing to load kernel code without SECUREWATCH_ALLOW_LOAD=YES in a disposable VM." >&2
    exit 2
fi
if ! command -v systemd-detect-virt >/dev/null 2>&1 ||
   [ "$(systemd-detect-virt --vm 2>/dev/null || true)" = "none" ]; then
    echo "A detected virtual machine is required." >&2
    exit 2
fi
module="$(dirname "$0")/../kernel/securewatch.ko"
device=/dev/securewatch
sudo insmod "$module" allow_lab_injection=1
cleanup() { sudo rmmod securewatch 2>/dev/null || true; }
trap cleanup EXIT INT TERM
test -c "$device"
sudo "$(dirname "$0")/../userspace/swctl" doctor
sudo "$(dirname "$0")/../userspace/swctl" inject-lab-event --message vm-smoke
"$(dirname "$0")/../userspace/swctl" listen --count 1 --timeout-ms 3000
"$(dirname "$0")/../userspace/swctl" stats
