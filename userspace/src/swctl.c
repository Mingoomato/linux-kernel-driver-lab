#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "sw_client.h"

int sw_open_device(const char *path, int flags) { return open(path, flags | O_CLOEXEC | O_NOFOLLOW); }

int sw_get_stats(int fd, struct sw_stats_v1 *stats) {
    memset(stats, 0, sizeof(*stats));
    return ioctl(fd, SW_IOC_GET_STATS, stats);
}

int sw_listen(int fd, unsigned int count, int timeout_ms) {
    unsigned int seen = 0;
    while (seen < count) {
        struct pollfd pfd = { .fd = fd, .events = POLLIN };
        int rc = poll(&pfd, 1, timeout_ms);
        if (rc < 0 && errno == EINTR) continue;
        if (rc <= 0) return rc == 0 ? 0 : -1;
        struct sw_event_v1 event;
        ssize_t n = read(fd, &event, sizeof(event));
        if (n < 0 && errno == EINTR) continue;
        if (n != (ssize_t)sizeof(event)) return -1;
        if (event.abi_version != SW_ABI_VERSION || event.payload_len > SW_MAX_PAYLOAD) {
            errno = EPROTO;
            return -1;
        }
        printf("{\"sequence\":%llu,\"event_type\":%u,\"timestamp_ns\":%llu,\"message\":\"",
               (unsigned long long)event.sequence, event.event_type,
               (unsigned long long)event.timestamp_ns);
        for (uint32_t i = 0; i < event.payload_len; ++i) {
            unsigned char c = event.payload[i];
            if (c == '"' || c == '\\') putchar('\\');
            if (c >= 0x20U) putchar(c);
        }
        puts("\"}");
        ++seen;
    }
    return 0;
}

int sw_inject(int fd, const char *message) {
    struct sw_event_v1 event;
    memset(&event, 0, sizeof(event));
    event.abi_version = SW_ABI_VERSION;
    event.event_type = SW_EVENT_LAB;
    size_t n = strlen(message);
    if (n > SW_MAX_PAYLOAD) { errno = EMSGSIZE; return -1; }
    event.payload_len = (uint32_t)n;
    memcpy(event.payload, message, n);
    ssize_t written = write(fd, &event, sizeof(event));
    return written == (ssize_t)sizeof(event) ? 0 : -1;
}

static void usage(void) {
    puts("swctl doctor|version|stats|listen --count N [--timeout-ms N]|inject-lab-event --message TEXT|unload-check [--device PATH]");
}

static const char *arg(int argc, char **argv, const char *name, const char *fallback) {
    for (int i = 2; i + 1 < argc; ++i) if (!strcmp(argv[i], name)) return argv[i + 1];
    return fallback;
}

static int parse_decimal(const char *text, unsigned long maximum, unsigned long *value) {
    unsigned long parsed = 0;
    if (text == NULL || *text == '\0') return -1;
    for (const unsigned char *cursor = (const unsigned char *)text; *cursor != '\0'; ++cursor) {
        unsigned long digit;
        if (*cursor < '0' || *cursor > '9') return -1;
        digit = (unsigned long)(*cursor - '0');
        if (parsed > (maximum - digit) / 10UL) return -1;
        parsed = parsed * 10UL + digit;
    }
    *value = parsed;
    return 0;
}

static int parse_listen_options(int argc, char **argv, unsigned int *count,
                                int *timeout_ms, const char **device) {
    const char *count_text = "1";
    const char *timeout_text = "1000";
    unsigned long parsed_count = 0;
    unsigned long parsed_timeout = 0;
    int count_seen = 0;
    int timeout_seen = 0;
    int device_seen = 0;

    for (int i = 2; i < argc; ++i) {
        const char *option = argv[i];
        const char **value = NULL;
        int *seen = NULL;
        if (!strcmp(option, "--count")) {
            value = &count_text; seen = &count_seen;
        } else if (!strcmp(option, "--timeout-ms")) {
            value = &timeout_text; seen = &timeout_seen;
        } else if (!strcmp(option, "--device")) {
            value = device; seen = &device_seen;
        } else {
            return -1;
        }
        if (*seen || i + 1 >= argc) return -1;
        *seen = 1;
        *value = argv[++i];
    }
    if (parse_decimal(count_text, 1000000UL, &parsed_count) != 0 || parsed_count == 0 ||
        parse_decimal(timeout_text, (unsigned long)INT_MAX, &parsed_timeout) != 0)
        return -1;
    *count = (unsigned int)parsed_count;
    *timeout_ms = (int)parsed_timeout;
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2 || !strcmp(argv[1], "--help")) { usage(); return argc < 2 ? 2 : 0; }
    const char *device = arg(argc, argv, "--device", "/dev/securewatch");
    unsigned int listen_count = 1;
    int listen_timeout_ms = 1000;
    if (!strcmp(argv[1], "listen") &&
        parse_listen_options(argc, argv, &listen_count, &listen_timeout_ms, &device) != 0) {
        fprintf(stderr, "invalid listen options\n");
        usage();
        return 2;
    }
    if (!strcmp(argv[1], "doctor")) {
        struct stat st;
        if (lstat(device, &st) != 0 || !S_ISCHR(st.st_mode)) {
            fprintf(stderr, "securewatch device unavailable: %s\n", device); return 1;
        }
        int fd = sw_open_device(device, O_RDONLY | O_NONBLOCK);
        if (fd < 0) { perror("open device"); return 1; }
        __u32 version = 0;
        int rc = ioctl(fd, SW_IOC_GET_VERSION, &version);
        close(fd);
        if (rc != 0 || version != SW_ABI_VERSION) { fprintf(stderr, "ABI mismatch\n"); return 1; }
        puts("device ready");
        return 0;
    }
    if (!strcmp(argv[1], "unload-check")) {
        if (access(device, F_OK) == 0) { puts("device is present; close readers before VM-only rmmod"); return 1; }
        puts("device absent; unload is not required"); return 0;
    }
    int fd = sw_open_device(device, (!strcmp(argv[1], "inject-lab-event") ? O_WRONLY : O_RDONLY) |
                           ((!strcmp(argv[1], "listen")) ? O_NONBLOCK : 0));
    if (fd < 0) { perror("open /dev/securewatch"); return 1; }
    int rc = 0;
    if (!strcmp(argv[1], "version")) {
        __u32 version = 0;
        rc = ioctl(fd, SW_IOC_GET_VERSION, &version);
        if (!rc) printf("ABI %u\n", version);
    } else if (!strcmp(argv[1], "stats")) {
        struct sw_stats_v1 stats;
        rc = sw_get_stats(fd, &stats);
        if (!rc) printf("{\"abi_version\":%u,\"enqueued\":%llu,\"dequeued\":%llu,\"dropped\":%llu,\"queued\":%llu}\n",
            stats.abi_version, (unsigned long long)stats.enqueued, (unsigned long long)stats.dequeued,
            (unsigned long long)stats.dropped, (unsigned long long)stats.queued);
    } else if (!strcmp(argv[1], "listen")) {
        rc = sw_listen(fd, listen_count, listen_timeout_ms);
    } else if (!strcmp(argv[1], "inject-lab-event")) {
        const char *message = arg(argc, argv, "--message", "lab-test");
        rc = sw_inject(fd, message);
    } else {
        usage(); rc = -1; errno = EINVAL;
    }
    if (rc != 0) perror("swctl");
    close(fd);
    return rc == 0 ? 0 : 1;
}
