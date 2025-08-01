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
#include "hrms_state.h"
#include "hrms_types.h"
#include "hrms_communication_hub.h"
#include "task.h"
#include "libc_stubs.h"
#include <stdbool.h>
#include <stdint.h>

#if ORION_INTEGRATION_ENABLED
#include "orion.h"
#endif

#define MODE_BUTTON_DEBOUNCE_MS 100

char num_buf[12];

static hrms_system_state_t hrms_system_state = {
    .current_mode = HRMS_MODE_MANUAL, .motion_state = HRMS_MOTION_STOP};

/* -------------------- Utilities -------------------- */

/*static int pseudo_random(int min, int max) {
  static uint32_t seed = 123456789;
  seed = seed * 1664525 + 1013904223;
  return min + (seed % (max - min + 1));
}

static void uint_to_str(char *buf, uint16_t value) {
  if (value >= 100) {
    buf[0] = '0' + (value / 100) % 10;
    buf[1] = '0' + (value / 10) % 10;
    buf[2] = '0' + value % 10;
    buf[3] = '\0';
  } else if (value >= 10) {
    buf[0] = '0' + (value / 10) % 10;
    buf[1] = '0' + value % 10;
    buf[2] = '\0';
  } else {
    buf[0] = '0' + value;
    buf[1] = '\0';
  }
  }*/

/* -------------------- Public Controller -------------------- */

void hrms_controller_init(void) {
  // Reserved for future initialization
}

// --- Mode-setting functions ---
static void hrms_set_mode_auto(hrms_actuator_command_t *out) {
  hrms_system_state.current_mode = HRMS_MODE_AUTO;
  out->led.mode = HRMS_LED_MODE_BLINK;
  out->led.blink_speed_ms = 250;
}

static void hrms_set_mode_manual(hrms_actuator_command_t *out) {
  hrms_system_state.current_mode = HRMS_MODE_MANUAL;
  out->led.mode = HRMS_LED_MODE_OFF;
}

static void hrms_set_mode_emergency(hrms_actuator_command_t *out) {
  hrms_system_state.current_mode = HRMS_MODE_EMERGENCY;
  out->led.mode = HRMS_LED_MODE_ON;
}

// --- Public interface to change mode ---
void hrms_controller_change_mode(hrms_mode_t mode,
                                 hrms_actuator_command_t *out) {
  switch (mode) {
  case HRMS_MODE_AUTO:
    hrms_set_mode_auto(out);
    break;
  case HRMS_MODE_MANUAL:
    hrms_set_mode_manual(out);
    break;
  case HRMS_MODE_EMERGENCY:
  default:
    hrms_set_mode_emergency(out);
    break;
  }

  hrms_system_state.motion_state = HRMS_MOTION_STOP;
}

void hrms_controller_process(const hrms_sensor_data_t *in,
                             hrms_actuator_command_t *out) {
  if (!in || !out)
    return;

  uint16_t led_blink_speed = 100;
  hrms_led_mode_t led_mode = HRMS_LED_MODE_BLINK;

  // Ultrasonic removed - disable distance-based alarm
  out->alarm.active = false;

  uint16_t pot_val = in->potentiometer.raw_value;

  led_mode = HRMS_LED_MODE_BLINK;
  led_blink_speed = pot_val; // + (pot_val * (1500 - 200)) / 4095;

  out->led.mode = led_mode;
  out->led.blink_speed_ms = led_blink_speed;

  // Send encrypted joystick data via radio
  hrms_controller_send_joystick_command(&in->joystick);

  out->oled.icon1 = HRMS_OLED_ICON_NONE;
  out->oled.icon2 = HRMS_OLED_ICON_NONE;
  out->oled.icon3 = HRMS_OLED_ICON_SMILEY;
  out->oled.icon4 = HRMS_OLED_ICON_NONE;

  safe_strncpy(out->oled.smalltext1, "JOY", HRMS_OLED_MAX_SMALL_TEXT_LEN);
  safe_strncpy(out->oled.bigtext, "HERMES 2025", HRMS_OLED_MAX_BIG_TEXT_LEN);
  safe_strncpy(out->oled.smalltext2, in->joystick.button_pressed ? "BTN" : "---", HRMS_OLED_MAX_SMALL_TEXT_LEN);

  out->oled.invert = 0;
  out->oled.progress_percent = 100;
}

void hrms_controller_process_mode_button(const hrms_mode_button_event_t *event,
                                         hrms_actuator_command_t *command) {
  static uint32_t last_press_tick = 0;
  (void)command;

  if (event && event->event_type == HRMS_MODE_BUTTON_EVENT_PRESSED) {
    uint32_t now = xTaskGetTickCount();
    if ((now - last_press_tick) > pdMS_TO_TICKS(MODE_BUTTON_DEBOUNCE_MS)) {
      hrms_gpio_toggle_pin((uint32_t)HRMS_LED_DEBUG_PORT, HRMS_LED_DEBUG_PIN);
      last_press_tick = now;
    }
  }
}

void hrms_controller_process_bigsound(const hrms_bigsound_event_t *event,
                                      hrms_actuator_command_t *out) {
  if (!event || !out)
    return;
  // You can implement this or leave empty
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
  
#if ORION_INTEGRATION_ENABLED
  // Encrypt the joystick data using ORION
  uint8_t encrypted_data[HRMS_COMM_MAX_PAYLOAD_SIZE];
  size_t encrypted_len = 0;
  
  if (ORION_Encrypt(plaintext, sizeof(hrms_joystick_data_t), encrypted_data, &encrypted_len) == 0) {
    // Successfully encrypted - use encrypted data
    packet.payload_size = (uint8_t)encrypted_len;
    if (encrypted_len <= HRMS_COMM_MAX_PAYLOAD_SIZE) {
      memcpy(packet.payload, encrypted_data, encrypted_len);
    }
  } else {
    // Encryption failed - fall back to plaintext (for debugging)
    packet.payload_size = sizeof(hrms_joystick_data_t);
    memcpy(packet.payload, plaintext, sizeof(hrms_joystick_data_t));
  }
#else
  // No encryption - send plaintext
  packet.payload_size = sizeof(hrms_joystick_data_t);
  memcpy(packet.payload, plaintext, sizeof(hrms_joystick_data_t));
#endif
  
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

// ESP32 communication module removed - ready for new implementation
