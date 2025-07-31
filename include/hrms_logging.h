
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_LOGGING_H
#define HRMS_LOGGING_H

#include <stdarg.h>
#include <stdint.h>

void hrms_logging_init(void);
void hrms_logging_log(const char *format, ...);

#endif // HRMS_LOGGING_H

