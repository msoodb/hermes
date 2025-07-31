
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_IMU_H
#define HRMS_IMU_H

#include "hrms_types.h"
#include <stdbool.h>

void hrms_imu_init(void);
bool hrms_imu_read(hrms_imu_data_t *data);

#endif // HRMS_IMU_H

