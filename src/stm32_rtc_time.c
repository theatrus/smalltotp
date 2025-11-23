#include "stm32_rtc_time.h"

static int is_leap_year(int year) {
    if (year % 400 == 0) return 1;
    if (year % 100 == 0) return 0;
    if (year % 4 == 0) return 1;
    return 0;
}

static int days_in_month(int month, int year) {
    static const uint8_t days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month < 1 || month > 12) {
        return 0;
    }

    if (month == 2 && is_leap_year(year)) {
        return 29;
    }

    return days[month - 1];
}

uint64_t stm32_rtc_to_unix_time(const RTC_TimeTypeDef *time, const RTC_DateTypeDef *date) {
    int year, month, day;
    uint64_t days_since_epoch;
    uint64_t seconds;
    int i;

    year = 2000 + date->Year;
    month = date->Month;
    day = date->Date;

    days_since_epoch = 0;

    for (i = 1970; i < year; i++) {
        days_since_epoch += is_leap_year(i) ? 366 : 365;
    }

    for (i = 1; i < month; i++) {
        days_since_epoch += days_in_month(i, year);
    }

    days_since_epoch += (day - 1);

    seconds = days_since_epoch * 86400ULL;
    seconds += (uint64_t)time->Hours * 3600;
    seconds += (uint64_t)time->Minutes * 60;
    seconds += (uint64_t)time->Seconds;

    return seconds;
}
