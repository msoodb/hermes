/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Hermes.
 *
 * Hermes is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_BOARD_CONFIG_H
#define HRMS_BOARD_CONFIG_H

#include "hrms_gpio.h"
#include "stm32f1xx.h"

// Centralized board configuration
typedef struct {
    GPIO_TypeDef *port;
    uint32_t pin;
    gpio_mode_t mode;
    const char *name;
} gpio_pin_config_t;

// All GPIO pins configured in one place
void hrms_board_gpio_init_all(void);

// Task configuration in one place
typedef struct {
    const char *name;
    uint16_t stack_size;
    uint8_t priority;
    uint16_t queue_length;
    uint16_t queue_item_size;
} task_config_t;

// Consolidated system configuration
#define HRMS_SENSOR_TASK_STACK          256  // Reduced from 384
#define HRMS_CONTROLLER_TASK_STACK      384  // Reduced from 512  
#define HRMS_ACTUATOR_TASK_STACK        256  // Reduced from 384
#define HRMS_COMM_TASK_STACK            384  // Reduced from 512

#define HRMS_SENSOR_QUEUE_LEN           8    // Was 15
#define HRMS_BUTTON_QUEUE_LEN           5    // Was 8
#define HRMS_ACTUATOR_QUEUE_LEN         5    // Was 10
#define HRMS_COMM_QUEUE_LEN             5    // Was 10

// Timing configuration
#define HRMS_SENSOR_READ_INTERVAL_MS    100  // Was 200
#define HRMS_CONTROLLER_CYCLE_MS        20   // Was 10  
#define HRMS_ACTUATOR_CYCLE_MS          50   // Was 10
#define HRMS_COMM_CYCLE_MS              20   // Was 10

#endif // HRMS_BOARD_CONFIG_H