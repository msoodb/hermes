
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef LIBC_STUBS_H
#define LIBC_STUBS_H

#include <stddef.h>

void *memset(void *dest, int val, size_t len);
void *memcpy(void *dest, const void *src, size_t len);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
size_t strlen(const char *s);
int abs(int v);
void safe_strncpy(char *dest, const char *src, size_t max_len);
void int_to_string(int value, char *buffer, size_t buffer_size);

// Special init function (does nothing)
void __libc_init_array(void);

#endif
