# UAPI versioning

ABI version 1 uses fixed-width Linux UAPI integer types. Reserved fields and flags must be zero. The ioctl magic is 0xB7; unknown commands return ENOTTY. Event records have a fixed size and contain a 64-byte payload. For injected records, userspace must set `sequence` and `timestamp_ns` to zero; the kernel rejects nonzero values and assigns both fields when the record is enqueued.

The injection path is off by default. The isolated-VM harness explicitly enables it with the read-only load-time parameter `allow_lab_injection=1`, and the caller must still have `CAP_SYS_ADMIN`.

Breaking changes require a new ABI version and a new structure/command number. Do not change field order or structure size in place. Consumers must reject unexpected versions and payload lengths.

`tests/uapi_layout.c` checks field offsets, structure sizes, and ioctl encodings with native and optional `-m32` compilation. The file operations now route `CONFIG_COMPAT` ioctl calls through `compat_ptr_ioctl`, which is suitable for this fixed-width, pointer-free UAPI. The layout compile is still not a 32-bit process against a loaded 64-bit kernel and does not prove `compat_ioctl` dispatch or multiple kernel releases. The userspace stats helper rejects a reply with an unexpected ABI version or nonzero reserved field.
