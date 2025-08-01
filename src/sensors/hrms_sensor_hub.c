
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_sensor_hub.h"
#include "hrms_potentiometer.h"
#include "hrms_joystick.h"
#include <stdbool.h>

void hrms_sensor_hub_init(void) {
  hrms_potentiometer_init();
  hrms_joystick_init();
}

bool hrms_sensor_hub_read(hrms_sensor_data_t *out) {
  if (!out) {
    return false;
  }

  bool ok = true;

  ok &= hrms_potentiometer_read(&out->potentiometer);
  ok &= hrms_joystick_read(&out->joystick);
  return ok;
}
