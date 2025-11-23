/* SPDX-License-Identifier: Apache-2.0 */

#include "totp.h"
#include "hmac_sha1.h"
#include "totp_time.h"
#include <string.h>

static uint32_t totp_generate_internal(const totp_config_t *config,
                                        uint64_t time_counter) {
    uint8_t hmac_result[SHA1_DIGEST_SIZE];
    uint8_t time_bytes[8];
    uint32_t offset;
    uint32_t binary;
    uint32_t totp;
    uint32_t divisor;
    int i;

    for (i = 7; i >= 0; i--) {
        time_bytes[i] = time_counter & 0xFF;
        time_counter >>= 8;
    }

    hmac_sha1(config->secret, config->secret_len,
              time_bytes, sizeof(time_bytes), hmac_result);

    offset = hmac_result[SHA1_DIGEST_SIZE - 1] & 0x0F;

    binary = ((hmac_result[offset] & 0x7F) << 24) |
             ((hmac_result[offset + 1] & 0xFF) << 16) |
             ((hmac_result[offset + 2] & 0xFF) << 8) |
             (hmac_result[offset + 3] & 0xFF);

    divisor = 1;
    for (i = 0; i < config->digits; i++) {
        divisor *= 10;
    }

    totp = binary % divisor;

    return totp;
}

uint32_t totp_generate(const totp_config_t *config, uint64_t timestamp) {
    uint64_t time_counter = timestamp / config->time_step;
    return totp_generate_internal(config, time_counter);
}

uint32_t totp_generate_current(const totp_config_t *config) {
    return totp_generate(config, totp_get_time());
}

int totp_validate(const totp_config_t *config, uint32_t code,
                  uint64_t timestamp, uint32_t window_before, uint32_t window_after) {
    uint64_t time_counter = timestamp / config->time_step;
    uint32_t i;

    uint32_t generated = totp_generate_internal(config, time_counter);
    if (generated == code) {
        return 1;
    }

    for (i = 1; i <= window_before; i++) {
        if (time_counter >= i) {
            generated = totp_generate_internal(config, time_counter - i);
            if (generated == code) {
                return 1;
            }
        }
    }

    for (i = 1; i <= window_after; i++) {
        generated = totp_generate_internal(config, time_counter + i);
        if (generated == code) {
            return 1;
        }
    }

    return 0;
}

int totp_validate_current(const totp_config_t *config, uint32_t code,
                          uint32_t window_before, uint32_t window_after) {
    return totp_validate(config, code, totp_get_time(), window_before, window_after);
}
