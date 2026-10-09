# Test plan

- Compile the userspace client with GCC and Clang and validate UAPI sizes/offsets.
- Build the module against the exact running guest headers; never load it on the host.
- In a disposable VM, exercise ABI version, unknown ioctl, short read buffer, nonblocking empty read, poll wakeup, signal interruption, ring overflow, concurrent readers/writers, and 100 load/unload cycles.
- Repeat on a second kernel; record exact config, headers, dmesg, and kernel version.
- Run KASAN/KCSAN-enabled VM checks where available.

Compile-only module validation passed against the current headers. The module must not be loaded on this host; runtime tests require a disposable VM with a snapshot and recovery console.
