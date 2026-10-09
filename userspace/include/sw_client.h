#ifndef SW_CLIENT_H
#define SW_CLIENT_H
#include <stdint.h>
#include "securewatch_uapi.h"
int sw_open_device(const char *path, int flags);
int sw_get_stats(int fd, struct sw_stats_v1 *stats);
int sw_listen(int fd, unsigned int count, int timeout_ms);
int sw_inject(int fd, const char *message);
#endif
