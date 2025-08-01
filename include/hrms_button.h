
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_MODE_BUTTON_H
#define HRMS_MODE_BUTTON_H

#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>
#include "stm32f1xx.h"
#include "hrms_types.h"
#include "hrms_pins.h"

void hrms_button_init(QueueHandle_t controller_queue);

#endif // HRMS_MODE_BUTTON_H
