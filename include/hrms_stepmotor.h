
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_STEPMOTOR_H
#define HRMS_STEPMOTOR_H

#include <stdint.h>
#include <stdbool.h>
#include "hrms_types.h"

// === API ===
void hrms_stepmotor_init(void);
void hrms_stepmotor_apply(hrms_stepmotor_id_t id, const hrms_stepmotor_command_t *cmd);
void hrms_stepmotor_apply_all(const hrms_stepmotor_command_t cmds[HRMS_STEPMOTOR_COUNT]);

#endif // HRMS_STEPMOTOR_H
