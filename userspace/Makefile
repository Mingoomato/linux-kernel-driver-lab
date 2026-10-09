CC ?= cc
CFLAGS ?= -O2 -std=c17 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS += -Iinclude -I../include/uapi

all: swctl

swctl: src/swctl.c ../include/uapi/securewatch_uapi.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ src/swctl.c

clean:
	rm -f swctl

.PHONY: all clean
