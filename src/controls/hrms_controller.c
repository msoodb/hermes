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
#define SWEEP_MIN_ANGLE 0
#define SWEEP_MAX_ANGLE 180

#define MOTOR_BACKWARD_TICKS_MAX 2
#define MOTOR_MIN_ROTATE_TICKS 4
#define MOTOR_MAX_ROTATE_TICKS 6
#define MODE_BUTTON_DEBOUNCE_MS 100
#define IR_CONTROL_TIMEOUT_MS 200

#define MOTOR_DEFAULT_SPEED 255

static int lcd_mode = 0;
static int lcd_counter = 0;


static int motor_backward_ticks = 0;
static int motor_rotate_ticks = 0;
static int motor_rotate_duration = 0;

char num_buf[12];

static hrms_system_state_t hrms_system_state = {
    .current_mode = HRMS_MODE_MANUAL, .motion_state = HRMS_MOTION_STOP};

/**
 * Helper: set motor motion from angle (-180 to 180) and speed.
 */
static void set_motor_motion_by_angle(int angle, int speed,
                                      hrms_motor_command_t *out);

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

/* -------------------- Motion Helpers -------------------- */
static int motion_state_to_angle(hrms_motion_state_t motion) {
  switch (motion) {
  case HRMS_MOTION_FORWARD:
  case HRMS_MOTION_STOP:
    return 0;
  case HRMS_MOTION_BACKWARD:
    return 180;
  case HRMS_MOTION_ROTATE_RIGHT:
    return 90;
  case HRMS_MOTION_ROTATE_LEFT:
    return -90;
  default:
    return 0;
  }
}

/**
 * Map angle (-180 to 180) and speed (0-255) to motor command.
 * Sets motor left/right direction and speed.
 *
 * No floating point used.
 */
static void set_motor_motion_by_angle(int angle, int speed,
                                      hrms_motor_command_t *out) {
  if (!out)
    return;

  // Normalize angle to -180..180
  while (angle > 180)
    angle -= 360;
  while (angle < -180)
    angle += 360;

  // Map angle to differential control
  // Example simple approach:
  // -90 = full left spin
  // +90 = full right spin
  // 0 = straight
  // ±180 = backward

  if (angle > 90) {
    // Hard right spin
    out->left.direction = HRMS_MOTION_BACKWARD;
    out->right.direction = HRMS_MOTION_BACKWARD;
    out->left.speed = speed;
    out->right.speed = speed;
  } else if (angle < -90) {
    // Hard left spin
    out->left.direction = HRMS_MOTION_BACKWARD;
    out->right.direction = HRMS_MOTION_BACKWARD;
    out->left.speed = speed;
    out->right.speed = speed;
  } else if (angle > 45) {
    // Right turn
    out->left.direction = HRMS_MOTION_FORWARD;
    out->right.direction = HRMS_MOTION_BACKWARD;
    out->left.speed = speed;
    out->right.speed = speed;
  } else if (angle < -45) {
    // Left turn
    out->left.direction = HRMS_MOTION_BACKWARD;
    out->right.direction = HRMS_MOTION_FORWARD;
    out->left.speed = speed;
    out->right.speed = speed;
  } else if (angle > -45 && angle < 45) {
    // Forward
    out->left.direction = HRMS_MOTION_FORWARD;
    out->right.direction = HRMS_MOTION_FORWARD;
    out->left.speed = speed;
    out->right.speed = speed;
  } else {
    // fallback stop
    out->left.direction = HRMS_MOTION_FORWARD;
    out->right.direction = HRMS_MOTION_FORWARD;
    out->left.speed = 0;
    out->right.speed = 0;
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

  set_motor_motion_by_angle(0, 0, &out->motor);
  hrms_system_state.motion_state = HRMS_MOTION_STOP;
}

void hrms_controller_process(const hrms_sensor_data_t *in,
                             hrms_actuator_command_t *out) {
  if (!in || !out)
    return;

  uint16_t led_blink_speed = 100;
  hrms_led_mode_t led_mode = HRMS_LED_MODE_BLINK;

  if (hrms_system_state.current_mode == HRMS_MODE_AUTO) {
    switch (hrms_system_state.motion_state) {
    case HRMS_MOTION_STOP:
    case HRMS_MOTION_FORWARD:
      // Ultrasonic removed - always move forward
      set_motor_motion_by_angle(0, MOTOR_DEFAULT_SPEED, &out->motor);
      break;

    case HRMS_MOTION_BACKWARD:
      set_motor_motion_by_angle(180, MOTOR_DEFAULT_SPEED, &out->motor);
      motor_backward_ticks++;
      if (motor_backward_ticks >= MOTOR_BACKWARD_TICKS_MAX) {
        motor_rotate_ticks = 0;
        motor_rotate_duration =
            pseudo_random(MOTOR_MIN_ROTATE_TICKS, MOTOR_MAX_ROTATE_TICKS);
        hrms_system_state.motion_state = HRMS_MOTION_ROTATE_LEFT;
      }
      break;

    case HRMS_MOTION_ROTATE_LEFT:
      set_motor_motion_by_angle(-90, MOTOR_DEFAULT_SPEED, &out->motor);
      motor_rotate_ticks++;
      if (motor_rotate_ticks >= motor_rotate_duration) {
        hrms_system_state.motion_state = HRMS_MOTION_FORWARD;
      }
      break;

    default:
      set_motor_motion_by_angle(0, 0, &out->motor);
      break;
    }
  }

  // Ultrasonic removed - disable distance-based alarm
  out->alarm.active = false;

  uint16_t pot_val = in->potentiometer.raw_value;

  led_mode = HRMS_LED_MODE_BLINK;
  led_blink_speed = pot_val; // + (pot_val * (1500 - 200)) / 4095;

  out->led.mode = led_mode;
  out->led.blink_speed_ms = led_blink_speed;

// Initialize servos to center position by default
  out->servo1.proportional_input = 0;  // Center position
  out->servo2.proportional_input = 0;  // Center position  
  out->servo3.proportional_input = 0;  // Center position
  out->servo4.proportional_input = 0;  // Center position

  // Process joystick for servo control - all servos move together
  if (hrms_system_state.current_mode == HRMS_MODE_MANUAL) {
    // Use Y-axis for all servos together (like *, 0, # keys)
    // Only update if joystick moved significantly from center
    if (abs(in->joystick.y_axis) > 100) {  // Deadzone to prevent shaking
      out->servo1.proportional_input = in->joystick.y_axis;  // -1000 to +1000
      out->servo2.proportional_input = in->joystick.y_axis;  // Same for all
      out->servo3.proportional_input = in->joystick.y_axis;  // Same for all
      out->servo4.proportional_input = in->joystick.y_axis;  // Same for all
    }
    // If joystick is in center deadzone, keep servos at last position (no update)

    // Joystick controls motors - X axis for steering, Y axis for speed
    int speed = (in->joystick.y_axis * MOTOR_DEFAULT_SPEED) / 1000;  // Forward/backward
    int steering = in->joystick.x_axis / 10;  // Left/right steering angle
    set_motor_motion_by_angle(steering, abs(speed), &out->motor);
  }

// Multiple servos - no default behavior needed, will be controlled by IR commands

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

void hrms_controller_process_ir_remote(const hrms_ir_remote_event_t *in,
                                       hrms_actuator_command_t *out) {
  if (!in || !out)
    return;

  int16_t speed = MOTOR_DEFAULT_SPEED;

  switch (in->command) {
  case HRMS_IR_CMD_1:
    hrms_controller_change_mode(HRMS_MODE_MANUAL, out);
    break;

  case HRMS_IR_CMD_2:
    hrms_controller_change_mode(HRMS_MODE_AUTO, out);
    break;

  case HRMS_IR_CMD_3:
    hrms_controller_change_mode(HRMS_MODE_EMERGENCY, out);
    break;

  // Servo1 control (PA0) - Keys 4, 5, 6
  case HRMS_IR_CMD_4:
    // Servo1 to minimum (-90°)
    out->servo1.proportional_input = -1000;
    break;
  case HRMS_IR_CMD_5:
    // Servo1 to center (0°)
    out->servo1.proportional_input = 0;
    break;
  case HRMS_IR_CMD_6:
    // Servo1 to maximum (+90°)
    out->servo1.proportional_input = 1000;
    break;
    
  // Servo2 control (PA1) - Keys 7, 8, 9
  case HRMS_IR_CMD_7:
    // Servo2 to minimum (-90°)
    out->servo4.proportional_input = -1000;
    break;
  case HRMS_IR_CMD_8:
    // Servo2 to center (0°)
    out->servo4.proportional_input = 0;
    break;
  case HRMS_IR_CMD_9:
    // Servo2 to maximum (+90°)
    out->servo4.proportional_input = 1000;
    break;
    
  // All servos control - Keys *, 0, #
  case HRMS_IR_CMD_STAR:
    // All servos to minimum (-90°)
    out->servo1.proportional_input = -1000;
    out->servo2.proportional_input = -1000;
    out->servo3.proportional_input = -1000;
    out->servo4.proportional_input = -1000;
    break;
  case HRMS_IR_CMD_0:
    // All servos to center (0°)
    out->servo1.proportional_input = 0;
    out->servo2.proportional_input = 0;
    out->servo3.proportional_input = 0;
    out->servo4.proportional_input = 0;
    break;
  case HRMS_IR_CMD_HASH:
    // All servos to maximum (+90°)
    out->servo1.proportional_input = 1000;
    out->servo2.proportional_input = 1000;
    out->servo3.proportional_input = 1000;
    out->servo4.proportional_input = 1000;
    break;

  default:
    break;
  }

  if (hrms_system_state.current_mode != HRMS_MODE_MANUAL)
    return;

  switch (in->command) {
  case HRMS_IR_CMD_UP:
    hrms_system_state.motion_state = HRMS_MOTION_FORWARD;
    break;

  case HRMS_IR_CMD_DOWN:
    hrms_system_state.motion_state = HRMS_MOTION_BACKWARD;
    break;

  case HRMS_IR_CMD_LEFT:
    hrms_system_state.motion_state = HRMS_MOTION_ROTATE_LEFT;
    break;

  case HRMS_IR_CMD_RIGHT:
    hrms_system_state.motion_state = HRMS_MOTION_ROTATE_RIGHT;
    break;

  case HRMS_IR_CMD_OK:
  default:
    speed = 0;
    hrms_system_state.motion_state = HRMS_MOTION_STOP;
    break;
  }

  int angle = motion_state_to_angle(hrms_system_state.motion_state);
  set_motor_motion_by_angle(angle, speed, &out->motor);
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

bool hrms_controller_check_ir_timeout(hrms_actuator_command_t *out) {
  (void)out;
  return false;
}

void hrms_controller_process_bigsound(const hrms_bigsound_event_t *event,
                                      hrms_actuator_command_t *out) {
  if (!event || !out)
    return;
  // You can implement this or leave empty
}

// ESP32 communication module removed - ready for new implementation
