
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */


#ifndef HRMS_ALARM_H
#define HRMS_ALARM_H

#include <stdint.h>
#include <stdbool.h>
#include "hrms_types.h"

void hrms_alarm_init(void);
void hrms_alarm_apply(const hrms_alarm_command_t *cmd);

#endif // HRMS_ALARM_H

