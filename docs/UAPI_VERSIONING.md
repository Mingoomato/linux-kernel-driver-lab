# UAPI versioning

ABI version 1 uses fixed-width Linux UAPI integer types. Reserved fields and flags must be zero. The ioctl magic is 0xB7; unknown commands return ENOTTY. Event records have a fixed size and contain a 64-byte payload. Sequence and real-time nanosecond timestamp are assigned by the kernel.

Breaking changes require a new ABI version and a new structure/command number. Do not change field order or structure size in place. Consumers must reject unexpected versions and payload lengths.

The current ABI has not been checked across 32-bit/64-bit userspace, compat_ioctl, or multiple kernel releases.
