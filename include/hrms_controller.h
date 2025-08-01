
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
 * Process sensor data and generate actuator commands.
 * Pure controller logic - no direct I/O operations.
 */
void hrms_controller_process(const hrms_sensor_data_t *in,
                             hrms_actuator_command_t *out);



/**
 * Handle mode button events with debounce.
 * Typically cycles modes and can trigger servo motions.
 */
void hrms_controller_process_button(const hrms_button_event_t *event,
                                         hrms_actuator_command_t *command);


// ESP32 communication removed - ready for new implementation



#endif /* HRMS_CONTROLLER_H */
