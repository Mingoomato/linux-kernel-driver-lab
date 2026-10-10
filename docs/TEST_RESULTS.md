# Test results

Status: userspace, UAPI layout, and compile-only module checks passed on the current host.

- GCC and Clang userspace C17 builds: passed.
- GCC and Clang `make test`: passed; native compile-time assertions verified ABI v1 struct sizes, field offsets, and ioctl encoding; optional 32-bit (`-m32`) layout compilation passed; CLI non-device checks passed; VM loader refused to proceed without load opt-in and without snapshot/recovery-console confirmations.
- Mocked listener client tests passed for stale poll readiness followed by `EAGAIN`, `poll` and `read` interruption retries, and overall monotonic timeout returning `ETIMEDOUT`. These tests exercise userspace retry semantics only, not kernel wait-queue or signal behavior.
- `swctl listen` rejects trailing characters, zero count, missing values, duplicate/unknown options, and timeout overflow before trying to open the device; tests passed with GCC and Clang builds.
- `swctl listen` JSON string writer tests quote/backslash and all JSON control escapes, preserves valid UTF-8, maps invalid/truncated/overlong/surrogate/out-of-range UTF-8 bytes to `U+00xx`, and rejects an invalid output argument. The standalone test ran with GCC, Clang, and Clang ASan/UBSan.
- A separate Clang AddressSanitizer/UndefinedBehaviorSanitizer userspace build and the same CLI/UAPI/listener smoke suite passed.
- Clang static analysis of `userspace/src/swctl.c`: no diagnostics.
- `make module`: passed against Linux `7.1.5+kali-amd64` headers.
- GCC and Clang module builds against Linux `7.1.5+kali-amd64` headers passed. The Clang Kbuild used system GNU `ld`/`objcopy` because `ld.lld` and `llvm-objcopy` are absent. Neither module build loaded the module.
- The compiled module defaults synthetic injection off. The guarded VM harness now checks a default-off rejection, unloads, reloads with explicit injection enablement, checks an event round trip, and unloads. This runtime path was not executed because the host is not a detected VM.
- After correcting blocking `read` to re-check the ring after wakeups, `make test` and `make module` were rerun and passed.
- Runtime environment check: `systemd-detect-virt --vm` reported `none`. The module was not loaded.

After those recorded results, `file_operations.compat_ioctl` was routed through `compat_ptr_ioctl` for `CONFIG_COMPAT`, and `sw_get_stats` began rejecting mismatched ABI versions/nonzero reserved fields with `EPROTO`. These latest code changes have not been built or tested; prior pass records are historical and do not cover them.

Not run: module load/unload, blocking read/poll, injected-event validation, ring overflow, concurrency, KASAN/KCSAN, a second kernel version, or 32-bit-process-to-64-bit-kernel `compat_ioctl` runtime. The 32-bit compile is a layout check, not kernel compatibility proof. Runtime verification requires a disposable Linux VM with a snapshot and recovery console.
