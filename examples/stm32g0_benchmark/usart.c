/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Simple USART2 driver for STM32G0
 * Copyright 2025 Yann Ramin
 */

#include "usart.h"

/* STM32G0 register definitions */
#define RCC_BASE            0x40021000
#define RCC_IOPENR          (*(volatile uint32_t *)(RCC_BASE + 0x34))
#define RCC_APBENR1         (*(volatile uint32_t *)(RCC_BASE + 0x3C))

#define GPIOA_BASE          0x50000000
#define GPIOA_MODER         (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL          (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

#define USART2_BASE         0x40004400
#define USART2_CR1          (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_BRR          (*(volatile uint32_t *)(USART2_BASE + 0x0C))
#define USART2_ISR          (*(volatile uint32_t *)(USART2_BASE + 0x1C))
#define USART2_TDR          (*(volatile uint32_t *)(USART2_BASE + 0x28))

/* USART CR1 bits */
#define USART_CR1_UE        (1 << 0)
#define USART_CR1_TE        (1 << 3)

/* USART ISR bits */
#define USART_ISR_TXE       (1 << 7)

/* System clock - STM32G0 defaults to 16MHz HSI */
#define SYSTEM_CLOCK_HZ     16000000

void usart2_init(uint32_t baudrate) {
    /* Enable GPIOA and USART2 clocks */
    RCC_IOPENR |= (1 << 0);     /* GPIOA */
    RCC_APBENR1 |= (1 << 17);   /* USART2 */

    /* Configure PA2 as alternate function (AF1 = USART2_TX) */
    GPIOA_MODER &= ~(3 << (2 * 2));     /* Clear mode bits for PA2 */
    GPIOA_MODER |= (2 << (2 * 2));      /* Set to alternate function */

    GPIOA_AFRL &= ~(0xF << (2 * 4));    /* Clear AF bits for PA2 */
    GPIOA_AFRL |= (1 << (2 * 4));       /* Set AF1 */

    /* Configure USART2 */
    USART2_CR1 = 0;                     /* Disable USART */
    USART2_BRR = SYSTEM_CLOCK_HZ / baudrate;  /* Set baud rate */
    USART2_CR1 = USART_CR1_TE | USART_CR1_UE; /* Enable TX and USART */
}

void usart2_putc(char c) {
    /* Wait for transmit buffer to be empty */
    while (!(USART2_ISR & USART_ISR_TXE));

    /* Send character */
    USART2_TDR = c;

    /* Convert LF to CRLF for proper terminal output */
    if (c == '\n') {
        while (!(USART2_ISR & USART_ISR_TXE));
        USART2_TDR = '\r';
    }
}

void usart2_puts(const char *s) {
    while (*s) {
        usart2_putc(*s++);
    }
}
