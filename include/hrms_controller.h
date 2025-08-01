
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_CONTROLLER_H
#define HRMS_CONTROLLER_H

#include "hrms_types.h"

/**
 * Initialize the controller state. Call once at startup.
 */
void hrms_controller_init(void);

/**
 * Cycle between MANUAL/AUTO/EMERGENCY modes.
 * Updates the LED and stops motors.
 */
void hrms_controller_cycle_mode(hrms_actuator_command_t *out);

/**
 * Process sensor data and update actuator commands.
 * Typically called periodically in AUTO mode.
 */
void hrms_controller_process(const hrms_sensor_data_t *in,
                             hrms_actuator_command_t *out);

/**
 * Send encrypted joystick commands via radio communication.
 * Uses ORION encryption when available, falls back to plaintext.
 */
void hrms_controller_send_joystick_command(const hrms_joystick_data_t *joystick_data);



/**
 * Handle mode button events with debounce.
 * Typically cycles modes and can trigger servo motions.
 */
void hrms_controller_process_mode_button(const hrms_mode_button_event_t *event,
                                         hrms_actuator_command_t *command);


// ESP32 communication removed - ready for new implementation



#endif /* HRMS_CONTROLLER_H */
