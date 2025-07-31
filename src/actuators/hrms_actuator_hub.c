
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_led.h"

#include "hrms_oled.h"

#include "hrms_alarm.h"

// Radio communication removed


#include "hrms_actuator_hub.h"
#include "hrms_types.h"

void hrms_actuator_hub_init(void) {
  hrms_led_init();

  hrms_oled_init();

  hrms_alarm_init();

// Radio init removed
}

void hrms_actuator_hub_apply(const hrms_actuator_command_t *cmd) {
  if (!cmd)
    return;

  hrms_led_apply(&cmd->led);

  hrms_oled_apply(&cmd->oled);

  hrms_alarm_apply(&cmd->alarm);

// Radio apply removed
}
