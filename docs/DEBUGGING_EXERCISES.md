# Debugging and Independent Reproduction Exercises

These exercises are unsolved. There is no patch or full solution in this repository. Run module lifecycle experiments only inside a disposable VM with a snapshot and recovery console, using the guarded VM script.

## Exercise 1 — Two blocking readers, one event

**Goal:** Reproduce the wakeup race that can occur when several readers wait for a single queue item.

**Starting points:** `sw_read`, `queue_has_data`, `sw_enqueue`, wait queue helpers, and `scripts/vm-test.sh`.

In a VM, start two blocking readers, inject one event, then inject a second event after a controlled delay. Record each read result and verify that the blocking reader with no event continues waiting instead of returning an empty-queue error. Add a repeatable multi-reader test.

**Constraints:** Keep the ring lock out of waits and user copies. Do not hold a kernel spinlock while sleeping. Preserve signal interruption semantics.

**Acceptance:** Each injected event is delivered at most once; an empty blocking read remains blocked until data, shutdown, or signal; a nonblocking empty read returns `EAGAIN`.

## Exercise 2 — User-copy fault and event loss accounting

**Goal:** Exercise `copy_to_user` failure after dequeue and defend the loss policy.

**Starting points:** `sw_read`, `sw_stats_v1`, `SW_IOC_GET_STATS`, and `copy_to_user` handling.

Use a VM-safe test method to supply an invalid userspace buffer to the read path. Check the returned errno, queue state, and `dropped` counter. Decide whether the current drop-on-copy-fault behavior is acceptable or whether a per-reader reservation/commit model is needed.

**Constraints:** Do not directly dereference user memory. Never move `copy_to_user` under the spinlock. Do not run fault injection on the host kernel.

**Acceptance:** Kernel state remains coherent after a failed copy, and the documented delivery guarantee matches observed behavior.

## Exercise 3 — Queue overflow and poll progress

**Goal:** Verify bounded FIFO overflow, statistics, wakeups, and forward progress.

**Starting points:** `SW_RING_CAPACITY`, `sw_write`, `sw_enqueue`, `sw_read`, `sw_poll`, `sw_ioctl`, and UAPI tests.

In the guarded VM, enqueue more records than capacity without a reader, capture expected `ENOSPC` and `dropped`, then start readers and test that a poller wakes and the queue drains. Include malformed records with a nonzero sequence or timestamp and confirm the UAPI rule. Repeat with multiple writers/readers and document ordering guarantees.

**Constraints:** Use only synthetic short messages. Always allow the VM test trap to unload the module. Keep the no-VM compile and userspace tests separate from privileged runtime results.

**Acceptance:** Counts reconcile with successful enqueues, dequeues, and drops; no duplicate/corrupt records appear; readers make progress after the queue is drained.

## Owner attempt record

Record the chosen exercise, VM kernel/config, exact commands, kernel log findings, and unresolved questions. Owner reproduction status is **not yet evidenced**; a passing compile is not runtime proof or personal mastery.
