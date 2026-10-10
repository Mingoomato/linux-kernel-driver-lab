# Changelog

## Unreleased

- Added `CONFIG_COMPAT` ioctl pointer routing and userspace stats ABI/reserved-field validation. This latest source change is not built or tested yet.
- `swctl listen` now retries stale-readiness `EAGAIN` and `EINTR` against one `CLOCK_MONOTONIC` deadline; timeout is reported as `ETIMEDOUT`.
- Added mocked poll/read tests for retry and deadline behavior, plus explicit VM snapshot and recovery-console confirmations in the module-load harness.
- Expanded the VM harness to prove default-off injection rejection before an opt-in event round trip; this privileged flow remains unrun on the non-VM host.
- GCC, Clang, and Clang ASan/UBSan user-space smoke, GCC/Clang module compile, UAPI layout checks, and Clang static analysis passed.
- Added a bounded JSON string encoder for `swctl listen`; valid UTF-8 is retained and invalid UTF-8 bytes map to U+00xx JSON escapes.
- Added device-free byte-level tests for control characters, UTF-8 edge cases, and invalid encoder arguments; GCC, Clang, and Clang ASan/UBSan smoke passed.

## 0.1.1 — 2026-10-10

- Hardened `swctl listen` count/timeout parsing and reject invalid arguments before opening the kernel device.
- Added CLI regression cases and ran them with GCC and Clang userspace builds.
- Clang static analysis reported no diagnostics. The kernel module remains compile-only on this non-VM host.

## 0.1.0 — 2026-10-10

- Initial educational implementation based on the associated SRS.
- Added bounded CLI functionality, security and architecture notes, and isolated smoke validation.
- Runtime and enforcement claims remain limited to the evidence recorded in `docs/TEST_RESULTS.md`.
