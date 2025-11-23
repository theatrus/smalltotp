#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "totp.h"
#include "base32.h"
#include "totp_time.h"

static uint64_t mock_time = 0;

static uint64_t mock_time_func(void) {
    return mock_time;
}

static void test_base32_decode(void) {
    printf("Testing Base32 decode...\n");

    const char *encoded = "JBSWY3DPEHPK3PXP";
    uint8_t decoded[32];
    int len = base32_decode(encoded, decoded, sizeof(decoded));

    assert(len > 0);
    printf("  Decoded length: %d\n", len);
    printf("  Decoded bytes: ");
    for (int i = 0; i < len; i++) {
        printf("%02x ", decoded[i]);
    }
    printf("\n");

    const uint8_t expected[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x21, 0xde, 0xad, 0xbe, 0xef};
    assert(len == 10);
    assert(memcmp(decoded, expected, 10) == 0);

    printf("  Base32 decode: PASSED\n");
}

static void test_totp_rfc6238_vectors(void) {
    printf("Testing TOTP with RFC 6238 test vectors...\n");

    const char *secret_b32 = "GEZDGNBVGY3TQOJQGEZDGNBVGY3TQOJQ";
    uint8_t secret[64];
    int secret_len = base32_decode(secret_b32, secret, sizeof(secret));
    assert(secret_len > 0);

    totp_config_t config = {
        .secret = secret,
        .secret_len = (size_t)secret_len,
        .time_step = 30,
        .digits = 8
    };

    struct {
        uint64_t time;
        uint32_t expected_code;
    } test_vectors[] = {
        {59, 94287082},
        {1111111109, 7081804},
        {1111111111, 14050471},
        {1234567890, 89005924},
        {2000000000, 69279037},
        {20000000000ULL, 65353130}
    };

    for (size_t i = 0; i < sizeof(test_vectors) / sizeof(test_vectors[0]); i++) {
        uint32_t code = totp_generate(&config, test_vectors[i].time);
        printf("  Time: %llu, Expected: %08u, Got: %08u\n",
               (unsigned long long)test_vectors[i].time,
               test_vectors[i].expected_code,
               code);
        assert(code == test_vectors[i].expected_code);
    }

    printf("  RFC 6238 vectors: PASSED\n");
}

static void test_totp_6_digits(void) {
    printf("Testing TOTP with 6 digits...\n");

    const char *secret_b32 = "JBSWY3DPEHPK3PXP";
    uint8_t secret[64];
    int secret_len = base32_decode(secret_b32, secret, sizeof(secret));
    assert(secret_len > 0);

    totp_config_t config = {
        .secret = secret,
        .secret_len = (size_t)secret_len,
        .time_step = 30,
        .digits = 6
    };

    uint64_t test_time = 1234567890;
    uint32_t code = totp_generate(&config, test_time);

    printf("  6-digit code at time %llu: %06u\n",
           (unsigned long long)test_time, code);
    assert(code < 1000000);

    printf("  6-digit TOTP: PASSED\n");
}

static void test_totp_time_window(void) {
    printf("Testing TOTP time window validation...\n");

    const char *secret_b32 = "JBSWY3DPEHPK3PXP";
    uint8_t secret[64];
    int secret_len = base32_decode(secret_b32, secret, sizeof(secret));
    assert(secret_len > 0);

    totp_config_t config = {
        .secret = secret,
        .secret_len = (size_t)secret_len,
        .time_step = 30,
        .digits = 6
    };

    uint64_t base_time = 1000000000;
    uint32_t code_current = totp_generate(&config, base_time);
    uint32_t code_prev = totp_generate(&config, base_time - 30);
    uint32_t code_prev2 = totp_generate(&config, base_time - 60);
    uint32_t code_next = totp_generate(&config, base_time + 30);
    uint32_t code_next2 = totp_generate(&config, base_time + 60);

    printf("  Code at T-60: %06u\n", code_prev2);
    printf("  Code at T-30: %06u\n", code_prev);
    printf("  Code at T:    %06u\n", code_current);
    printf("  Code at T+30: %06u\n", code_next);
    printf("  Code at T+60: %06u\n", code_next2);

    assert(totp_validate(&config, code_current, base_time, 0, 0) == 1);
    printf("  Current code validated with window_before=0, window_after=0: PASSED\n");

    assert(totp_validate(&config, code_prev, base_time, 0, 0) == 0);
    printf("  Previous code rejected with window_before=0, window_after=0: PASSED\n");

    assert(totp_validate(&config, code_prev, base_time, 1, 0) == 1);
    printf("  Previous code accepted with window_before=1, window_after=0: PASSED\n");

    assert(totp_validate(&config, code_next, base_time, 0, 1) == 1);
    printf("  Next code accepted with window_before=0, window_after=1: PASSED\n");

    assert(totp_validate(&config, code_next, base_time, 1, 0) == 0);
    printf("  Next code rejected with window_before=1, window_after=0: PASSED\n");

    assert(totp_validate(&config, code_prev2, base_time, 2, 0) == 1);
    printf("  Code from T-60 accepted with window_before=2, window_after=0: PASSED\n");

    assert(totp_validate(&config, code_prev2, base_time, 1, 0) == 0);
    printf("  Code from T-60 rejected with window_before=1, window_after=0: PASSED\n");

    assert(totp_validate(&config, code_next2, base_time, 0, 2) == 1);
    printf("  Code from T+60 accepted with window_before=0, window_after=2: PASSED\n");

    assert(totp_validate(&config, code_next2, base_time, 0, 1) == 0);
    printf("  Code from T+60 rejected with window_before=0, window_after=1: PASSED\n");

    assert(totp_validate(&config, code_prev, base_time, 2, 2) == 1);
    printf("  Previous code accepted with window_before=2, window_after=2: PASSED\n");

    assert(totp_validate(&config, code_next, base_time, 2, 2) == 1);
    printf("  Next code accepted with window_before=2, window_after=2: PASSED\n");

    printf("  Time window validation: PASSED\n");
}

static void test_totp_current_time(void) {
    printf("Testing TOTP with current time function...\n");

    totp_set_time_func(mock_time_func);

    const char *secret_b32 = "JBSWY3DPEHPK3PXP";
    uint8_t secret[64];
    int secret_len = base32_decode(secret_b32, secret, sizeof(secret));
    assert(secret_len > 0);

    totp_config_t config = {
        .secret = secret,
        .secret_len = (size_t)secret_len,
        .time_step = 30,
        .digits = 6
    };

    mock_time = 1234567890;
    uint32_t code1 = totp_generate_current(&config);
    uint32_t code2 = totp_generate(&config, 1234567890);

    assert(code1 == code2);
    printf("  Current time function: PASSED\n");

    assert(totp_validate_current(&config, code1, 0, 0) == 1);
    printf("  Current time validation: PASSED\n");

    totp_set_time_func(NULL);
}

int main(void) {
    printf("TOTP Library Test Suite\n");
    printf("=======================\n\n");

    test_base32_decode();
    printf("\n");

    test_totp_rfc6238_vectors();
    printf("\n");

    test_totp_6_digits();
    printf("\n");

    test_totp_time_window();
    printf("\n");

    test_totp_current_time();
    printf("\n");

    printf("=======================\n");
    printf("All tests PASSED!\n");

    return 0;
}
