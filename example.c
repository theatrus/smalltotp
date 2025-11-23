#include <stdio.h>
#include <stdint.h>
#include "totp.h"
#include "base32.h"

int main(void) {
    const char *secret_b32 = "JBSWY3DPEHPK3PXP";
    uint8_t secret[64];
    int secret_len;

    printf("TOTP Example\n");
    printf("============\n\n");

    secret_len = base32_decode(secret_b32, secret, sizeof(secret));
    if (secret_len <= 0) {
        printf("Error: Failed to decode Base32 secret\n");
        return 1;
    }

    printf("Secret (Base32): %s\n", secret_b32);
    printf("Secret length: %d bytes\n\n", secret_len);

    totp_config_t config = {
        .secret = secret,
        .secret_len = (size_t)secret_len,
        .time_step = 30,
        .digits = 6
    };

    uint32_t code = totp_generate_current(&config);
    printf("Current TOTP code: %06u\n\n", code);

    printf("Testing validation with different time windows:\n");
    printf("  Exact match (0, 0):           %s\n",
           totp_validate_current(&config, code, 0, 0) ? "VALID" : "INVALID");
    printf("  Accept past codes (1, 0):     %s\n",
           totp_validate_current(&config, code, 1, 0) ? "VALID" : "INVALID");
    printf("  Accept future codes (0, 1):   %s\n",
           totp_validate_current(&config, code, 0, 1) ? "VALID" : "INVALID");
    printf("  Accept ±30s (1, 1):           %s\n",
           totp_validate_current(&config, code, 1, 1) ? "VALID" : "INVALID");
    printf("  Accept ±60s (2, 2):           %s\n",
           totp_validate_current(&config, code, 2, 2) ? "VALID" : "INVALID");

    uint32_t wrong_code = 999999;
    printf("\nTesting wrong code (%06u):\n", wrong_code);
    printf("  Exact match (0, 0): %s\n",
           totp_validate_current(&config, wrong_code, 0, 0) ? "VALID" : "INVALID");
    printf("  Accept ±60s (2, 2): %s\n",
           totp_validate_current(&config, wrong_code, 2, 2) ? "VALID" : "INVALID");

    return 0;
}
