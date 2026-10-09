KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: userspace

userspace:
	$(MAKE) -C userspace

module:
	@test -d "$(KDIR)" || { echo "kernel headers missing: $(KDIR)"; exit 2; }
	$(MAKE) -C "$(KDIR)" M="$(PWD)/kernel" modules

test:
	./tests/smoke.sh

clean:
	$(MAKE) -C userspace clean
	@if test -d "$(KDIR)"; then $(MAKE) -C "$(KDIR)" M="$(PWD)/kernel" clean; fi

.PHONY: all userspace module test clean
