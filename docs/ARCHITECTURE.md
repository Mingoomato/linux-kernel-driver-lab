# Architecture

The kernel side is a miscdevice at /dev/securewatch. A fixed-size v1 event record passes through a bounded 256-entry ring protected by a spinlock. A wait queue wakes blocking readers, poll/epoll users, and concurrent readers. The ioctl surface is a strict three-command allowlist for ABI version, stats, and privileged counter reset. Synthetic injection uses write and is gated by CAP_SYS_ADMIN.

The file_operations owner field holds a module reference while the device is open, so ordinary module removal waits for active file handles. Module init registers one miscdevice; exit marks shutdown, wakes waiters, and deregisters it.

The user client checks the device type and ABI before use. No file contents or arbitrary strings are sent to kernel logs.
