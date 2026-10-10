#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../userspace/src/json_output.h"

static int matches(const unsigned char *input, size_t length, const char *expected) {
    char *output = NULL;
    size_t output_length = 0U;
    FILE *stream = open_memstream(&output, &output_length);
    if (stream == NULL) return 0;
    int rc = sw_json_write_string(stream, input, length);
    if (fclose(stream) != 0) rc = -1;
    const size_t expected_length = strlen(expected);
    const int ok = rc == 0 && output_length == expected_length &&
                   memcmp(output, expected, expected_length) == 0;
    free(output);
    return ok;
}

int main(void) {
    static const unsigned char controls[] = {
        '"', '\\', '\b', '\f', '\n', '\r', '\t', 0x00U, 0x1fU
    };
    static const char controls_expected[] =
        "\"\\\"\\\\\\b\\f\\n\\r\\t\\u0000\\u001f\"";
    static const unsigned char valid_utf8[] = { 'h', 0xc3U, 0xa9U };
    static const char valid_utf8_expected[] = "\"h\xc3\xa9\"";
    static const unsigned char invalid_utf8[] = { 0xc3U, 'x', 0xffU, 0x80U };
    static const char invalid_utf8_expected[] = "\"\\u00c3x\\u00ff\\u0080\"";
    static const unsigned char truncated_utf8[] = { 0xe2U, 0x82U };
    static const char truncated_utf8_expected[] = "\"\\u00e2\\u0082\"";
    static const unsigned char overlong_utf8[] = { 0xc0U, 0xafU };
    static const char overlong_utf8_expected[] = "\"\\u00c0\\u00af\"";
    static const unsigned char surrogate_utf8[] = { 0xedU, 0xa0U, 0x80U };
    static const char surrogate_utf8_expected[] = "\"\\u00ed\\u00a0\\u0080\"";
    static const unsigned char out_of_range_utf8[] = { 0xf4U, 0x90U, 0x80U, 0x80U };
    static const char out_of_range_utf8_expected[] =
        "\"\\u00f4\\u0090\\u0080\\u0080\"";

    if (sw_json_write_string(NULL, controls, sizeof(controls)) != -1 || errno != EINVAL ||
        !matches(controls, sizeof(controls), controls_expected) ||
        !matches(valid_utf8, sizeof(valid_utf8), valid_utf8_expected) ||
        !matches(invalid_utf8, sizeof(invalid_utf8), invalid_utf8_expected) ||
        !matches(truncated_utf8, sizeof(truncated_utf8), truncated_utf8_expected) ||
        !matches(overlong_utf8, sizeof(overlong_utf8), overlong_utf8_expected) ||
        !matches(surrogate_utf8, sizeof(surrogate_utf8), surrogate_utf8_expected) ||
        !matches(out_of_range_utf8, sizeof(out_of_range_utf8), out_of_range_utf8_expected)) {
        fputs("JSON string encoding regression\n", stderr);
        return 1;
    }
    puts("KDL JSON output encoding: PASS");
    return 0;
}
