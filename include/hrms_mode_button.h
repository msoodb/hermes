
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_config.h"

#if BLFM_ENABLED_MODE_BUTTON

#ifndef BLFM_MODE_BUTTON_H
#define BLFM_MODE_BUTTON_H

#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>
#include "stm32f1xx.h"
#include "hrms_types.h"
#include "hrms_pins.h"

void hrms_mode_button_init(QueueHandle_t controller_queue);

#endif // BLFM_MODE_BUTTON_H

#endif /* BLFM_ENABLED_MODE_BUTTON */
