
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "libc_stubs.h"

// Minimal memset implementation
void *memset(void *dest, int val, size_t len) {
  unsigned char *ptr = dest;
  while (len-- > 0) {
    *ptr++ = (unsigned char)val;
  }
  return dest;
}

// Minimal memcpy implementation
void *memcpy(void *dest, const void *src, size_t len) {
  unsigned char *d = dest;
  const unsigned char *s = src;
  while (len-- > 0) {
    *d++ = *s++;
  }
  return dest;
}

// Minimal strcpy implementation (assumes dest is large enough)
char *strcpy(char *dest, const char *src) {
  char *d = dest;
  while ((*d++ = *src++))
    ;
  return dest;
}

// Minimal __libc_init_array (does nothing)
void __libc_init_array(void) {
  // No global/static C++ constructors used, safe to leave empty.
}

size_t strlen(const char *s) {
  size_t len = 0;
  while (s[len])
    ++len;
  return len;
}

// Minimal strcat implementation
char *strcat(char *dest, const char *src) {
  char *d = dest + strlen(dest);
  while ((*d++ = *src++))
    ;
  return dest;
}

// Minimal abs implementation
int abs(int v) {
  return v < 0 ? -v : v;
}

void safe_strncpy(char *dest, const char *src, size_t max_len) {
  if (max_len == 0) return;
  size_t src_len = strlen(src);
  size_t copy_len = (src_len < max_len) ? src_len : max_len - 1;
  memcpy(dest, src, copy_len);
  dest[copy_len] = '\0';  // Ensure null termination
}

// Simple integer to string conversion for embedded use
void int_to_string(int value, char *buffer, size_t buffer_size) {
  if (buffer_size < 2) return; // Need at least space for '0' and '\0'
  
  char *ptr = buffer;
  
  // Handle negative numbers
  if (value < 0) {
    value = -value;
    *ptr++ = '-';
    buffer_size--;
  }
  
  // Handle zero case
  if (value == 0) {
    *ptr++ = '0';
    *ptr = '\0';
    return;
  }
  
  // Convert digits (reverse order first)
  char temp[12]; // Enough for 32-bit int
  size_t temp_len = 0;
  
  while (value > 0 && temp_len < sizeof(temp) - 1) {
    temp[temp_len++] = '0' + (value % 10);
    value /= 10;
  }
  
  // Copy digits in correct order
  for (size_t i = temp_len; i > 0 && buffer_size > 1; i--) {
    *ptr++ = temp[i - 1];
    buffer_size--;
  }
  
  *ptr = '\0';
}
