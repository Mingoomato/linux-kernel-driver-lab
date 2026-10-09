# Requirements traceability (SRS-05)

Compile and UAPI layout checks passed. The module was not loaded and no privileged runtime test was run. See `docs/TEST_RESULTS.md`.

| Requirement | Source / artifact | Verification | Status |
|---|---|---|---|
| KDL-FR-001 | kernel/Makefile, top Makefile | docs/TEST_RESULTS.md (Linux 7.1.5+kali-amd64) | IN_PROGRESS |
| KDL-FR-002 | module init/exit | KDL-T-002 VM pending | IN_PROGRESS |
| KDL-FR-003 | miscdevice registration | KDL-T-002 VM pending | IMPLEMENTED |
| KDL-FR-004 | include/uapi/securewatch_uapi.h | tests/uapi_layout.c | IMPLEMENTED |
| KDL-FR-005 | sw_ioctl | KDL-T-003/004 pending | IMPLEMENTED |
| KDL-FR-006 | sw_write, CAP_SYS_ADMIN | KDL-T-013/015 pending | IMPLEMENTED |
| KDL-FR-007 | sw_read | KDL-T-005/006 pending | IMPLEMENTED |
| KDL-FR-008 | sw_read, sw_poll | KDL-T-007/008 pending | IMPLEMENTED |
| KDL-FR-009 | fixed ring + dropped counter | KDL-T-009 pending | IMPLEMENTED |
| KDL-FR-010 | spinlock-protected ring | KDL-T-010 VM pending | IN_PROGRESS |
| KDL-FR-011 | fixed UAPI fields and validation | ABI tests pending | IN_PROGRESS |
| KDL-FR-012 | exact ioctl allowlist | KDL-T-004 pending | IMPLEMENTED |
| KDL-FR-013 | copy_*_user only | KDL-T-014 pending | IMPLEMENTED |
| KDL-FR-014 | owner reference, shutdown/wakeup | KDL-T-011 VM pending | IN_PROGRESS |
| KDL-FR-015 | no payload printk | source review pending | IMPLEMENTED |
| KDL-FR-016 | mode 0600 | permission test pending | IMPLEMENTED |
| KDL-FR-017 | docs and VM-only guard | VM harness review pending | IN_PROGRESS |
| KDL-FR-018 | swctl CLI | userspace build pending | IN_PROGRESS |
| KDL-FR-019 | docs/compatibility-matrix.md | no matrix entries yet | NOT_STARTED |
| KDL-FR-020 | — | KASAN/KCSAN not run | NOT_STARTED |
| KDL-FR-021 | — | optional | NOT_APPLICABLE |
| KDL-FR-022 | — | optional | NOT_APPLICABLE |
| KDL-NFR-01..07 | docs/TEST_PLAN.md | not run | NOT_STARTED |
