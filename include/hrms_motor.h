
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_config.h"

#if BLFM_ENABLED_MOTOR

#ifndef BLFM_MOTOR_H
#define BLFM_MOTOR_H

#include "hrms_types.h"

void hrms_motor_init(void);
void hrms_motor_apply(const hrms_motor_command_t *cmd);

#endif // BLFM_MOTOR_H

#endif /* BLFM_ENABLED_MOTOR */

