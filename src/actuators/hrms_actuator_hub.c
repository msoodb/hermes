
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_config.h"

#if HRMS_ENABLED_LED
#include "hrms_led.h"
#endif

#if HRMS_ENABLED_MOTOR
#include "hrms_motor.h"
#endif

#if HRMS_ENABLED_DISPLAY
#include "hrms_display.h"
#endif

#if HRMS_ENABLED_OLED
#include "hrms_oled.h"
#endif

#if HRMS_ENABLED_ALARM
#include "hrms_alarm.h"
#endif

// Radio communication removed

#if HRMS_ENABLED_SERVO
#include "hrms_servomotor.h"
#endif

#include "hrms_actuator_hub.h"
#include "hrms_types.h"

void hrms_actuator_hub_init(void) {
#if HRMS_ENABLED_LED
  hrms_led_init();
#endif

#if HRMS_ENABLED_MOTOR
  hrms_motor_init();
#endif

#if HRMS_ENABLED_DISPLAY
  hrms_display_init();
#endif

#if HRMS_ENABLED_OLED
  hrms_oled_init();
#endif

#if HRMS_ENABLED_SERVO
  hrms_servomotor_init();
  // Set servo types for enabled servos only
#if HRMS_ENABLED_SERVO1
  hrms_servomotor_set_type(0, HRMS_SERVO_TYPE_PROPORTIONAL);  // Servo1 - PA0
#endif
#if HRMS_ENABLED_SERVO2
  hrms_servomotor_set_type(1, HRMS_SERVO_TYPE_PROPORTIONAL);  // Servo2 - PA1
#endif
#if HRMS_ENABLED_SERVO3
  hrms_servomotor_set_type(2, HRMS_SERVO_TYPE_PROPORTIONAL);  // Servo3 - PA2
#endif
#if HRMS_ENABLED_SERVO4
  hrms_servomotor_set_type(3, HRMS_SERVO_TYPE_PROPORTIONAL);  // Servo4 - PA3
#endif
#endif

#if HRMS_ENABLED_ALARM
  hrms_alarm_init();
#endif

// Radio init removed
}

void hrms_actuator_hub_apply(const hrms_actuator_command_t *cmd) {
  if (!cmd)
    return;

#if HRMS_ENABLED_LED
  hrms_led_apply(&cmd->led);
#endif

#if HRMS_ENABLED_MOTOR
  hrms_motor_apply(&cmd->motor);
#endif

#if HRMS_ENABLED_DISPLAY
  hrms_display_apply(&cmd->display);
#endif

#if HRMS_ENABLED_OLED
  hrms_oled_apply(&cmd->oled);
#endif

#if HRMS_ENABLED_SERVO
  // Apply commands to enabled servos only
#if HRMS_ENABLED_SERVO1
  hrms_servomotor_apply(0, &cmd->servo1);
#endif
#if HRMS_ENABLED_SERVO2
  hrms_servomotor_apply(1, &cmd->servo2);
#endif
#if HRMS_ENABLED_SERVO3
  hrms_servomotor_apply(2, &cmd->servo3);
#endif
#if HRMS_ENABLED_SERVO4
  hrms_servomotor_apply(3, &cmd->servo4);
#endif
#endif

#if HRMS_ENABLED_ALARM
  hrms_alarm_apply(&cmd->alarm);
#endif

// Radio apply removed
}
