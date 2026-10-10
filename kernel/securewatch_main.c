#include <linux/capability.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/ktime.h>
#include "securewatch_uapi.h"

#define SW_RING_CAPACITY 256U

struct sw_state {
    spinlock_t lock;
    wait_queue_head_t readable;
    struct sw_event_v1 ring[SW_RING_CAPACITY];
    unsigned int head;
    unsigned int tail;
    unsigned int count;
    bool stopping;
    u64 next_sequence;
    u64 enqueued;
    u64 dequeued;
    u64 dropped;
};

static struct sw_state sw;
static bool allow_lab_injection;
module_param(allow_lab_injection, bool, 0400);
MODULE_PARM_DESC(allow_lab_injection, "enable synthetic lab event writes (set only in an isolated VM)");

static bool queue_has_data(void)
{
    return READ_ONCE(sw.count) != 0 || READ_ONCE(sw.stopping);
}

static int sw_enqueue(struct sw_event_v1 *event)
{
    unsigned long flags;
    spin_lock_irqsave(&sw.lock, flags);
    if (sw.stopping) {
        spin_unlock_irqrestore(&sw.lock, flags);
        return -ESHUTDOWN;
    }
    if (sw.count == SW_RING_CAPACITY) {
        sw.dropped++;
        spin_unlock_irqrestore(&sw.lock, flags);
        return -ENOSPC;
    }
    event->sequence = ++sw.next_sequence;
    event->timestamp_ns = ktime_get_real_ns();
    sw.ring[sw.tail] = *event;
    sw.tail = (sw.tail + 1U) % SW_RING_CAPACITY;
    sw.count++;
    sw.enqueued++;
    spin_unlock_irqrestore(&sw.lock, flags);
    wake_up_interruptible(&sw.readable);
    return 0;
}

static ssize_t sw_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
    struct sw_event_v1 event;
    unsigned long flags;
    int rc;
    (void)offset;
    if (length < sizeof(event))
        return -EINVAL;
    for (;;) {
        spin_lock_irqsave(&sw.lock, flags);
        if (sw.count) {
            event = sw.ring[sw.head];
            memset(&sw.ring[sw.head], 0, sizeof(sw.ring[sw.head]));
            sw.head = (sw.head + 1U) % SW_RING_CAPACITY;
            sw.count--;
            sw.dequeued++;
            spin_unlock_irqrestore(&sw.lock, flags);
            break;
        }
        if (sw.stopping) {
            spin_unlock_irqrestore(&sw.lock, flags);
            return 0;
        }
        spin_unlock_irqrestore(&sw.lock, flags);
        if (file->f_flags & O_NONBLOCK)
            return -EAGAIN;
        rc = wait_event_interruptible(sw.readable, queue_has_data());
        if (rc)
            return rc;
    }
    if (copy_to_user(buffer, &event, sizeof(event)) != 0) {
        unsigned long restore_flags;
        spin_lock_irqsave(&sw.lock, restore_flags);
        sw.dropped++;
        spin_unlock_irqrestore(&sw.lock, restore_flags);
        return -EFAULT;
    }
    return sizeof(event);
}

static ssize_t sw_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
    struct sw_event_v1 event;
    int rc;
    (void)file;
    (void)offset;
    if (!allow_lab_injection)
        return -EOPNOTSUPP;
    if (!capable(CAP_SYS_ADMIN))
        return -EACCES;
    if (length != sizeof(event))
        return -EINVAL;
    if (copy_from_user(&event, buffer, sizeof(event)) != 0)
        return -EFAULT;
    if (event.abi_version != SW_ABI_VERSION || event.event_type != SW_EVENT_LAB ||
        event.payload_len > SW_MAX_PAYLOAD || event.flags != 0 ||
        event.sequence != 0 || event.timestamp_ns != 0)
        return -EINVAL;
    if (memchr_inv(event.payload + event.payload_len, 0, SW_MAX_PAYLOAD - event.payload_len))
        return -EINVAL;
    rc = sw_enqueue(&event);
    if (rc != 0)
        return rc;
    return sizeof(event);
}

static __poll_t sw_poll(struct file *file, poll_table *wait)
{
    __poll_t mask = 0;
    (void)file;
    poll_wait(file, &sw.readable, wait);
    if (READ_ONCE(sw.count) != 0)
        mask |= EPOLLIN | EPOLLRDNORM;
    if (READ_ONCE(sw.stopping))
        mask |= EPOLLHUP;
    return mask;
}

static long sw_ioctl(struct file *file, unsigned int command, unsigned long argument)
{
    unsigned long flags;
    (void)file;
    if (_IOC_TYPE(command) != SW_IOC_MAGIC)
        return -ENOTTY;
    switch (command) {
    case SW_IOC_GET_VERSION: {
        __u32 version = SW_ABI_VERSION;
        return copy_to_user((void __user *)argument, &version, sizeof(version)) ? -EFAULT : 0;
    }
    case SW_IOC_GET_STATS: {
        struct sw_stats_v1 stats = { .abi_version = SW_ABI_VERSION };
        spin_lock_irqsave(&sw.lock, flags);
        stats.enqueued = sw.enqueued;
        stats.dequeued = sw.dequeued;
        stats.dropped = sw.dropped;
        stats.queued = sw.count;
        spin_unlock_irqrestore(&sw.lock, flags);
        return copy_to_user((void __user *)argument, &stats, sizeof(stats)) ? -EFAULT : 0;
    }
    case SW_IOC_CLEAR_STATS:
        if (!capable(CAP_SYS_ADMIN))
            return -EACCES;
        spin_lock_irqsave(&sw.lock, flags);
        sw.enqueued = 0;
        sw.dequeued = 0;
        sw.dropped = 0;
        spin_unlock_irqrestore(&sw.lock, flags);
        return 0;
    default:
        return -ENOTTY;
    }
}

static const struct file_operations sw_fops = {
    .owner = THIS_MODULE,
    .read = sw_read,
    .write = sw_write,
    .poll = sw_poll,
    .unlocked_ioctl = sw_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = compat_ptr_ioctl,
#endif
    .llseek = noop_llseek,
};

static struct miscdevice sw_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "securewatch",
    .fops = &sw_fops,
    .mode = 0600,
};

static int __init sw_init(void)
{
    int rc;
    memset(&sw, 0, sizeof(sw));
    spin_lock_init(&sw.lock);
    init_waitqueue_head(&sw.readable);
    rc = misc_register(&sw_device);
    if (rc)
        return rc;
    pr_info("securewatch: lab device registered, ABI %u\n", SW_ABI_VERSION);
    return 0;
}

static void __exit sw_exit(void)
{
    unsigned long flags;
    spin_lock_irqsave(&sw.lock, flags);
    sw.stopping = true;
    spin_unlock_irqrestore(&sw.lock, flags);
    wake_up_interruptible_all(&sw.readable);
    misc_deregister(&sw_device);
    pr_info("securewatch: lab device removed\n");
}

module_init(sw_init);
module_exit(sw_exit);

MODULE_DESCRIPTION("Synthetic event character device for an isolated Linux kernel lab");
MODULE_AUTHOR("LinuxSecurity project");
MODULE_LICENSE("GPL");
