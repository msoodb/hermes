
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_config.h"

#if HRMS_ENABLED_ULTRASONIC

#ifndef HRMS_ULTRASONIC_H
#define HRMS_ULTRASONIC_H

#include "hrms_types.h"
#include <stdbool.h>

void hrms_ultrasonic_init(void);
bool hrms_ultrasonic_read(hrms_ultrasonic_data_t *data);

#endif // HRMS_ULTRASONIC_H

#endif /* HRMS_ENABLED_ULTRASONIC */

