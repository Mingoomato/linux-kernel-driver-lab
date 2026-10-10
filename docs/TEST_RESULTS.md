# Test results

Status: userspace, UAPI layout, and compile-only module checks passed on the current host.

- Userspace C17 build: passed.
- Userspace C17 build with Clang: passed; GCC build was restored and used for the final `make test` run.
- `make test`: passed; native compile-time assertions verified ABI v1 struct sizes, field offsets, and ioctl encoding; optional 32-bit (`-m32`) layout compilation passed; CLI non-device checks passed; VM loader refused to proceed without explicit opt-in.
- `swctl listen` rejects trailing characters, zero count, missing values, duplicate/unknown options, and timeout overflow before trying to open the device; tests passed with GCC and Clang builds.
- A separate GCC AddressSanitizer/UndefinedBehaviorSanitizer userspace build and the same CLI/UAPI smoke suite passed.
- Clang static analysis of `userspace/src/swctl.c`: no diagnostics.
- `make module`: passed against Linux `7.1.5+kali-amd64` headers.
- The compiled module defaults synthetic injection off; `scripts/vm-test.sh` supplies the explicit load-time enable only after its VM and user opt-in gates. This configuration path was compile-checked but not exercised by loading the module.
- After correcting blocking `read` to re-check the ring after wakeups, `make test` and `make module` were rerun and passed.
- Runtime environment check: `systemd-detect-virt --vm` reported `none`. The module was not loaded.

Not run: module load/unload, blocking read/poll, injected-event validation, ring overflow, concurrency, KASAN/KCSAN, a second kernel version, or 32-bit-process-to-64-bit-kernel `compat_ioctl` runtime. The 32-bit compile is a layout check, not kernel compatibility proof. Runtime verification requires a disposable Linux VM with a snapshot and recovery console.
