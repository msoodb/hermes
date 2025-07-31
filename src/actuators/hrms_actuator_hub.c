
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_config.h"

#include "hrms_led.h"

#include "hrms_motor.h"

#include "hrms_display.h"

#include "hrms_oled.h"

#include "hrms_alarm.h"

// Radio communication removed

#include "hrms_servomotor.h"

#include "hrms_actuator_hub.h"
#include "hrms_types.h"

void hrms_actuator_hub_init(void) {
  hrms_led_init();

  hrms_motor_init();

  hrms_display_init();

  hrms_oled_init();

  hrms_servomotor_init();
  // Set servo types for enabled servos only
  hrms_servomotor_set_type(0, HRMS_SERVO_TYPE_PROPORTIONAL);  // Servo1 - PA0
  hrms_servomotor_set_type(1, HRMS_SERVO_TYPE_PROPORTIONAL);  // Servo2 - PA1
  hrms_servomotor_set_type(2, HRMS_SERVO_TYPE_PROPORTIONAL);  // Servo3 - PA2
  hrms_servomotor_set_type(3, HRMS_SERVO_TYPE_PROPORTIONAL);  // Servo4 - PA3

  hrms_alarm_init();

// Radio init removed
}

void hrms_actuator_hub_apply(const hrms_actuator_command_t *cmd) {
  if (!cmd)
    return;

  hrms_led_apply(&cmd->led);

  hrms_motor_apply(&cmd->motor);

  hrms_display_apply(&cmd->display);

  hrms_oled_apply(&cmd->oled);

  // Apply commands to enabled servos only
  hrms_servomotor_apply(0, &cmd->servo1);
  hrms_servomotor_apply(1, &cmd->servo2);
  hrms_servomotor_apply(2, &cmd->servo3);
  hrms_servomotor_apply(3, &cmd->servo4);

  hrms_alarm_apply(&cmd->alarm);

// Radio apply removed
}
