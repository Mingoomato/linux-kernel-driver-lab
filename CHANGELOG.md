# Changelog

## 0.1.1 — 2026-10-10

- Hardened `swctl listen` count/timeout parsing and reject invalid arguments before opening the kernel device.
- Added CLI regression cases and ran them with GCC and Clang userspace builds.
- Clang static analysis reported no diagnostics. The kernel module remains compile-only on this non-VM host.

## 0.1.0 — 2026-10-10

- Initial educational implementation based on the associated SRS.
- Added bounded CLI functionality, security and architecture notes, and isolated smoke validation.
- Runtime and enforcement claims remain limited to the evidence recorded in `docs/TEST_RESULTS.md`.
