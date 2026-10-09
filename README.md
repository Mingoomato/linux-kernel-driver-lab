# Linux Kernel Driver Lab

This project contains a versioned UAPI, an out-of-tree misc character device, and a C17 client for synthetic lab events. The module has no filesystem hooks, process-memory access, hidden behavior, or production protection claims.

## Build userspace

    make userspace

`make test` compiles the userspace client and statically checks the UAPI layout. `make module` compiles the module against the running kernel's headers without loading it.

## Build the module (compile only)

    make module

This requires headers matching the running kernel. Building does not load the module. Never run insmod/rmmod on a shared host; privileged lifecycle work belongs in a disposable Linux VM with a snapshot and recovery console.

## VM-only exercise

    sudo insmod kernel/securewatch.ko
    sudo ./userspace/swctl doctor
    sudo ./userspace/swctl inject-lab-event --message hello
    ./userspace/swctl listen --count 1 --timeout-ms 1000
    ./userspace/swctl stats
    sudo rmmod securewatch

The device defaults to mode 0600. Injection and counter clearing require CAP_SYS_ADMIN. A bounded 256-record ring reports drops. The read interface returns one fixed-size event per successful read.

The current host is not detected as a VM, so the module was compile-checked but not loaded. Module runtime tests are deliberately not run unless a disposable VM with a snapshot and recovery console is available. See docs/UAPI_VERSIONING.md and docs/TEST_RESULTS.md.
