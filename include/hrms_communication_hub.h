/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef BLFM_COMMUNICATION_HUB_H
#define BLFM_COMMUNICATION_HUB_H

#include "hrms_config.h"

#if BLFM_ENABLED_COMMUNICATION_HUB

#include "hrms_types.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Initialize the communication hub and all enabled communication modules
 */
void hrms_communication_hub_init(void);

/**
 * @brief Send a packet through the communication hub
 * @param packet Pointer to the packet to send
 * @return true if packet was queued for transmission, false on error
 */
bool hrms_communication_hub_send(const hrms_comm_packet_t *packet);

/**
 * @brief Check for received packets
 * @param packet Pointer to store received packet
 * @return true if a packet was received, false if no packets available
 */
bool hrms_communication_hub_receive(hrms_comm_packet_t *packet);

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

#endif /* BLFM_ENABLED_COMMUNICATION_HUB */

#endif /* BLFM_COMMUNICATION_HUB_H */