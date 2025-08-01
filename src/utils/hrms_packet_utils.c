/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Hermes.
 *
 * Hermes is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_packet_utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "libc_stubs.h"
#include <stddef.h>

// Static packet ID counter
static uint8_t next_packet_id = 1;

void hrms_packet_create_heartbeat(hrms_comm_packet_t *packet, uint8_t source_id) {
  if (!packet) {
    return;
  }

  memset(packet, 0, sizeof(hrms_comm_packet_t));
  
  packet->packet_id = next_packet_id++;
  packet->packet_type = HRMS_COMM_PACKET_HEARTBEAT;
  packet->source_id = source_id;
  packet->dest_id = 0xFF; // Broadcast
  packet->payload_size = 0; // No payload for heartbeat
  packet->timestamp = xTaskGetTickCount();
  
  hrms_packet_set_checksum(packet);
}

void hrms_packet_set_checksum(hrms_comm_packet_t *packet) {
  if (!packet) {
    return;
  }

  // Simple checksum calculation (excluding checksum field itself)
  uint16_t checksum = 0;
  uint8_t *data = (uint8_t*)packet;
  
  // Calculate checksum for all fields except checksum field
  for (size_t i = 0; i < offsetof(hrms_comm_packet_t, checksum); i++) {
    checksum += data[i];
  }
  
  // Include timestamp in checksum
  uint8_t *timestamp_ptr = (uint8_t*)&packet->timestamp;
  for (size_t i = 0; i < sizeof(packet->timestamp); i++) {
    checksum += timestamp_ptr[i];
  }
  
  packet->checksum = checksum;
}

bool hrms_packet_verify_checksum(const hrms_comm_packet_t *packet) {
  if (!packet) {
    return false;
  }

  // Calculate expected checksum
  uint16_t expected_checksum = 0;
  const uint8_t *data = (const uint8_t*)packet;
  
  // Calculate checksum for all fields except checksum field
  for (size_t i = 0; i < offsetof(hrms_comm_packet_t, checksum); i++) {
    expected_checksum += data[i];
  }
  
  // Include timestamp in checksum
  const uint8_t *timestamp_ptr = (const uint8_t*)&packet->timestamp;
  for (size_t i = 0; i < sizeof(packet->timestamp); i++) {
    expected_checksum += timestamp_ptr[i];
  }
  
  return (packet->checksum == expected_checksum);
}

uint8_t hrms_packet_get_next_id(void) {
  return next_packet_id++;
}