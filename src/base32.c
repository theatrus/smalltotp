#include "base32.h"

static int base32_char_to_val(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a';
    }
    if (c >= '2' && c <= '7') {
        return 26 + (c - '2');
    }
    return -1;
}

int base32_decode(const char *encoded, uint8_t *result, size_t result_len) {
    size_t encoded_len = 0;
    size_t output_len = 0;
    uint32_t buffer = 0;
    int bits_left = 0;
    int val;

    while (encoded[encoded_len] != '\0' && encoded[encoded_len] != '=') {
        encoded_len++;
    }

    for (size_t i = 0; i < encoded_len; i++) {
        val = base32_char_to_val(encoded[i]);
        if (val < 0) {
            continue;
        }

        buffer = (buffer << 5) | val;
        bits_left += 5;

        if (bits_left >= 8) {
            if (output_len >= result_len) {
                return -1;
            }
            result[output_len++] = (buffer >> (bits_left - 8)) & 0xFF;
            bits_left -= 8;
        }
    }

    return (int)output_len;
}
