# Code Walkthrough — Linux Kernel Driver Lab

## 1. UAPI first

`include/uapi/securewatch_uapi.h` defines ABI version 1, fixed-width integer fields, a 64-byte payload, stats structure, ioctl magic, and three exact commands. `tests/uapi_layout.c` compiles static assertions about field offsets, structure layout, and ioctl encodings for native and optional `-m32` targets. This checks structure layout, not a 32-bit process talking to a loaded 64-bit kernel. Field order and sizes are part of the ABI; do not edit them in place without a versioned compatibility decision.

## 2. Module state and registration

`kernel/securewatch_main.c` has one static `sw_state`: spinlock, wait queue, 256-element event ring, indexes, count, stop flag, and counters. `sw_init` initializes state, registers `sw_device` as a dynamic-minor miscdevice, and logs a generic message. `sw_exit` sets stopping under the lock, wakes waiters, deregisters the device, and does not print payloads.

`sw_fops.owner = THIS_MODULE` pins the module while a device FD is open. The device defaults to mode 0600.

## 3. Injection path

Userspace `sw_inject` zeroes a structure and writes exactly one full record. Kernel `sw_write` first requires the load-time `allow_lab_injection=1` parameter (default off), then validates capability and size before copying. It uses `copy_from_user`, checks ABI version, event type, payload length, flags, zero sequence/timestamp inputs, and that unused payload bytes are zero. `sw_enqueue` takes the spinlock, checks shutdown/capacity, sets sequence and timestamp, copies to the tail slot, updates counters/indexes, unlocks, and wakes the wait queue.

## 4. Read path and the concurrent-reader fix

`sw_read` rejects a short buffer immediately. It repeatedly checks the queue under the spinlock; if empty and nonblocking it returns `EAGAIN`, if stopping it returns EOF, otherwise it waits interruptibly and loops to re-check. Rechecking is required because a competing reader may consume an item after wakeup. The selected record is copied to a local stack object, its ring slot is zeroed, indexes/counters are updated, and the lock is released before `copy_to_user`.

If `copy_to_user` fails, the dequeued event is counted as dropped and the call returns `EFAULT`. Reinserting it could reorder concurrent consumers; a stronger at-least-once guarantee would require an explicit per-reader reservation/commit design.

## 5. poll and ioctl

`sw_poll` calls `poll_wait` on the same wait queue and reports `EPOLLIN` when data exists or `EPOLLHUP` on shutdown. `sw_ioctl` checks the ioctl magic and switches on exact command encodings. Version/stats copy fixed-size structures to userspace; clearing counters requires `CAP_SYS_ADMIN`; all other commands return `ENOTTY`.

## 6. Userspace client and VM harness

`swctl` opens with `O_CLOEXEC | O_NOFOLLOW`. `doctor` verifies that the path is a character device, opens it nonblocking, queries ABI version, and closes it. `listen` polls and reads a fixed-size event; `inject-lab-event` writes a synthetic record. `tests/smoke.sh` compiles the client and UAPI layout but never opens a live kernel device.

`scripts/vm-test.sh` has two explicit gates: `SECUREWATCH_ALLOW_LOAD=YES` and a positive VM detection result. It loads the module, runs smoke commands, and has an unload trap. This script has not been executed in the current non-VM environment.
