/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_controller.h"
#include "ORION_Config.h"
#include "FreeRTOS.h"
#include "hrms_gpio.h"
#include "hrms_pins.h"
#include "hrms_types.h"
#include "hrms_communication_hub.h"
#include "task.h"
#include "libc_stubs.h"
#include <stdbool.h>
#include <stdint.h>
#include "orion.h"

#define BUTTON_DEBOUNCE_MS 100

char num_buf[12];

// Pre-initialized actuator command template for performance
static hrms_actuator_command_t default_actuator_cmd;

/* -------------------- Public Controller -------------------- */

void hrms_controller_init(void) {
  // Initialize default actuator command template for performance
  memset(&default_actuator_cmd, 0, sizeof(default_actuator_cmd));
  
  // Set static values once during init
  default_actuator_cmd.alarm.active = false;
  default_actuator_cmd.led.mode = HRMS_LED_MODE_BLINK;
  
  // OLED static configuration
  default_actuator_cmd.oled.icon1 = HRMS_OLED_ICON_NONE;
  default_actuator_cmd.oled.icon2 = HRMS_OLED_ICON_NONE;
  default_actuator_cmd.oled.icon3 = HRMS_OLED_ICON_SMILEY;
  default_actuator_cmd.oled.icon4 = HRMS_OLED_ICON_NONE;
  default_actuator_cmd.oled.invert = 0;
  default_actuator_cmd.oled.progress_percent = 100;
  
  // Set static text strings
  safe_strncpy(default_actuator_cmd.oled.smalltext1, "JOY", HRMS_OLED_MAX_SMALL_TEXT_LEN);
  safe_strncpy(default_actuator_cmd.oled.bigtext, "HERMES 2025", HRMS_OLED_MAX_BIG_TEXT_LEN);
}


void hrms_controller_process(const hrms_sensor_data_t *in,
                             hrms_actuator_command_t *out) {
  if (!in || !out)
    return;

  // Copy pre-initialized template for performance (bulk copy of static values)
  *out = default_actuator_cmd;

  // Only update dynamic values
  out->led.blink_speed_ms = in->potentiometer.raw_value;
  
  // Update dynamic OLED text based on joystick button state
  safe_strncpy(out->oled.smalltext2, in->joystick.button_pressed ? "BTN" : "---", HRMS_OLED_MAX_SMALL_TEXT_LEN);

  // Send encrypted joystick data via radio
  hrms_controller_send_joystick_command(&in->joystick);
}

void hrms_controller_process_button(const hrms_button_event_t *event,
                                         hrms_actuator_command_t *command) {
  static uint32_t last_press_tick = 0;
  (void)command;

  if (event && event->event_type == HRMS_BUTTON_EVENT_PRESSED) {
    uint32_t now = xTaskGetTickCount();
    if ((now - last_press_tick) > pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS)) {
      hrms_gpio_toggle_pin((uint32_t)HRMS_LED_DEBUG_PORT, HRMS_LED_DEBUG_PIN);
      last_press_tick = now;
    }
  }
}


void hrms_controller_send_joystick_command(const hrms_joystick_data_t *joystick_data) {
  if (!joystick_data) return;
  
  static uint32_t last_transmission = 0;
  uint32_t now = xTaskGetTickCount();
  
  // Limit transmission rate to prevent flooding - send every 500ms max
  if ((now - last_transmission) < pdMS_TO_TICKS(500)) {
    return;
  }
  
  // Only send if joystick has moved significantly or button state changed
  static hrms_joystick_data_t last_sent = {0};
  if (joystick_data->x_axis == last_sent.x_axis && 
      joystick_data->y_axis == last_sent.y_axis &&
      joystick_data->button_pressed == last_sent.button_pressed) {
    return;
  }
  
  hrms_comm_packet_t packet;
  memset(&packet, 0, sizeof(packet));
  
  packet.packet_id = (uint8_t)(now % 255) + 1;
  packet.packet_type = HRMS_COMM_PACKET_CONTROL_CMD;
  packet.source_id = 0x01; // Hermes controller ID
  packet.dest_id = 0x02;   // Remote receiver ID
  packet.timestamp = now;
  
  // Prepare joystick data for encryption
  uint8_t plaintext[sizeof(hrms_joystick_data_t)];
  memcpy(plaintext, joystick_data, sizeof(hrms_joystick_data_t));
  
  // Encrypt the joystick data using ORION
  uint8_t encrypted_data[HRMS_COMM_MAX_PAYLOAD_SIZE];
  size_t encrypted_len = 0;
  
  // Always encrypt - no plaintext fallback
  if (ORION_Encrypt(plaintext, sizeof(hrms_joystick_data_t), encrypted_data, &encrypted_len) == 0) {
    packet.payload_size = (uint8_t)encrypted_len;
    if (encrypted_len <= HRMS_COMM_MAX_PAYLOAD_SIZE) {
      memcpy(packet.payload, encrypted_data, encrypted_len);
    }
  } else {
    // Encryption failed - abort transmission for security
    return;
  }
  
  hrms_communication_hub_set_checksum(&packet);
  
  bool transmission_success = hrms_communication_hub_send(&packet);
  if (transmission_success) {
    last_transmission = now;
    last_sent = *joystick_data;
    
    // Flash debug LED on successful transmission for visual feedback
    hrms_gpio_toggle_pin((uint32_t)HRMS_LED_DEBUG_PORT, HRMS_LED_DEBUG_PIN);
    
    // Also flash onboard LED to show radio activity
    hrms_gpio_toggle_pin((uint32_t)HRMS_LED_ONBOARD_PORT, HRMS_LED_ONBOARD_PIN);
  } else {
    // Transmission failed - turn off onboard LED
    hrms_gpio_clear_pin((uint32_t)HRMS_LED_ONBOARD_PORT, HRMS_LED_ONBOARD_PIN);
  }
}
