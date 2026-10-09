# Technical Defense — Linux Kernel Driver Lab

Questions only; no answers are included. Kernel code must be defended with exact lock, lifetime, and user-memory reasoning.

## Code reading and ABI

1. Trace `swctl inject-lab-event` through `write`, `copy_from_user`, `sw_enqueue`, and the ring.
2. Which fields in `sw_event_v1` are assigned by userspace and which are overwritten by the kernel?
3. What do `_IOR` and `_IO` encode in the three ioctl definitions?
4. How does `sw_ioctl` reject unknown magic and unknown command encodings?
5. Why must reserved fields and structure layout remain stable after ABI v1 ships?
6. What does the UAPI compile-time layout test prove, and what does it not prove across 32/64-bit ABIs?
7. Trace `swctl listen` through `poll`, `read`, event validation, and JSON output.

## Concurrency and debugging

8. Which ring fields are protected by `sw.lock`? Which operations must remain outside the spinlock?
9. Why must a blocking reader loop and re-check the queue after a wakeup?
10. How does `wait_event_interruptible` report a signal, and what does userspace observe?
11. What happens when a writer finds the ring full? Which counters and errno change?
12. If `copy_to_user` fails after dequeue, what happens to the event and why?
13. How do `poll_wait` and `EPOLLIN` interact? Can readiness guarantee the following read will succeed?
14. How would you test four readers and four writers without racing the test harness itself?

## Security and lifecycle

15. Why is direct dereference of a user pointer forbidden, even after a length check?
16. Which checks in `sw_write` reject malformed structures and payload padding?
17. What does `.owner=THIS_MODULE` protect during `rmmod`?
18. Why is the module loaded only in a disposable VM, and what recovery artifacts are required?
19. What does device mode 0600 guarantee and what does it not guarantee about privileged users?
20. Why should event payloads and secrets never be logged with `pr_info`?

## Design tradeoffs

21. Compare the explicit fixed ring with `kfifo`; what synchronization still remains either way?
22. When would a mutex be simpler than a spinlock, and what execution contexts change that answer?
23. Compare a miscdevice, a dedicated char-device major, and netlink for this synthetic stream.
24. What is the tradeoff between dropping after a user-copy fault and implementing per-reader retry/reservation?
25. Which runtime gate is still outstanding after module compilation succeeds?

Owner mastery is not inferred from automated build completion; it requires direct evidence from an independent implementation and technical defense.
