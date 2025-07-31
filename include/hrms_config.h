
/**
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_CONFIG_H
#define HRMS_CONFIG_H

/* === Sensors presence flags === */
#define HRMS_ENABLED_ULTRASONIC 0
#define HRMS_ENABLED_POTENTIOMETER 0
#define HRMS_ENABLED_TEMPERATURE 0

/* === Actuators presence flags === */
#define HRMS_ENABLED_LED 1
#define HRMS_ENABLED_MOTOR 0
#define HRMS_ENABLED_DISPLAY 0
#define HRMS_ENABLED_ALARM 0
#define HRMS_ENABLED_SERVO 1

// Individual servo channel control - all servos enabled
#define HRMS_ENABLED_SERVO1 1  // PA0 
#define HRMS_ENABLED_SERVO2 0  // PA1
#define HRMS_ENABLED_SERVO3 0  // PA2
#define HRMS_ENABLED_SERVO4 0  // PA3

#define HRMS_ENABLED_OLED 0

/* === Communication features === */
#define HRMS_ENABLED_COMMUNICATION_HUB 0
#define HRMS_ENABLED_NRF24L01 0
#define HRMS_ENABLED_LORA_SX1278 0  // Future expansion

/* === input/event features === */
#define HRMS_ENABLED_BIGSOUND 0
#define HRMS_ENABLED_IR_REMOTE 1
#define HRMS_ENABLED_MODE_BUTTON 0
#define HRMS_ENABLED_JOYSTICK 0  // Disabled - will be controlled by Hermes remote

#endif /* HRMS_CONFIG_H */
