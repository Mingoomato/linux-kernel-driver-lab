#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sw_client.h"

enum test_mode {
    TEST_EAGAIN_AFTER_POLL,
    TEST_POLL_INTERRUPTED,
    TEST_READ_INTERRUPTED,
    TEST_TIMEOUT
};

static enum test_mode mode;
static int poll_calls;
static int read_calls;

int sw_test_poll(struct pollfd *fds, nfds_t count, int timeout_ms) {
    (void)timeout_ms;
    ++poll_calls;
    if (count != 1U) return -1;
    if (mode == TEST_TIMEOUT) return 0;
    if (mode == TEST_POLL_INTERRUPTED && poll_calls == 1) {
        errno = EINTR;
        return -1;
    }
    fds[0].revents = POLLIN;
    return 1;
}

ssize_t sw_test_read(int fd, void *buffer, size_t length) {
    (void)fd;
    ++read_calls;
    if (mode == TEST_EAGAIN_AFTER_POLL && read_calls == 1) {
        errno = EAGAIN;
        return -1;
    }
    if (mode == TEST_READ_INTERRUPTED && read_calls == 1) {
        errno = EINTR;
        return -1;
    }
    if (length < sizeof(struct sw_event_v1)) {
        errno = EINVAL;
        return -1;
    }
    struct sw_event_v1 event;
    memset(&event, 0, sizeof(event));
    event.abi_version = SW_ABI_VERSION;
    event.event_type = SW_EVENT_LAB;
    event.sequence = 1U;
    event.timestamp_ns = 1U;
    event.payload_len = 2U;
    memcpy(event.payload, "ok", 2U);
    memcpy(buffer, &event, sizeof(event));
    return (ssize_t)sizeof(event);
}

static int run_case(enum test_mode selected, int expected_rc,
                    int expected_errno, int expected_polls, int expected_reads) {
    mode = selected;
    poll_calls = 0;
    read_calls = 0;
    errno = 0;
    const int rc = sw_listen(17, 1U, selected == TEST_TIMEOUT ? 0 : 1000);
    if (rc != expected_rc || (expected_errno >= 0 && errno != expected_errno) ||
        poll_calls != expected_polls || read_calls != expected_reads) {
        fprintf(stderr, "listen case %d: rc=%d errno=%d polls=%d reads=%d\n",
                (int)selected, rc, errno, poll_calls, read_calls);
        return 1;
    }
    return 0;
}

int main(void) {
    if (run_case(TEST_EAGAIN_AFTER_POLL, 0, -1, 2, 2) != 0 ||
        run_case(TEST_POLL_INTERRUPTED, 0, -1, 2, 1) != 0 ||
        run_case(TEST_READ_INTERRUPTED, 0, -1, 2, 2) != 0 ||
        run_case(TEST_TIMEOUT, -1, ETIMEDOUT, 1, 0) != 0) return 1;
    puts("KDL listen retry/timeout semantics: PASS");
    return 0;
}
