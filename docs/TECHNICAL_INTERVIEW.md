# Technical Defense — Linux Kernel Driver Lab

Questions only; no answers are included. Kernel code must be defended with exact lock, lifetime, and user-memory reasoning.

## Code reading and ABI

1. Trace `swctl inject-lab-event` through `write`, `copy_from_user`, `sw_enqueue`, and the ring.
2. Which fields in `sw_event_v1` are assigned by userspace and which are overwritten by the kernel?
3. What do `_IOR` and `_IO` encode in the three ioctl definitions?
4. How does `sw_ioctl` reject unknown magic and unknown command encodings?
5. Why must reserved fields and structure layout remain stable after ABI v1 ships?
6. What do native and `-m32` UAPI layout tests prove, and what remains unproven for compat ioctl runtime?
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
17. Why must injected sequence and timestamp be zero, and where are their final values assigned?
18. What protection does the default-off `allow_lab_injection` parameter add, and why is `CAP_SYS_ADMIN` still checked separately?
19. What does `.owner=THIS_MODULE` protect during `rmmod`?
20. Why is the module loaded only in a disposable VM, and what recovery artifacts are required?
21. What does device mode 0600 guarantee and what does it not guarantee about privileged users?
22. Why should event payloads and secrets never be logged with `pr_info`?

## Design tradeoffs

23. Compare the explicit fixed ring with `kfifo`; what synchronization still remains either way?
24. When would a mutex be simpler than a spinlock, and what execution contexts change that answer?
25. Compare a miscdevice, a dedicated char-device major, and netlink for this synthetic stream.
26. What is the tradeoff between dropping after a user-copy fault and implementing per-reader retry/reservation?
27. Which runtime gate is still outstanding after module compilation succeeds?
28. How does `sw_json_write_string` distinguish valid UTF-8 from malformed bytes, and what exact character does a `U+00xx` byte escape represent to a JSON consumer?
29. Why can `poll` report readability and the following nonblocking `read` still return `EAGAIN` when multiple readers share one queue?
30. Why should `sw_listen` use one `CLOCK_MONOTONIC` deadline across repeated `poll` calls rather than passing the original timeout after every `EINTR`?
31. How does `compat_ptr_ioctl` translate an ioctl argument from a 32-bit process, and which parts of this UAPI make forwarding to `sw_ioctl` safe?
32. Why should the userspace stats client check the returned ABI version and reserved field even after a successful ioctl call?

Owner mastery is not inferred from automated build completion; it requires direct evidence from an independent implementation and technical defense.
