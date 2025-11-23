#include "hmac_sha1.h"
#include <string.h>

void hmac_sha1(const uint8_t *key, size_t key_len,
               const uint8_t *data, size_t data_len,
               uint8_t output[SHA1_DIGEST_SIZE]) {
    uint8_t k_pad[SHA1_BLOCK_SIZE];
    uint8_t tk[SHA1_DIGEST_SIZE];
    sha1_ctx_t ctx;
    int i;

    if (key_len > SHA1_BLOCK_SIZE) {
        sha1_init(&ctx);
        sha1_update(&ctx, key, key_len);
        sha1_final(&ctx, tk);
        key = tk;
        key_len = SHA1_DIGEST_SIZE;
    }

    memset(k_pad, 0, sizeof(k_pad));
    memcpy(k_pad, key, key_len);

    for (i = 0; i < SHA1_BLOCK_SIZE; i++) {
        k_pad[i] ^= 0x36;
    }

    sha1_init(&ctx);
    sha1_update(&ctx, k_pad, SHA1_BLOCK_SIZE);
    sha1_update(&ctx, data, data_len);
    sha1_final(&ctx, output);

    memset(k_pad, 0, sizeof(k_pad));
    memcpy(k_pad, key, key_len);

    for (i = 0; i < SHA1_BLOCK_SIZE; i++) {
        k_pad[i] ^= 0x5C;
    }

    sha1_init(&ctx);
    sha1_update(&ctx, k_pad, SHA1_BLOCK_SIZE);
    sha1_update(&ctx, output, SHA1_DIGEST_SIZE);
    sha1_final(&ctx, output);
}
