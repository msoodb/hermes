
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_config.h"

#if HRMS_ENABLED_LED

#ifndef HRMS_LED_H
#define HRMS_LED_H

#include "hrms_types.h"

void hrms_led_init(void);
void hrms_led_apply(const hrms_led_command_t *cmd);

#endif /* HRMS_ALARM_H */

#endif /* HRMS_ENABLED_LED */
