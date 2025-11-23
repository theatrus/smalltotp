/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BASE32_H
#define BASE32_H

#include <stdint.h>
#include <stddef.h>

int base32_decode(const char *encoded, uint8_t *result, size_t result_len);

#endif
