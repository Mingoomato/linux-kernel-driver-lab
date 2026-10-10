# Test plan

- Compile the userspace client with GCC and Clang; validate UAPI field offsets, structure sizes, ioctl encodings, and optional 32-bit layout compilation.
- Mock poll/read sequences to verify stale readiness followed by `EAGAIN`, poll/read `EINTR`, one monotonic overall timeout, and explicit `ETIMEDOUT` without loading the module.
- Confirm the guarded VM script refuses to run without load opt-in, snapshot confirmation, recovery-console confirmation, and a detected VM; never exercise module loading on the development host.
- Reject malformed, missing, duplicate, zero, and overflowing `swctl listen` numeric options before attempting device access.
- Test JSON output for quotes, backslashes, every control byte, valid UTF-8, and malformed/truncated/overlong/surrogate/out-of-range UTF-8 bytes without a device.
- Build the module against the exact running guest headers; never load it on the host.
- In the guarded VM harness, load once with default-off injection and prove rejection, unload, reload with explicit injection enablement, then exercise one event round trip and explicit unload.
- In a disposable VM, exercise default-disabled injection, explicitly enabled injection, malformed kernel-owned sequence/timestamp inputs, ABI version, unknown ioctl, short read buffer, nonblocking empty read, poll wakeup, signal interruption, ring overflow, concurrent readers/writers, and 100 load/unload cycles.
- Repeat on a second kernel; record exact config, headers, dmesg, and kernel version.
- Run KASAN/KCSAN-enabled VM checks where available.

Native/32-bit UAPI compile checks and the no-opt-in loader refusal passed, and compile-only module validation passed against the current headers. The module must not be loaded on this host; runtime tests require a disposable VM with a snapshot and recovery console.
