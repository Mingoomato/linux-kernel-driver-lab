# Engineering Study Guide — Linux Kernel Driver Lab

## Learning objective and implementation status

This repository is a small out-of-tree Linux misc-device lab. It teaches a versioned userspace ABI, kernel/user memory boundaries, bounded queues, blocking and nonblocking reads, poll notification, ioctl validation, synchronization, and module lifecycle. It handles only synthetic records; it has no file-system hooks, process-memory access, or production protection behavior.

GCC and Clang userspace builds, `make test`, module compilation against Linux `7.1.5+kali-amd64` headers, and Clang static analysis passed. The smoke checks native and `-m32` UAPI layout compilation, strict `swctl listen` option parsing before device access, JSON byte encoding, CLI non-device behavior, and refusal by the VM script without explicit opt-in. A separate userspace ASan/UBSan build and smoke run also passed. The module was not loaded because this workspace is not a disposable VM with a recovery console. Blocking `read` re-checks the ring after wakeup so concurrent readers do not return a spurious empty result; injected records require kernel-owned sequence/timestamp fields to be zero. Runtime concurrency, load/unload, poll, overflow, copy fault, KASAN/KCSAN, 32-bit compat dispatch, and second-kernel tests remain unverified. Static 32-bit layout compilation is not a compat runtime claim.

## Why each subsystem exists

| Subsystem | Why it exists | Code |
|---|---|---|
| miscdevice | Registers a small dynamic-minor char device without a custom major/minor allocator. | `sw_device`, `sw_init`, `sw_exit` |
| Fixed UAPI | Gives userspace a stable, versioned record and ioctl contract. | `include/uapi/securewatch_uapi.h` |
| Bounded FIFO | Demonstrates queue overflow and backpressure without unbounded kernel allocation. | `sw_state.ring`, `sw_enqueue`, `sw_read` |
| Lock/wait queue | Protects ring indexes/counters and wakes readers when new events arrive. | `spin_lock_irqsave`, `readable`, `wake_up_interruptible` |
| `read`/`write` operations | Separates synthetic event injection from consuming records while validating user buffers. | `sw_read`, `sw_write` |
| `poll` and ioctl | Supports event-loop clients and a narrow management/statistics surface. | `sw_poll`, `sw_ioctl` |
| Userspace client | Checks device and ABI, strictly parses listen bounds before opening the device, writes bounded lab messages, reads fixed records, and reports stats. | `userspace/src/swctl.c`, `parse_listen_options` |
| JSON byte encoder | Keeps a fixed UAPI payload from breaking the `listen` JSON line on quotes, controls, or malformed UTF-8. | `userspace/src/json_output.h`, `sw_json_write_string` |

## Data and control flow

### Event injection

`swctl inject-lab-event` creates a zero-initialized `sw_event_v1`, fills ABI version/type/payload, and writes exactly one structure. Injection is disabled by default and requires the load-time `allow_lab_injection=1` parameter plus `CAP_SYS_ADMIN`. The kernel checks those gates, exact write size, ABI/type, payload length, flags, zeroed payload tail, and zero user-provided sequence/timestamp with `copy_from_user`. `sw_enqueue` acquires the ring spinlock, rejects shutdown/full state, assigns sequence and real-time timestamp, writes the record, updates counters/indexes, releases the lock, then wakes readers.

### Read/listen

`sw_read` checks the requested user buffer length. Under the lock it either dequeues one structure, returns EOF if stopping, or observes an empty queue. Nonblocking reads return `-EAGAIN`; blocking reads sleep interruptibly on the wait queue and re-check the condition after every wake. This loop matters because multiple readers can be woken for one event and another reader may consume it first. The queue lock is released before `copy_to_user`, because copying to user memory may fault/sleep. A copy fault after dequeue is counted as a drop.

The userspace client strictly parses `--count` and `--timeout-ms` as bounded decimal values before attempting to open a device. It then uses `poll` before `read`, validates the returned fixed-size ABI record, and formats JSON with `sw_json_write_string`: quotes/backslashes and controls are escaped, valid UTF-8 is retained, and each malformed byte is represented by a `U+00xx` escape. The encoder has a device-free standalone test for control bytes, valid/malformed/truncated UTF-8, and invalid arguments. `sw_poll` registers its wait queue with poll/epoll and reports readable data or hangup state.

### Lifecycle and control

`sw_init` zeroes state, initializes lock and wait queue, then calls `misc_register`. The file operations set `.owner = THIS_MODULE`, so an open file pins the module. `sw_exit` marks stopping, wakes waiters, deregisters the device, and emits a generic kernel message. `sw_ioctl` permits only exact version/stats/clear commands; unknown magic/commands return `ENOTTY`.

## Ownership, lifetime, concurrency

- The ring and counters are static module state. No per-event heap allocation is required.
- `copy_from_user`/`copy_to_user` cross the user/kernel memory boundary; kernel pointers must never be derived by direct dereference of user addresses.
- The spinlock protects head/tail/count/counters and event assignment. It is held only for bounded memory operations; user copies, waits, and output must happen outside it.
- Wait queue lifetime follows the static module state. `wake_up_interruptible` occurs after queue state is published. The blocking reader rechecks under the lock to handle competing consumers.
- `.owner=THIS_MODULE` prevents ordinary unload while file descriptors hold the module. The VM harness uses `insmod`/`rmmod` only after a detected-VM and explicit environment guard.
- The ring is bounded at 256 entries. A full enqueue increments `dropped` and returns `-ENOSPC`; it never silently grows memory.

## Security assumptions and failure modes

Userspace supplies untrusted structure bytes. The kernel validates length, ABI fields, and kernel-assigned fields before queueing and uses `copy_*_user`. The device mode is 0600; injection is disabled by default and requires an explicit module parameter and `CAP_SYS_ADMIN`; stats reset requires `CAP_SYS_ADMIN`. Kernel logs contain no payload. Read buffer too small returns `EINVAL`, invalid pointers return `EFAULT`, empty nonblocking read returns `EAGAIN`, interrupted blocking wait returns the signal error, and full ring returns `ENOSPC`.

If `copy_to_user` fails after dequeue, that record cannot be put back safely without a more elaborate reservation protocol; it is counted as dropped. Reader/writer races and unload behavior have not been runtime-tested. UAPI structure packing/compatibility must be checked on 32-bit and 64-bit userspaces before claiming cross-architecture stability.

## Alternatives and tradeoffs

- **mutex** is simpler for process-context-only producers, but the queue may later be fed from contexts that cannot sleep; the current spinlock works for short critical sections but requires strict no-sleep discipline.
- **kfifo** provides reusable ring helpers and may simplify indexing, but explicit indexes make this small ABI lab easier to audit. Correct lock strategy is still required.
- **blocking read only** is simpler but less composable; `poll` allows integration with an event loop. `epoll` scales to many descriptors but does not change queue semantics.
- **netlink/generic netlink** may be more suitable for extensible event/control protocols, but it adds message framing and family registration complexity.
- **ioctl for event injection** could be fixed-size but is less naturally stream-like; `write` demonstrates partial/length validation for a single exact record.

## Study route

Start with the UAPI header and `tests/uapi_layout.c`, then read `sw_write` → `sw_enqueue`, `sw_read`, `sw_poll`, `sw_ioctl`, and module init/exit. Draw the lock/unlock points and mark every operation that can fault or sleep. Review `scripts/vm-test.sh` before any privileged runtime work; do not remove its isolation gates.

## Official references

- [Linux misc devices](https://docs.kernel.org/driver-api/misc_devices.html)
- [Linux driver wait queues](https://docs.kernel.org/driver-api/basics.html#wait-queues-and-wake-events)
- [Linux locking documentation](https://docs.kernel.org/locking/locktypes.html)
- [Linux ioctl interface guidance](https://docs.kernel.org/driver-api/ioctl.html) and [ioctl number conventions](https://docs.kernel.org/userspace-api/ioctl/ioctl-number.html)
- [Building external modules with Kbuild](https://docs.kernel.org/kbuild/modules.html)
- [`poll(2)`](https://man7.org/linux/man-pages/man2/poll.2.html), [`read(2)`](https://man7.org/linux/man-pages/man2/read.2.html), [`write(2)`](https://man7.org/linux/man-pages/man2/write.2.html)
