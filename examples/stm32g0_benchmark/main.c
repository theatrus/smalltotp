/* SPDX-License-Identifier: Apache-2.0 */
/*
 * STM32G0 TOTP Benchmark Example
 * Copyright 2025 Yann Ramin
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "totp.h"
#include "base32.h"
#include "usart.h"

/* STM32G0 register definitions */
#define RCC_BASE            0x40021000
#define RCC_CR              (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_CFGR            (*(volatile uint32_t *)(RCC_BASE + 0x08))
#define RCC_PLLCFGR         (*(volatile uint32_t *)(RCC_BASE + 0x0C))
#define RCC_IOPENR          (*(volatile uint32_t *)(RCC_BASE + 0x34))
#define RCC_AHBENR          (*(volatile uint32_t *)(RCC_BASE + 0x38))

#define FLASH_BASE          0x40022000
#define FLASH_ACR           (*(volatile uint32_t *)(FLASH_BASE + 0x00))

#define GPIOA_BASE          0x50000000
#define GPIOA_MODER         (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR           (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

/* SysTick registers */
#define SYST_CSR            (*(volatile uint32_t *)0xE000E010)
#define SYST_RVR            (*(volatile uint32_t *)0xE000E014)
#define SYST_CVR            (*(volatile uint32_t *)0xE000E018)

#define SYST_CSR_ENABLE     (1 << 0)
#define SYST_CSR_CLKSOURCE  (1 << 2)

/* System clock frequency - STM32G0 default is 16 MHz HSI */
#define SYSTEM_CLOCK_HZ     16000000

/* SysTick ticks per millisecond */
#define TICKS_PER_MS        (SYSTEM_CLOCK_HZ / 1000)

static void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 4000; i++) {
        __asm__ volatile ("nop");
    }
}

static void gpio_init(void) {
    /* Enable GPIOA clock */
    RCC_IOPENR |= (1 << 0);

    /* Configure PA5 as output (LED on most STM32G0 boards) */
    GPIOA_MODER &= ~(3 << (5 * 2));
    GPIOA_MODER |= (1 << (5 * 2));
}

static void led_on(void) {
    GPIOA_ODR |= (1 << 5);
}

static void led_off(void) {
    GPIOA_ODR &= ~(1 << 5);
}

static void led_toggle(void) {
    GPIOA_ODR ^= (1 << 5);
}

static volatile uint32_t systick_ms = 0;

static void systick_init(void) {
    /* Configure SysTick to generate interrupt every 1ms */
    SYST_RVR = TICKS_PER_MS - 1;
    SYST_CVR = 0;
    SYST_CSR = SYST_CSR_ENABLE | SYST_CSR_CLKSOURCE | (1 << 1); /* Enable with interrupt */
}

void SysTick_Handler(void) {
    systick_ms++;
}

static uint32_t get_milliseconds(void) {
    return systick_ms;
}


/* Test secret from RFC 6238 */
static const char *test_secret_b32 = "GEZDGNBVGY3TQOJQGEZDGNBVGY3TQOJQ";

/* Known test vectors from RFC 6238 */
typedef struct {
    uint64_t time;
    uint32_t expected_code;
} test_vector_t;

static const test_vector_t test_vectors[] = {
    {59, 94287082},
    {1111111109, 7081804},
    {1234567890, 89005924},
};

static void run_benchmark(void) {
    uint8_t secret[64];
    int secret_len;
    uint32_t code;
    uint32_t time_start, time_end, time_elapsed;
    uint32_t total_time = 0;
    const int iterations = 100;

    printf("\n=== TOTP Benchmark on STM32G0 ===\n");
    printf("System Clock: %lu Hz\n", (unsigned long)SYSTEM_CLOCK_HZ);
    printf("Processor: Cortex-M0+\n\n");

    /* Decode secret */
    secret_len = base32_decode(test_secret_b32, secret, sizeof(secret));
    if (secret_len <= 0) {
        printf("ERROR: Failed to decode Base32 secret\n");
        return;
    }

    printf("Secret decoded: %d bytes\n", secret_len);

    totp_config_t config = {
        .secret = secret,
        .secret_len = (size_t)secret_len,
        .time_step = 30,
        .digits = 8
    };

    /* Validate against test vectors */
    printf("\n--- RFC 6238 Test Vectors ---\n");
    for (size_t i = 0; i < sizeof(test_vectors) / sizeof(test_vectors[0]); i++) {
        led_toggle();

        time_start = get_milliseconds();
        code = totp_generate(&config, test_vectors[i].time);
        time_end = get_milliseconds();
        time_elapsed = time_end - time_start;

        printf("Time: %lu\n", (unsigned long)test_vectors[i].time);
        printf("  Expected: %08lu\n", (unsigned long)test_vectors[i].expected_code);
        printf("  Generated: %08lu\n", (unsigned long)code);
        printf("  Result: %s\n", code == test_vectors[i].expected_code ? "PASS" : "FAIL");
        printf("  Time: %lu ms\n", (unsigned long)time_elapsed);
    }

    /* Performance benchmark */
    printf("\n--- Performance Benchmark ---\n");
    printf("Running %d iterations...\n", iterations);

    time_start = get_milliseconds();
    for (int i = 0; i < iterations; i++) {
        if (i % 10 == 0) {
            led_toggle();
        }
        code = totp_generate(&config, 1234567890);
    }
    time_end = get_milliseconds();
    total_time = time_end - time_start;

    float avg_ms = (float)total_time / iterations;
    uint32_t codes_per_sec = total_time > 0 ? (iterations * 1000) / total_time : 0;

    printf("\nResults:\n");
    printf("  Total time: %lu ms\n", (unsigned long)total_time);
    printf("  Average time: %.2f ms per code\n", avg_ms);
    printf("  Codes per second: %lu\n", (unsigned long)codes_per_sec);

    /* Memory usage estimate */
    printf("\n--- Memory Usage ---\n");
    printf("  Code size: ~1.2 KB (library)\n");
    printf("  Stack usage: ~512 bytes (estimated)\n");
    printf("  Secret storage: %d bytes\n", secret_len);

    printf("\n=== Benchmark Complete ===\n");
}

static void run_validation_tests(void) {
    uint8_t secret[64];
    int secret_len;

    printf("\n=== TOTP Validation Tests ===\n");

    secret_len = base32_decode(test_secret_b32, secret, sizeof(secret));
    if (secret_len <= 0) {
        printf("ERROR: Failed to decode secret\n");
        return;
    }

    totp_config_t config = {
        .secret = secret,
        .secret_len = (size_t)secret_len,
        .time_step = 30,
        .digits = 8
    };

    uint64_t test_time = 1234567890;
    uint32_t code_current = totp_generate(&config, test_time);
    uint32_t code_prev = totp_generate(&config, test_time - 30);
    uint32_t code_next = totp_generate(&config, test_time + 30);

    printf("\nTime window validation:\n");
    printf("  T-30s: %08lu\n", (unsigned long)code_prev);
    printf("  T:     %08lu\n", (unsigned long)code_current);
    printf("  T+30s: %08lu\n", (unsigned long)code_next);

    /* Test window validation */
    int result;

    result = totp_validate(&config, code_current, test_time, 0, 0);
    printf("\nCurrent code (0,0): %s\n", result ? "PASS" : "FAIL");

    result = totp_validate(&config, code_prev, test_time, 1, 0);
    printf("Previous code (1,0): %s\n", result ? "PASS" : "FAIL");

    result = totp_validate(&config, code_next, test_time, 0, 1);
    printf("Next code (0,1): %s\n", result ? "PASS" : "FAIL");

    result = totp_validate(&config, code_prev, test_time, 0, 0);
    printf("Previous code (0,0): %s\n", result ? "FAIL (expected)" : "ERROR");

    printf("\n=== Validation Complete ===\n");
}

int main(void) {
    /* Initialize peripherals */
    gpio_init();
    systick_init();
    usart2_init(115200);

    printf("\n\n");
    printf("====================================\n");
    printf("  smalltotp STM32G0 Benchmark\n");
    printf("====================================\n");
    printf("  Running continuous benchmarks\n");
    printf("====================================\n\n");

    led_on();
    delay_ms(500);
    led_off();

    uint32_t iteration = 0;

    /* Run benchmarks in infinite loop */
    while (1) {
        printf("\n\n");
        printf(">>> Iteration %lu <<<\n", (unsigned long)++iteration);

        /* Run benchmark */
        run_benchmark();

        /* Run validation tests */
        run_validation_tests();

        printf("\n--- Waiting 5 seconds before next iteration ---\n");

        /* Blink LED while waiting */
        for (int i = 0; i < 10; i++) {
            led_toggle();
            delay_ms(500);
        }
    }

    return 0;
}
