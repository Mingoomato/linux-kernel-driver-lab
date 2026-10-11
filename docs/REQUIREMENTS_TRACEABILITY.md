# Requirements traceability (SRS-05)

Latest scoped evidence: [2026-10-11 validation checkpoint](VALIDATION_CHECKPOINT.md). Historical pending/unverified rows below remain applicable to gates not exercised by that checkpoint.

Earlier compile and UAPI layout checks passed. The latest source adds compat ioctl routing and stats response validation but was not rebuilt or tested. The module was not loaded and no privileged runtime test was run. See `docs/TEST_RESULTS.md`.

| Requirement | Source / artifact | Verification | Status |
|---|---|---|---|
| KDL-FR-001 | kernel/Makefile, top Makefile | docs/TEST_RESULTS.md (Linux 7.1.5+kali-amd64) | IN_PROGRESS |
| KDL-FR-002 | module init/exit | KDL-T-002 VM pending | IN_PROGRESS |
| KDL-FR-003 | miscdevice registration | KDL-T-002 VM pending | IMPLEMENTED |
| KDL-FR-004 | include/uapi/securewatch_uapi.h | tests/uapi_layout.c | IMPLEMENTED |
| KDL-FR-005 | sw_ioctl; `compat_ptr_ioctl` when CONFIG_COMPAT | Prior native ABI behavior implemented; new compat dispatch unbuilt and 32-to-64 runtime pending | IN_PROGRESS |
| KDL-FR-006 | default-off `allow_lab_injection` module parameter, `sw_write`, CAP_SYS_ADMIN | VM harness covers default-off rejection then opted-in event round trip; not run here, KDL-T-013/015 runtime checks pending | IMPLEMENTED |
| KDL-FR-007 | sw_read | KDL-T-005/006 pending | IMPLEMENTED |
| KDL-FR-008 | sw_read, sw_poll | KDL-T-007/008 kernel wait/signal runtime pending; userspace stale-readiness `EAGAIN` and `EINTR` retry unit test passed | IMPLEMENTED |
| KDL-FR-009 | fixed ring + dropped counter | KDL-T-009 pending | IMPLEMENTED |
| KDL-FR-010 | spinlock-protected ring | KDL-T-010 VM pending | IN_PROGRESS |
| KDL-FR-011 | fixed UAPI fields and validation; kernel-owned sequence/timestamp inputs must be zero | native and `-m32` layout checks; runtime validation pending | IN_PROGRESS |
| KDL-FR-012 | exact ioctl allowlist | Exact command switch; malformed encoding runtime pending; latest source not rebuilt | IN_PROGRESS |
| KDL-FR-013 | copy_*_user only | KDL-T-014 pending | IMPLEMENTED |
| KDL-FR-014 | owner reference, shutdown/wakeup | KDL-T-011 VM pending | IN_PROGRESS |
| KDL-FR-015 | no payload printk | source review pending | IMPLEMENTED |
| KDL-FR-016 | mode 0600 | permission test pending | IMPLEMENTED |
| KDL-FR-017 | docs and VM-only guard | smoke verifies default refusal and missing snapshot/recovery-console confirmations; VM runtime pending | IN_PROGRESS |
| KDL-FR-018 | swctl CLI including strict listen argument validation and valid JSON event output | GCC/Clang/ASan `make test`, native and optional 32-bit UAPI compile, no-device CLI, JSON byte-encoding and mocked poll/read retry/deadline tests | IMPLEMENTED; device runtime pending |
| KDL-FR-019 | docs/compatibility-matrix.md | current kernel build + native/32-bit layout compile; second kernel pending | IN_PROGRESS |
| KDL-FR-020 | — | KASAN/KCSAN not run | NOT_STARTED |
| KDL-FR-021 | — | optional | NOT_APPLICABLE |
| KDL-FR-022 | — | optional | NOT_APPLICABLE |
| KDL-NFR-01..07 | docs/TEST_PLAN.md | not run | NOT_STARTED |
