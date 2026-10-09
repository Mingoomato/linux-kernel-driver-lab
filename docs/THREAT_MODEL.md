# Threat model

## Trust boundary

User memory is untrusted. The driver checks record size and ABI fields, uses copy_from_user/copy_to_user, validates payload length, rejects unknown ioctl commands, and caps queue storage at compile time.

## Access and lifecycle

The device mode is 0600. Synthetic event writes and counter reset require CAP_SYS_ADMIN. The module does not load on behalf of the CLI. Kernel build/load/unload experiments require a disposable VM because a bug can panic the guest.

## Residual risks

The driver is educational and not self-protecting from privileged users. If copy_to_user fails after dequeue, that event is counted as dropped. Kernel API compatibility and runtime behavior have not been verified against any particular kernel version. No KASAN/KCSAN or concurrency result is claimed.
