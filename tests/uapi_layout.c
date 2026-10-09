#include <stddef.h>
#include "securewatch_uapi.h"

_Static_assert(SW_ABI_VERSION == 1U, "ABI version changed");
_Static_assert(sizeof(struct sw_event_v1) == 96U, "event ABI size changed");
_Static_assert(offsetof(struct sw_event_v1, sequence) == 8U, "sequence offset changed");
_Static_assert(offsetof(struct sw_event_v1, payload) == 32U, "payload offset changed");
_Static_assert(sizeof(struct sw_stats_v1) == 40U, "stats ABI size changed");
_Static_assert(_IOC_SIZE(SW_IOC_GET_STATS) == sizeof(struct sw_stats_v1), "stats ioctl size changed");

int main(void) { return 0; }
