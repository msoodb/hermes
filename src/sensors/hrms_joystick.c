/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_config.h"

#include "hrms_joystick.h"
#include "hrms_pins.h"
#include "hrms_gpio.h"
#include "hrms_adc.h"
#include "libc_stubs.h"

// Calibration values - adjust based on your joystick
#define JOYSTICK_CENTER_VALUE 2048  // ADC center value (12-bit ADC: 0-4095)
#define JOYSTICK_DEADZONE 600       // Very large deadzone to prevent servo shaking
#define JOYSTICK_MAX_VALUE 4095     // Maximum ADC value
#define JOYSTICK_UPDATE_THRESHOLD 100  // Only update if change is significant

static bool last_button_state = false;
static int16_t last_x_axis = 0;
static int16_t last_y_axis = 0;

void hrms_joystick_init(void) {
  // Configure analog pins for VRX and VRY
  hrms_gpio_config_analog((uint32_t)HRMS_JOYSTICK_VRX_PORT, HRMS_JOYSTICK_VRX_PIN);
  hrms_gpio_config_analog((uint32_t)HRMS_JOYSTICK_VRY_PORT, HRMS_JOYSTICK_VRY_PIN);
  
  // Configure digital pin for SW button (with pull-up)
  hrms_gpio_config_input_pullup((uint32_t)HRMS_JOYSTICK_SW_PORT, HRMS_JOYSTICK_SW_PIN);
}

void hrms_joystick_read(hrms_joystick_data_t *data) {
  if (!data) return;
  
  // Read ADC values
  uint16_t vrx_raw, vry_raw;
  if (hrms_adc_read(HRMS_JOYSTICK_VRX_ADC_CHANNEL, &vrx_raw) != 0) {
    vrx_raw = JOYSTICK_CENTER_VALUE;  // Default to center if read fails
  }
  if (hrms_adc_read(HRMS_JOYSTICK_VRY_ADC_CHANNEL, &vry_raw) != 0) {
    vry_raw = JOYSTICK_CENTER_VALUE;  // Default to center if read fails
  }
  
  // Convert to -1000 to +1000 range with deadzone
  int16_t x_centered = (int16_t)vrx_raw - JOYSTICK_CENTER_VALUE;
  int16_t y_centered = (int16_t)vry_raw - JOYSTICK_CENTER_VALUE;
  
  // Apply deadzone
  if (x_centered > -JOYSTICK_DEADZONE && x_centered < JOYSTICK_DEADZONE) {
    x_centered = 0;
  }
  if (y_centered > -JOYSTICK_DEADZONE && y_centered < JOYSTICK_DEADZONE) {
    y_centered = 0;
  }
  
  // Scale to -1000 to +1000 range
  int16_t new_x = (x_centered * 1000) / (JOYSTICK_CENTER_VALUE - JOYSTICK_DEADZONE);
  int16_t new_y = (y_centered * 1000) / (JOYSTICK_CENTER_VALUE - JOYSTICK_DEADZONE);
  
  // Clamp values
  if (new_x > 1000) new_x = 1000;
  if (new_x < -1000) new_x = -1000;
  if (new_y > 1000) new_y = 1000;
  if (new_y < -1000) new_y = -1000;
  
  // Aggressive filtering - only update if change is very significant
  if (abs(new_x - last_x_axis) > JOYSTICK_UPDATE_THRESHOLD) {
    last_x_axis = new_x;
  }
  if (abs(new_y - last_y_axis) > JOYSTICK_UPDATE_THRESHOLD) {
    last_y_axis = new_y;
  }
  
  data->x_axis = last_x_axis;
  data->y_axis = last_y_axis;
  
  // Read button state (active low - pressed = 0)
  data->button_pressed = !hrms_gpio_read_pin((uint32_t)HRMS_JOYSTICK_SW_PORT, HRMS_JOYSTICK_SW_PIN);
}

void hrms_joystick_check_events(hrms_joystick_event_t *event) {
  if (!event) return;
  
  hrms_joystick_data_t data;
  hrms_joystick_read(&data);
  
  // Check for button press event (rising edge)
  bool current_button = data.button_pressed;
  event->event_occurred = (current_button && !last_button_state);
  event->button_pressed = current_button;
  
  last_button_state = current_button;
}

