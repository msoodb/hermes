
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_config.h"

#if BLFM_ENABLED_ALARM

#ifndef BLFM_ALARM_H
#define BLFM_ALARM_H

#include <stdint.h>
#include <stdbool.h>
#include "hrms_types.h"

void hrms_alarm_init(void);
void hrms_alarm_apply(const hrms_alarm_command_t *cmd);

#endif // BLFM_ALARM_H

#endif /* BLFM_ENABLED_ALARM */

