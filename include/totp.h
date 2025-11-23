#ifndef TOTP_H
#define TOTP_H

#include <stdint.h>
#include <stddef.h>

#define TOTP_DEFAULT_TIME_STEP 30
#define TOTP_DEFAULT_DIGITS 6

typedef struct {
    const uint8_t *secret;
    size_t secret_len;
    uint32_t time_step;
    uint8_t digits;
} totp_config_t;

uint32_t totp_generate(const totp_config_t *config, uint64_t timestamp);

uint32_t totp_generate_current(const totp_config_t *config);

int totp_validate(const totp_config_t *config, uint32_t code,
                  uint64_t timestamp, uint32_t window_before, uint32_t window_after);

int totp_validate_current(const totp_config_t *config, uint32_t code,
                          uint32_t window_before, uint32_t window_after);

#endif
