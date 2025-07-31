/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_EXTI_DISPATCHER_H
#define HRMS_EXTI_DISPATCHER_H

#include <stdint.h>

typedef void (*hrms_exti_callback_t)(void);

void hrms_exti_register_callback(uint8_t exti_line, hrms_exti_callback_t callback);

#endif
