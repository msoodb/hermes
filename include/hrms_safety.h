
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_SAFETY_H
#define HRMS_SAFETY_H

#include <stdint.h>

void hrms_safety_init(void);
void hrms_safety_check(void);
int hrms_safety_get_status(void);

#endif // HRMS_SAFETY_H

