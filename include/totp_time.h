/* SPDX-License-Identifier: Apache-2.0 */

#ifndef TOTP_TIME_H
#define TOTP_TIME_H

#include <stdint.h>

typedef uint64_t (*totp_time_func_t)(void);

void totp_set_time_func(totp_time_func_t func);
uint64_t totp_get_time(void);

#endif
