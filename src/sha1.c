/* SPDX-License-Identifier: Apache-2.0 */

#include "sha1.h"
#include <string.h>

#define ROL32(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

static void sha1_transform(uint32_t state[5], const uint8_t block[SHA1_BLOCK_SIZE]) {
    uint32_t w[80];
    uint32_t a, b, c, d, e, temp;
    int i;

    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)block[i * 4] << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8) |
               ((uint32_t)block[i * 4 + 3]);
    }

    for (i = 16; i < 80; i++) {
        w[i] = ROL32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    for (i = 0; i < 80; i++) {
        uint32_t f, k;

        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }

        temp = ROL32(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = ROL32(b, 30);
        b = a;
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

void sha1_init(sha1_ctx_t *ctx) {
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
    ctx->count[0] = 0;
    ctx->count[1] = 0;
}

void sha1_update(sha1_ctx_t *ctx, const uint8_t *data, size_t len) {
    size_t i, index, part_len;

    index = (size_t)((ctx->count[0] >> 3) & 0x3F);

    if ((ctx->count[0] += ((uint32_t)len << 3)) < ((uint32_t)len << 3)) {
        ctx->count[1]++;
    }
    ctx->count[1] += ((uint32_t)len >> 29);

    part_len = 64 - index;

    if (len >= part_len) {
        memcpy(&ctx->buffer[index], data, part_len);
        sha1_transform(ctx->state, ctx->buffer);

        for (i = part_len; i + 63 < len; i += 64) {
            sha1_transform(ctx->state, &data[i]);
        }
        index = 0;
    } else {
        i = 0;
    }

    memcpy(&ctx->buffer[index], &data[i], len - i);
}

void sha1_final(sha1_ctx_t *ctx, uint8_t digest[SHA1_DIGEST_SIZE]) {
    uint8_t final_count[8];
    uint8_t c;
    int i;

    for (i = 0; i < 8; i++) {
        final_count[i] = (uint8_t)((ctx->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 0xFF);
    }

    c = 0x80;
    sha1_update(ctx, &c, 1);

    while ((ctx->count[0] & 504) != 448) {
        c = 0x00;
        sha1_update(ctx, &c, 1);
    }

    sha1_update(ctx, final_count, 8);

    for (i = 0; i < SHA1_DIGEST_SIZE; i++) {
        digest[i] = (uint8_t)((ctx->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 0xFF);
    }
}
