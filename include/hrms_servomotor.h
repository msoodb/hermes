
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_SERVOMOTOR_H
#define HRMS_SERVOMOTOR_H

#include "hrms_config.h"

#if HRMS_ENABLED_SERVO

#include <stdint.h>
#include "hrms_types.h"

void hrms_servomotor_init(void);
void hrms_servomotor_set_type(uint8_t servo_id, hrms_servo_type_t type);
void hrms_servomotor_apply(uint8_t servo_id, const hrms_servomotor_command_t *cmd);

#endif /* HRMS_ENABLED_SERVO */

#endif /* HRMS_SERVOMOTOR_H */
