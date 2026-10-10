#ifndef SW_JSON_OUTPUT_H
#define SW_JSON_OUTPUT_H

#include <errno.h>
#include <stddef.h>
#include <stdio.h>

static inline size_t sw_utf8_sequence_length(const unsigned char *bytes, size_t length) {
    if (length == 0U) return 0U;
    const unsigned char first = bytes[0];
    if (first <= 0x7fU) return 1U;

    size_t continuation = 0U;
    unsigned int codepoint = 0U;
    unsigned int minimum = 0U;
    if (first >= 0xc2U && first <= 0xdfU) {
        continuation = 1U; codepoint = first & 0x1fU; minimum = 0x80U;
    } else if (first >= 0xe0U && first <= 0xefU) {
        continuation = 2U; codepoint = first & 0x0fU; minimum = 0x800U;
    } else if (first >= 0xf0U && first <= 0xf4U) {
        continuation = 3U; codepoint = first & 0x07U; minimum = 0x10000U;
    } else {
        return 0U;
    }
    if (continuation > length - 1U) return 0U;
    for (size_t i = 1U; i <= continuation; ++i) {
        if ((bytes[i] & 0xc0U) != 0x80U) return 0U;
        codepoint = (codepoint << 6U) | (bytes[i] & 0x3fU);
    }
    if (codepoint < minimum || codepoint > 0x10ffffU ||
        (codepoint >= 0xd800U && codepoint <= 0xdfffU)) return 0U;
    return continuation + 1U;
}

static inline int sw_json_write_string(FILE *output, const unsigned char *bytes, size_t length) {
    static const char hex[] = "0123456789abcdef";
    if (output == NULL || (bytes == NULL && length != 0U)) {
        errno = EINVAL;
        return -1;
    }
    if (fputc('"', output) == EOF) return -1;
    for (size_t i = 0U; i < length;) {
        const unsigned char value = bytes[i];
        const char *escape = NULL;
        switch (value) {
        case '"': escape = "\\\""; break;
        case '\\': escape = "\\\\"; break;
        case '\b': escape = "\\b"; break;
        case '\f': escape = "\\f"; break;
        case '\n': escape = "\\n"; break;
        case '\r': escape = "\\r"; break;
        case '\t': escape = "\\t"; break;
        default: break;
        }
        if (escape != NULL) {
            if (fputs(escape, output) == EOF) return -1;
            ++i;
        } else if (value < 0x20U) {
            const char escaped_byte[6] = {
                '\\', 'u', '0', '0', hex[value >> 4U], hex[value & 0x0fU]
            };
            if (fwrite(escaped_byte, 1U, sizeof(escaped_byte), output) != sizeof(escaped_byte))
                return -1;
            ++i;
        } else if (value < 0x80U) {
            if (fputc(value, output) == EOF) return -1;
            ++i;
        } else {
            const size_t sequence = sw_utf8_sequence_length(bytes + i, length - i);
            if (sequence != 0U) {
                if (fwrite(bytes + i, 1U, sequence, output) != sequence) return -1;
                i += sequence;
            } else {
                const char escaped_byte[6] = {
                    '\\', 'u', '0', '0', hex[value >> 4U], hex[value & 0x0fU]
                };
                if (fwrite(escaped_byte, 1U, sizeof(escaped_byte), output) != sizeof(escaped_byte))
                    return -1;
                ++i;
            }
        }
    }
    return fputc('"', output) == EOF ? -1 : 0;
}

#endif
