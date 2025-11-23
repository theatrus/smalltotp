#include <stdio.h>
#include <assert.h>
#include "stm32_rtc_time.h"

static void test_stm32_rtc_conversion(void) {
    printf("Testing STM32 RTC to Unix timestamp conversion...\n");

    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};
    uint64_t timestamp;

    date.Year = 0;
    date.Month = 1;
    date.Date = 1;
    time.Hours = 0;
    time.Minutes = 0;
    time.Seconds = 0;
    timestamp = stm32_rtc_to_unix_time(&time, &date);
    printf("  2000-01-01 00:00:00 = %llu (expected 946684800)\n",
           (unsigned long long)timestamp);
    assert(timestamp == 946684800);

    date.Year = 9;
    date.Month = 2;
    date.Date = 13;
    time.Hours = 23;
    time.Minutes = 31;
    time.Seconds = 30;
    timestamp = stm32_rtc_to_unix_time(&time, &date);
    printf("  2009-02-13 23:31:30 = %llu (expected 1234567890)\n",
           (unsigned long long)timestamp);
    assert(timestamp == 1234567890);

    date.Year = 24;
    date.Month = 12;
    date.Date = 31;
    time.Hours = 23;
    time.Minutes = 59;
    time.Seconds = 59;
    timestamp = stm32_rtc_to_unix_time(&time, &date);
    printf("  2024-12-31 23:59:59 = %llu (expected 1735689599)\n",
           (unsigned long long)timestamp);
    assert(timestamp == 1735689599);

    date.Year = 20;
    date.Month = 2;
    date.Date = 29;
    time.Hours = 12;
    time.Minutes = 0;
    time.Seconds = 0;
    timestamp = stm32_rtc_to_unix_time(&time, &date);
    printf("  2020-02-29 12:00:00 = %llu (leap year test)\n",
           (unsigned long long)timestamp);
    assert(timestamp == 1582977600);

    printf("  STM32 RTC conversion: PASSED\n");
}

int main(void) {
    printf("STM32 RTC Conversion Test Suite\n");
    printf("================================\n\n");

    test_stm32_rtc_conversion();

    printf("\n================================\n");
    printf("All tests PASSED!\n");

    return 0;
}
