/* SPDX-License-Identifier: Apache-2.0 */

#ifndef STM32_RTC_TIME_H
#define STM32_RTC_TIME_H

#include <stdint.h>

typedef struct {
    uint8_t Hours;
    uint8_t Minutes;
    uint8_t Seconds;
    uint8_t TimeFormat;
    uint32_t SubSeconds;
    uint32_t SecondFraction;
    uint32_t DayLightSaving;
    uint32_t StoreOperation;
} RTC_TimeTypeDef;

typedef struct {
    uint8_t WeekDay;
    uint8_t Month;
    uint8_t Date;
    uint8_t Year;
} RTC_DateTypeDef;

uint64_t stm32_rtc_to_unix_time(const RTC_TimeTypeDef *time, const RTC_DateTypeDef *date);

#endif
