/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Hermes.
 *
 * Hermes is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_UTILS_H
#define HRMS_UTILS_H

#include "FreeRTOS.h"
#include "queue.h"
#include "hrms_config.h"
#include <stdbool.h>
#include <stdint.h>

// =============================================================================
// QUEUE UTILITIES - Eliminate repeated queue handling patterns
// =============================================================================

// Safe queue send with timeout and error handling
bool hrms_queue_send_safe(QueueHandle_t queue, const void *item, 
                          uint32_t timeout_ms, const char *queue_name);

// Safe queue receive with timeout
bool hrms_queue_receive_safe(QueueHandle_t queue, void *item, 
                            uint32_t timeout_ms, const char *queue_name);

// =============================================================================
// ERROR HANDLING UTILITIES
// =============================================================================

typedef enum {
    HRMS_ERROR_NONE = 0,
    HRMS_ERROR_QUEUE_FULL,
    HRMS_ERROR_QUEUE_EMPTY,
    HRMS_ERROR_TIMEOUT,
    HRMS_ERROR_INVALID_PARAM,
    HRMS_ERROR_HARDWARE,
    HRMS_ERROR_COMMUNICATION
} hrms_error_t;

// Common error handler
void hrms_handle_error(hrms_error_t error, const char *context);

// =============================================================================
// TIME UTILITIES
// =============================================================================

// Convert milliseconds to ticks (more readable than pdMS_TO_TICKS everywhere)
#define HRMS_MS_TO_TICKS(ms) pdMS_TO_TICKS(ms)

// Get current time in ms
uint32_t hrms_get_time_ms(void);

// Check if timeout elapsed
bool hrms_timeout_elapsed(uint32_t start_time, uint32_t timeout_ms);

// =============================================================================
// MATH UTILITIES
// =============================================================================

// Clamp value to range
int16_t hrms_clamp_i16(int16_t value, int16_t min, int16_t max);
uint16_t hrms_clamp_u16(uint16_t value, uint16_t min, uint16_t max);

// Map value from one range to another (for sensor scaling)
int16_t hrms_map_range(int16_t value, int16_t in_min, int16_t in_max, 
                       int16_t out_min, int16_t out_max);

// Apply deadzone to analog inputs
int16_t hrms_apply_deadzone(int16_t value, int16_t deadzone);

// =============================================================================
// BIT MANIPULATION UTILITIES
// =============================================================================

#define HRMS_SET_BIT(reg, bit)      ((reg) |= (1U << (bit)))
#define HRMS_CLEAR_BIT(reg, bit)    ((reg) &= ~(1U << (bit)))
#define HRMS_TOGGLE_BIT(reg, bit)   ((reg) ^= (1U << (bit)))
#define HRMS_READ_BIT(reg, bit)     (((reg) >> (bit)) & 1U)

// =============================================================================
// VALIDATION UTILITIES
// =============================================================================

// Parameter validation macros
#define HRMS_RETURN_IF_NULL(ptr)    if (!(ptr)) return false
#define HRMS_RETURN_VAL_IF_NULL(ptr, val) if (!(ptr)) return (val)

// Range validation
bool hrms_validate_range_u8(uint8_t value, uint8_t min, uint8_t max);
bool hrms_validate_range_u16(uint16_t value, uint16_t min, uint16_t max);

#endif // HRMS_UTILS_H