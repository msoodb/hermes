/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_COMMUNICATION_HUB_H
#define HRMS_COMMUNICATION_HUB_H


#include "hrms_types.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Initialize the communication hub and all enabled communication modules
 */
void hrms_communication_hub_init(void);

/**
 * @brief Send raw data through the communication hub
 * @param data Pointer to the data to send
 * @param len Length of data in bytes
 * @return true if data was sent successfully, false on error
 */
bool hrms_communication_hub_send(const uint8_t *data, size_t len);

/**
 * @brief Check for received data
 * @param data Buffer to store received data
 * @param max_len Maximum buffer size
 * @param received_len Pointer to store actual received length
 * @return true if data was received, false if no data available
 */
bool hrms_communication_hub_receive(uint8_t *data, size_t max_len, size_t *received_len);

/**
 * @brief Process communication hub (call periodically from task)
 * Handles transmission retries, timeouts, and module polling
 */
void hrms_communication_hub_process(void);

/**
 * @brief Get communication statistics
 * @param stats Pointer to store statistics
 */
void hrms_communication_hub_get_stats(hrms_comm_stats_t *stats);

/**
 * @brief Create a simple heartbeat packet
 * @param packet Pointer to store the created packet
 * @param source_id Source device ID
 */
void hrms_communication_hub_create_heartbeat(hrms_comm_packet_t *packet, uint8_t source_id);

/**
 * @brief Calculate and set checksum for a packet
 * @param packet Pointer to the packet
 */
void hrms_communication_hub_set_checksum(hrms_comm_packet_t *packet);

/**
 * @brief Verify checksum of a received packet
 * @param packet Pointer to the packet to verify
 * @return true if checksum is valid, false otherwise
 */
bool hrms_communication_hub_verify_checksum(const hrms_comm_packet_t *packet);

/**
 * @brief Send joystick data as encrypted communication packet
 * @param comm_cmd Communication command from controller
 * @return true if transmission was successful, false otherwise
 */
bool hrms_communication_hub_send_joystick_data(const hrms_comm_command_t *comm_cmd);

#endif /* HRMS_COMMUNICATION_HUB_H */