/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Simple USART2 driver for STM32G0
 * Copyright 2025 Yann Ramin
 */

#ifndef USART_H
#define USART_H

#include <stdint.h>

void usart2_init(uint32_t baudrate);
void usart2_putc(char c);
void usart2_puts(const char *s);

#endif
