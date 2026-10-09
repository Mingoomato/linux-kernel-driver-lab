# Test plan

- Compile the userspace client with GCC and Clang; validate UAPI field offsets, structure sizes, ioctl encodings, and optional 32-bit layout compilation.
- Confirm the guarded VM script refuses to run without explicit opt-in; never exercise module loading on the development host.
- Build the module against the exact running guest headers; never load it on the host.
- In a disposable VM, exercise default-disabled injection, explicitly enabled injection, malformed kernel-owned sequence/timestamp inputs, ABI version, unknown ioctl, short read buffer, nonblocking empty read, poll wakeup, signal interruption, ring overflow, concurrent readers/writers, and 100 load/unload cycles.
- Repeat on a second kernel; record exact config, headers, dmesg, and kernel version.
- Run KASAN/KCSAN-enabled VM checks where available.

Native/32-bit UAPI compile checks and the no-opt-in loader refusal passed, and compile-only module validation passed against the current headers. The module must not be loaded on this host; runtime tests require a disposable VM with a snapshot and recovery console.
