/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_controller.h"
#include "FreeRTOS.h"
#include "hrms_gpio.h"
#include "hrms_pins.h"
#include "hrms_types.h"
#include "libc_stubs.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>

#define BUTTON_DEBOUNCE_MS 100

char num_buf[12];

// Pre-initialized actuator command template for performance
static hrms_actuator_command_t default_actuator_cmd;

/* -------------------- Public Controller -------------------- */

void hrms_controller_init(void) {
  // Initialize default actuator command template for performance
  memset(&default_actuator_cmd, 0, sizeof(default_actuator_cmd));

  // Set static values once during init
  default_actuator_cmd.led.mode = HRMS_LED_MODE_BLINK;
  default_actuator_cmd.led.toggle_debug_led = false;

  // OLED static configuration
  default_actuator_cmd.oled.icon1 = HRMS_OLED_ICON_NONE;
  default_actuator_cmd.oled.icon2 = HRMS_OLED_ICON_NONE;
  default_actuator_cmd.oled.icon3 = HRMS_OLED_ICON_SMILEY;
  default_actuator_cmd.oled.icon4 = HRMS_OLED_ICON_NONE;
  default_actuator_cmd.oled.invert = 0;
  default_actuator_cmd.oled.progress_percent = 100;

  // Set static text strings
  safe_strncpy(default_actuator_cmd.oled.smalltext1, "JOY",
               HRMS_OLED_MAX_SMALL_TEXT_LEN);
  safe_strncpy(default_actuator_cmd.oled.bigtext, "HERMES 2025",
               HRMS_OLED_MAX_BIG_TEXT_LEN);

  // Communication command defaults
  default_actuator_cmd.comm.should_transmit = false;
  default_actuator_cmd.comm.packet_type = HRMS_COMM_PACKET_CONTROL_CMD;
  default_actuator_cmd.comm.dest_id = 0x02; // Remote receiver ID
}

void hrms_controller_process(const hrms_sensor_data_t *in,
                             hrms_actuator_command_t *out) {
  if (!in || !out)
    return;

  // Copy pre-initialized template for performance (bulk copy of static values)
  *out = default_actuator_cmd;

  // Only update dynamic values
  out->led.blink_speed_ms = 200;

  // Update dynamic OLED text based on joystick button state
  safe_strncpy(out->oled.smalltext2,
               in->joystick.button_pressed ? "BTN" : "---",
               HRMS_OLED_MAX_SMALL_TEXT_LEN);

  // Controller logic: decide if joystick data should be transmitted
  static hrms_joystick_data_t last_joystick = {0};
  static uint32_t last_transmission_request = 0;
  uint32_t now = xTaskGetTickCount();

  // Request transmission if joystick changed significantly or enough time
  // passed
  bool joystick_changed =
      (in->joystick.x_axis != last_joystick.x_axis ||
       in->joystick.y_axis != last_joystick.y_axis ||
       in->joystick.button_pressed != last_joystick.button_pressed);

  bool time_elapsed = (now - last_transmission_request) > pdMS_TO_TICKS(500);

  if (joystick_changed && time_elapsed) {
    out->comm.should_transmit = true;
    out->comm.joystick_data = in->joystick;
    last_joystick = in->joystick;
    last_transmission_request = now;
  } else {
    out->comm.should_transmit = false;
  }
}

void hrms_controller_process_button(const hrms_button_event_t *event,
                                    hrms_actuator_command_t *command) {
  static uint32_t last_press_tick = 0;

  if (!command)
    return;

  // Copy default command template
  *command = default_actuator_cmd;

  if (event && event->event_type == HRMS_BUTTON_EVENT_PRESSED) {
    uint32_t now = xTaskGetTickCount();
    if ((now - last_press_tick) > pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS)) {
      // Immediate LED feedback for real-time response (controller logic)
      hrms_gpio_toggle_pin((uint32_t)HRMS_LED_DEBUG_PORT, HRMS_LED_DEBUG_PIN);
      
      // Set LED command for other potential feedback
      command->led.toggle_debug_led = true;
      
      last_press_tick = now;
    }
  }
}
