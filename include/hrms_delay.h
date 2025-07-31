
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_DELAY_H
#define HRMS_DELAY_H

#include <stdint.h>

void hrms_delay_init(void);
void hrms_delay_ms(uint32_t ms);
void hrms_delay_us(uint32_t us);

#endif // HRMS_DELAY_H

