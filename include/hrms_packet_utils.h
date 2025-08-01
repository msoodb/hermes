/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Hermes.
 *
 * Hermes is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_PACKET_UTILS_H
#define HRMS_PACKET_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "hrms_types.h"

/**
 * @file hrms_packet_utils.h
 * @brief Packet management utilities for communication system
 * 
 * This module provides utilities for creating, validating, and managing
 * communication packets, separated from the communication hub logic.
 */

/**
 * Create a heartbeat packet
 * @param packet Pointer to packet structure to fill
 * @param source_id Source device ID
 */
void hrms_packet_create_heartbeat(hrms_comm_packet_t *packet, uint8_t source_id);

/**
 * Set checksum for a packet
 * @param packet Pointer to packet to set checksum for
 */
void hrms_packet_set_checksum(hrms_comm_packet_t *packet);

/**
 * Verify packet checksum
 * @param packet Pointer to packet to verify
 * @return true if checksum is valid, false otherwise
 */
bool hrms_packet_verify_checksum(const hrms_comm_packet_t *packet);

/**
 * Get next packet ID
 * @return Next sequential packet ID
 */
uint8_t hrms_packet_get_next_id(void);

#endif /* HRMS_PACKET_UTILS_H */