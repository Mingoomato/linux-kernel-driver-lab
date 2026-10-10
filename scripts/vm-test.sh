#!/bin/sh
set -eu

if [ "${SECUREWATCH_ALLOW_LOAD:-}" != "YES" ]; then
    echo "Refusing to load kernel code without SECUREWATCH_ALLOW_LOAD=YES in a disposable VM." >&2
    exit 2
fi
if [ "${SECUREWATCH_VM_SNAPSHOT_READY:-}" != "YES" ]; then
    echo "Refusing to load: confirm a disposable VM snapshot with SECUREWATCH_VM_SNAPSHOT_READY=YES." >&2
    exit 2
fi
if [ "${SECUREWATCH_RECOVERY_CONSOLE_READY:-}" != "YES" ]; then
    echo "Refusing to load: confirm a recovery console with SECUREWATCH_RECOVERY_CONSOLE_READY=YES." >&2
    exit 2
fi
if ! command -v systemd-detect-virt >/dev/null 2>&1 ||
   [ "$(systemd-detect-virt --vm 2>/dev/null || true)" = "none" ]; then
    echo "A detected virtual machine is required." >&2
    exit 2
fi
module="$(dirname "$0")/../kernel/securewatch.ko"
device=/dev/securewatch
swctl="$(dirname "$0")/../userspace/swctl"
loaded=0
cleanup() {
    if [ "$loaded" -eq 1 ]; then sudo rmmod securewatch 2>/dev/null || true; fi
}
trap cleanup EXIT INT TERM
sudo insmod "$module"
loaded=1
test -c "$device"
sudo "$swctl" doctor
if disabled_output=$(sudo env LC_ALL=C "$swctl" inject-lab-event --message vm-default-disabled 2>&1); then
    echo "Synthetic injection unexpectedly succeeded with the default module parameter." >&2
    exit 1
fi
case "$disabled_output" in
    *"Operation not supported"*) ;;
    *) echo "Default-off injection failed for an unexpected reason: $disabled_output" >&2; exit 1 ;;
esac
sudo rmmod securewatch
loaded=0
test ! -e "$device"
sudo insmod "$module" allow_lab_injection=1
loaded=1
test -c "$device"
sudo "$swctl" doctor
sudo "$swctl" inject-lab-event --message vm-smoke
sudo "$swctl" listen --count 1 --timeout-ms 3000
sudo "$swctl" stats
sudo rmmod securewatch
loaded=0
test ! -e "$device"
