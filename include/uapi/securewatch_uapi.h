#ifndef SECUREWATCH_UAPI_H
#define SECUREWATCH_UAPI_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define SW_ABI_VERSION 1U
#define SW_MAX_PAYLOAD 64U
#define SW_EVENT_LAB 1U

struct sw_event_v1 {
    __u32 abi_version;
    __u32 event_type;
    __u64 sequence;
    __u64 timestamp_ns;
    __u32 payload_len;
    __u32 flags;
    __u8 payload[SW_MAX_PAYLOAD];
};

struct sw_stats_v1 {
    __u32 abi_version;
    __u32 reserved;
    __u64 enqueued;
    __u64 dequeued;
    __u64 dropped;
    __u64 queued;
};

#define SW_IOC_MAGIC 0xB7
#define SW_IOC_GET_VERSION  _IOR(SW_IOC_MAGIC, 1, __u32)
#define SW_IOC_GET_STATS    _IOR(SW_IOC_MAGIC, 2, struct sw_stats_v1)
#define SW_IOC_CLEAR_STATS  _IO(SW_IOC_MAGIC, 3)

#endif
