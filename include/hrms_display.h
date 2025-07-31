/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_config.h"

#if BLFM_ENABLED_DISPLAY

#ifndef BLFM_DISPLAY_H
#define BLFM_DISPLAY_H

#include "hrms_types.h"
#include <stdint.h>

void test_single_pin_on_port(void);
void test_all_pins_on_port(void);
void hrms_display_init(void);
void hrms_display_startup_sequence(void);
void hrms_display_apply(const hrms_display_command_t *cmd);

#endif // BLFM_DISPLAY_H

#endif /* BLFM_ENABLED_DISPLAY */
