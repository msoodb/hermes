/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_JOYSTICK_H
#define HRMS_JOYSTICK_H

#include <stdint.h>
#include <stdbool.h>
#include "hrms_types.h"

typedef struct {
  bool event_occurred;
  bool button_pressed;
} hrms_joystick_event_t;

void hrms_joystick_init(void);
bool hrms_joystick_read(hrms_joystick_data_t *data);
void hrms_joystick_check_events(hrms_joystick_event_t *event);

#endif /* HRMS_JOYSTICK_H */