
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_PWM_H
#define HRMS_PWM_H

#include <stdint.h>
#include "hrms_config.h"

#define HRMS_PWM_MAX_CHANNELS 4

void hrms_pwm_init(void);
void hrms_pwm_set_pulse_us(uint8_t channel, uint16_t us);
void hrms_pwm_generate_cycle(void);

#endif // HRMS_PWM_H

