# Test results

Status: userspace, UAPI layout, and compile-only module checks passed on the current host.

- Userspace C17 build: passed.
- `make test`: passed; compile-time assertions verified ABI v1 struct sizes, offsets, and ioctl size, and the CLI's non-device checks passed.
- `make module`: passed against Linux `7.1.5+kali-amd64` headers.
- After correcting blocking `read` to re-check the ring after wakeups, `make test` and `make module` were rerun and passed.
- Runtime environment check: `systemd-detect-virt --vm` reported `none`. The module was not loaded.

Not run: module load/unload, blocking read/poll, injected-event behavior, ring overflow, concurrency, KASAN/KCSAN, a second kernel version, or compatibility matrix execution. Runtime verification requires a disposable Linux VM with a snapshot and recovery console.
