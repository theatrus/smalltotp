#include <stdio.h>
#include "totp.h"
#include "base32.h"
#include "totp_time.h"
#include "stm32_rtc_time.h"

static RTC_HandleTypeDef hrtc;

uint64_t stm32_rtc_get_unix_time(void) {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    return stm32_rtc_to_unix_time(&sTime, &sDate);
}

int main(void) {
    const char *secret_b32 = "JBSWY3DPEHPK3PXP";
    uint8_t secret[64];
    int secret_len;
    uint32_t code;

    HAL_Init();
    SystemClock_Config();
    MX_RTC_Init();

    secret_len = base32_decode(secret_b32, secret, sizeof(secret));
    if (secret_len <= 0) {
        Error_Handler();
    }

    totp_set_time_func(stm32_rtc_get_unix_time);

    totp_config_t config = {
        .secret = secret,
        .secret_len = (size_t)secret_len,
        .time_step = 30,
        .digits = 6
    };

    while (1) {
        code = totp_generate_current(&config);
        printf("TOTP Code: %06u\n", code);

        if (totp_validate_current(&config, code, 1, 0)) {
            printf("Code validated (accepting past 30s)\n");
        }

        HAL_Delay(1000);
    }

    return 0;
}
