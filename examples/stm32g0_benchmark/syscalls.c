/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Minimal syscalls implementation for UART output
 * Copyright 2025 Yann Ramin
 */

#include <sys/stat.h>
#include <errno.h>
#include "usart.h"

extern int errno;

/* Heap management */
extern char _end;
extern char _estack;
static char *heap_ptr = &_end;

void *_sbrk(int incr) {
    char *prev_heap_ptr = heap_ptr;

    if (heap_ptr + incr > &_estack - 1024) {
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_ptr += incr;
    return (void *)prev_heap_ptr;
}

int _write(int file, char *ptr, int len) {
    (void)file;

    for (int i = 0; i < len; i++) {
        usart2_putc(ptr[i]);
    }

    return len;
}

int _close(int file) {
    (void)file;
    return -1;
}

int _fstat(int file, struct stat *st) {
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) {
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir) {
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _read(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

void _exit(int status) {
    (void)status;
    while (1);
}

void _kill(int pid, int sig) {
    (void)pid;
    (void)sig;
}

int _getpid(void) {
    return -1;
}
