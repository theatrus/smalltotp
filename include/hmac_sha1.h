/* SPDX-License-Identifier: Apache-2.0 */

#ifndef HMAC_SHA1_H
#define HMAC_SHA1_H

#include <stdint.h>
#include <stddef.h>
#include "sha1.h"

void hmac_sha1(const uint8_t *key, size_t key_len,
               const uint8_t *data, size_t data_len,
               uint8_t output[SHA1_DIGEST_SIZE]);

#endif
