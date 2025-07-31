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
#include "hrms_config.h"
#include "hrms_gpio.h"
#include "hrms_pins.h"
#include "hrms_state.h"
#include "hrms_types.h"
#include "task.h"
#include "libc_stubs.h"
#include <stdbool.h>
#include <stdint.h>

#define LCD_CYCLE_COUNT 50
#define MODE_BUTTON_DEBOUNCE_MS 100

static int lcd_mode = 0;
static int lcd_counter = 0;

char num_buf[12];

static hrms_system_state_t hrms_system_state = {
    .current_mode = HRMS_MODE_MANUAL, .motion_state = HRMS_MOTION_STOP};

/* -------------------- Utilities -------------------- */

static int pseudo_random(int min, int max) {
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
}

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

  char buf1[17];
  char num_buf[12];

  lcd_counter++;
  if (lcd_counter >= LCD_CYCLE_COUNT) {
    lcd_counter = 0;
    lcd_mode = (lcd_mode + 1) % 3;
  }

  if (lcd_mode == 0) {
    strcpy(buf1, "Temp: ");
    uint_to_str(num_buf, in->temperature.temperature_mc / 1000);
    strcat(buf1, num_buf);
    strcat(buf1, " C");
  } else if (lcd_mode == 1) {
    strcpy(buf1, "Speed: ");
    // uint_to_str(num_buf, in->imu.speed_cm_s);
    strcat(buf1, num_buf);
    strcat(buf1, " cm/s");
  } else {
    strcpy(buf1, "Temp: ");
    uint_to_str(num_buf, in->temperature.temperature_mc);
    strcat(buf1, num_buf);
    strcat(buf1, " C");
  }

  strcpy(out->display.line1, buf1);
  strcpy(out->display.line2, num_buf);

  out->oled.icon1 = HRMS_OLED_ICON_NONE;
  out->oled.icon2 = HRMS_OLED_ICON_NONE;
  out->oled.icon3 = HRMS_OLED_ICON_SMILEY;
  out->oled.icon4 = HRMS_OLED_ICON_NONE;

  safe_strncpy(out->oled.smalltext1, "Pass", HRMS_OLED_MAX_SMALL_TEXT_LEN);
  safe_strncpy(out->oled.bigtext, "BELFHYM 2025", HRMS_OLED_MAX_BIG_TEXT_LEN);
  safe_strncpy(out->oled.smalltext2, "V1", HRMS_OLED_MAX_SMALL_TEXT_LEN);

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

// ESP32 communication module removed - ready for new implementation
