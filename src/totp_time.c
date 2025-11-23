/* SPDX-License-Identifier: Apache-2.0 */

#include "totp_time.h"
#include <time.h>

static totp_time_func_t time_func = NULL;

static uint64_t default_time_func(void) {
    return (uint64_t)time(NULL);
}

void totp_set_time_func(totp_time_func_t func) {
    time_func = func;
}

uint64_t totp_get_time(void) {
    if (time_func == NULL) {
        return default_time_func();
    }
    return time_func();
}
