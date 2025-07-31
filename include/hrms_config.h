
/**
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef BLFM_CONFIG_H
#define BLFM_CONFIG_H

/* === Sensors presence flags === */
#define BLFM_ENABLED_ULTRASONIC 0
#define BLFM_ENABLED_POTENTIOMETER 0
#define BLFM_ENABLED_TEMPERATURE 0

/* === Actuators presence flags === */
#define BLFM_ENABLED_LED 1
#define BLFM_ENABLED_MOTOR 0
#define BLFM_ENABLED_DISPLAY 0
#define BLFM_ENABLED_ALARM 0
#define BLFM_ENABLED_SERVO 1

// Individual servo channel control - all servos enabled
#define BLFM_ENABLED_SERVO1 1  // PA0 
#define BLFM_ENABLED_SERVO2 1  // PA1
#define BLFM_ENABLED_SERVO3 1  // PA2
#define BLFM_ENABLED_SERVO4 1  // PA3

#define BLFM_ENABLED_OLED 0

/* === Communication features === */
#define BLFM_ENABLED_COMMUNICATION_HUB 1
#define BLFM_ENABLED_NRF24L01 1
#define BLFM_ENABLED_LORA_SX1278 0  // Future expansion

/* === input/event features === */
#define BLFM_ENABLED_BIGSOUND 0
#define BLFM_ENABLED_IR_REMOTE 1
#define BLFM_ENABLED_MODE_BUTTON 1
#define BLFM_ENABLED_JOYSTICK 0  // Disabled - will be controlled by Hermes remote

#endif /* BLFM_CONFIG_H */
