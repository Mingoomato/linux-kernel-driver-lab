#include <stddef.h>
#include "securewatch_uapi.h"

_Static_assert(SW_ABI_VERSION == 1U, "ABI version changed");
_Static_assert(sizeof(struct sw_event_v1) == 96U, "event ABI size changed");
_Static_assert(offsetof(struct sw_event_v1, sequence) == 8U, "sequence offset changed");
_Static_assert(offsetof(struct sw_event_v1, timestamp_ns) == 16U, "timestamp offset changed");
_Static_assert(offsetof(struct sw_event_v1, payload_len) == 24U, "payload length offset changed");
_Static_assert(offsetof(struct sw_event_v1, flags) == 28U, "flags offset changed");
_Static_assert(offsetof(struct sw_event_v1, payload) == 32U, "payload offset changed");
_Static_assert(sizeof(struct sw_stats_v1) == 40U, "stats ABI size changed");
_Static_assert(offsetof(struct sw_stats_v1, enqueued) == 8U, "stats enqueued offset changed");
_Static_assert(offsetof(struct sw_stats_v1, queued) == 32U, "stats queued offset changed");
_Static_assert(_IOC_TYPE(SW_IOC_GET_VERSION) == SW_IOC_MAGIC, "ioctl magic changed");
_Static_assert(_IOC_NR(SW_IOC_GET_VERSION) == 1U, "version ioctl number changed");
_Static_assert(_IOC_DIR(SW_IOC_GET_VERSION) == _IOC_READ, "version ioctl direction changed");
_Static_assert(_IOC_SIZE(SW_IOC_GET_VERSION) == sizeof(__u32), "version ioctl size changed");
_Static_assert(_IOC_SIZE(SW_IOC_GET_STATS) == sizeof(struct sw_stats_v1), "stats ioctl size changed");
_Static_assert(_IOC_DIR(SW_IOC_CLEAR_STATS) == _IOC_NONE, "clear ioctl direction changed");

int main(void) { return 0; }
