
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_config.h"

#if BLFM_ENABLED_TEMPERATURE

#ifndef BLFM_TEMPERATURE_H
#define BLFM_TEMPERATURE_H

#include <stdbool.h>
#include "hrms_types.h"

void hrms_temperature_init(void);
bool hrms_temperature_read(hrms_temperature_data_t *temp);

#endif // BLFM_TEMPERATURE_H

#endif /* BLFM_ENABLED_TEMPERATURE */
