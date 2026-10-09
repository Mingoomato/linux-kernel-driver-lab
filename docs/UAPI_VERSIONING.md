# UAPI versioning

ABI version 1 uses fixed-width Linux UAPI integer types. Reserved fields and flags must be zero. The ioctl magic is 0xB7; unknown commands return ENOTTY. Event records have a fixed size and contain a 64-byte payload. For injected records, userspace must set `sequence` and `timestamp_ns` to zero; the kernel rejects nonzero values and assigns both fields when the record is enqueued.

The injection path is off by default. The isolated-VM harness explicitly enables it with the read-only load-time parameter `allow_lab_injection=1`, and the caller must still have `CAP_SYS_ADMIN`.

Breaking changes require a new ABI version and a new structure/command number. Do not change field order or structure size in place. Consumers must reject unexpected versions and payload lengths.

`tests/uapi_layout.c` checks field offsets, structure sizes, and ioctl encodings with native and optional `-m32` compilation. This verifies compile-time structure layout only; it does not exercise a 32-bit process against a loaded 64-bit kernel, test `compat_ioctl` dispatch, or validate multiple kernel releases.
