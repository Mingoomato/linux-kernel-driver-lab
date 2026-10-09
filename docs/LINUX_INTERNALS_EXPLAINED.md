# Linux Internals Explained — Linux Kernel Driver Lab

## Character devices and miscdevice registration

A character device exposes file operations through a device node. The miscdevice framework allocates a dynamic minor and registers `/dev/securewatch` through `misc_register`; `misc_deregister` removes the registration. The `file_operations` callbacks receive kernel-managed `struct file` objects and user-space pointers for buffers.

`.owner = THIS_MODULE` connects open file lifetime to module reference counting. Ordinary unload cannot complete while open file handles still pin the module. The module must not free callback state while a file operation can still access it.

## User memory boundary

Pointers passed to `read`, `write`, and `ioctl` originate in a userspace address space. `copy_from_user` and `copy_to_user` can fail and may fault; they return the number of bytes not copied. The driver checks that result and never dereferences a `__user` pointer directly. User copy operations happen outside the spinlock because fault handling can sleep.

## Ring buffer and synchronization

The ring has fixed capacity and uses head/tail/count to distinguish empty/full state. A spinlock protects all compound updates so two writers cannot reserve the same slot and readers cannot race index changes. `spin_lock_irqsave` also saves/restores local interrupt state, which is a broadly safe pattern when lock users may interact with interrupt context. No sleeping operation belongs inside this critical section.

The wait queue expresses “data available or stopping.” A producer changes state while locked, unlocks, then wakes waiters. A blocking reader must loop: a wake is a hint to re-check, not a reservation of an event. `poll_wait` registers the caller so `poll`/`epoll` can sleep on the same condition. Readiness may change before the subsequent `read`, so the file must still handle `EAGAIN` when nonblocking.

## ABI and ioctl numbering

`_IOR` and `_IO` encode direction, type, command number, and payload size into an ioctl request. A strict exact-command switch rejects unexpected encodings with `ENOTTY`. Fixed-width Linux UAPI types and explicit ABI version fields reduce layout ambiguity, but 32/64-bit compatibility and padding rules still require compile-time tests on both ABIs. Reserved fields should remain zero and must not be repurposed casually.

## Queue full, signals, and copy faults

When the ring is full, the kernel increments `dropped` and returns `ENOSPC`. A blocked `wait_event_interruptible` may return an interruption code when a signal arrives. If the queue record has already been removed and `copy_to_user` fails, this implementation increments the drop count; it does not retry/requeue that event. These are visible delivery semantics, not exactly-once delivery.

## Official references

- [Miscellaneous devices](https://docs.kernel.org/driver-api/misc_devices.html)
- [Wait queues and wake events](https://docs.kernel.org/driver-api/basics.html#wait-queues-and-wake-events)
- [Kernel locking types](https://docs.kernel.org/locking/locktypes.html)
- [Kernel ioctl guidance](https://docs.kernel.org/driver-api/ioctl.html), [ioctl numbers](https://docs.kernel.org/userspace-api/ioctl/ioctl-number.html)
- [Kbuild external modules](https://docs.kernel.org/kbuild/modules.html)
- [`poll(2)`](https://man7.org/linux/man-pages/man2/poll.2.html), [`read(2)`](https://man7.org/linux/man-pages/man2/read.2.html), [`write(2)`](https://man7.org/linux/man-pages/man2/write.2.html)
