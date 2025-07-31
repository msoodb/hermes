
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_config.h"
#include "hrms_sensor_hub.h"

#if HRMS_ENABLED_ULTRASONIC
#include "hrms_ultrasonic.h"
#endif

#if HRMS_ENABLED_POTENTIOMETER
#include "hrms_potentiometer.h"
#endif

#if HRMS_ENABLED_TEMPERATURE
#include "hrms_temperature.h"
#endif

#if HRMS_ENABLED_JOYSTICK
#include "hrms_joystick.h"
#endif

#include <stdbool.h>

void hrms_sensor_hub_init(void) {
#if HRMS_ENABLED_ULTRASONIC
  hrms_ultrasonic_init();
#endif

#if HRMS_ENABLED_POTENTIOMETER
  hrms_potentiometer_init();
#endif

#if HRMS_ENABLED_TEMPERATURE
  hrms_temperature_init();
#endif

#if HRMS_ENABLED_JOYSTICK
  hrms_joystick_init();
#endif
}

bool hrms_sensor_hub_read(hrms_sensor_data_t *out) {
  if (!out) {
    return false;
  }

  bool ok = true;

#if HRMS_ENABLED_ULTRASONIC
  ok &= hrms_ultrasonic_read(&out->ultrasonic);
#endif

#if HRMS_ENABLED_POTENTIOMETER
  ok &= hrms_potentiometer_read(&out->potentiometer);
#endif

#if HRMS_ENABLED_TEMPERATURE
  ok &= hrms_temperature_read(&out->temperature);
#endif

#if HRMS_ENABLED_JOYSTICK
  hrms_joystick_read(&out->joystick);  // Always succeeds, no need to check ok
#else
  // Initialize joystick data to zero when disabled
  out->joystick.x_axis = 0;
  out->joystick.y_axis = 0;
  out->joystick.button_pressed = false;
#endif

  return ok;
}
