/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Hermes.
 *
 * Hermes is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_communication_hub.h"
#include "hrms_nrf24_comm.h"
#include "hrms_packet_utils.h"
#include "hrms_types.h"
#include "ORION_Config.h"
#include "orion.h"
#include "FreeRTOS.h"
#include "task.h"
#include "libc_stubs.h"

// Communication hub statistics
static hrms_comm_stats_t comm_stats = {0};

void hrms_communication_hub_init(void) {
  // Initialize statistics
  memset(&comm_stats, 0, sizeof(comm_stats));
  
  // Initialize ORION encryption system
  ORION_Init();
  
  // Initialize communication modules (following sensor/actuator pattern)
  hrms_nrf24_comm_init();
}

bool hrms_communication_hub_send(const uint8_t *data, size_t len) {
  if (!data || len == 0) {
    return false;
  }
  
  // Send via nRF24L01
  bool success = hrms_nrf24_comm_send(data, len);
  
  // Update statistics
  if (success) {
    comm_stats.packets_sent++;
    comm_stats.last_tx_timestamp = xTaskGetTickCount();
  } else {
    comm_stats.packets_failed++;
  }
  
  return success;
}

bool hrms_communication_hub_receive(uint8_t *data, size_t max_len, size_t *received_len) {
  if (!data || !received_len || max_len == 0) {
    return false;
  }
  
  *received_len = 0;
  
  // Check nRF24L01 for received data
  bool received = hrms_nrf24_comm_receive(data, max_len, received_len);
  
  if (received) {
    comm_stats.packets_received++;
    comm_stats.last_rx_timestamp = xTaskGetTickCount();
  }
  
  return received;
}

void hrms_communication_hub_process(void) {
  // Process nRF24L01 communication module
  hrms_nrf24_comm_process();
}

void hrms_communication_hub_get_stats(hrms_comm_stats_t *stats) {
  if (stats) {
    memcpy(stats, &comm_stats, sizeof(hrms_comm_stats_t));
  }
}

// Wrapper functions for packet utilities (for backward compatibility)
void hrms_communication_hub_create_heartbeat(hrms_comm_packet_t *packet, uint8_t source_id) {
  hrms_packet_create_heartbeat(packet, source_id);
}

void hrms_communication_hub_set_checksum(hrms_comm_packet_t *packet) {
  hrms_packet_set_checksum(packet);
}

bool hrms_communication_hub_verify_checksum(const hrms_comm_packet_t *packet) {
  return hrms_packet_verify_checksum(packet);
}

bool hrms_communication_hub_send_joystick_data(const hrms_comm_command_t *comm_cmd) {
  if (!comm_cmd || !comm_cmd->should_transmit) {
    return false;
  }
  
  hrms_comm_packet_t packet;
  memset(&packet, 0, sizeof(packet));
  
  uint32_t now = xTaskGetTickCount();
  packet.packet_id = (uint8_t)(now % 255) + 1;
  packet.packet_type = comm_cmd->packet_type;
  packet.source_id = 0x01; // Hermes controller ID
  packet.dest_id = comm_cmd->dest_id;
  packet.timestamp = now;
  
  // Prepare joystick data for encryption
  uint8_t plaintext[sizeof(hrms_joystick_data_t)];
  memcpy(plaintext, &comm_cmd->joystick_data, sizeof(hrms_joystick_data_t));
  
  // Encrypt the joystick data using ORION
  uint8_t encrypted_data[HRMS_COMM_MAX_PAYLOAD_SIZE];
  size_t encrypted_len = 0;
  
  // Always encrypt - no plaintext fallback
  if (ORION_Encrypt(plaintext, sizeof(hrms_joystick_data_t), encrypted_data, &encrypted_len) == 0) {
    packet.payload_size = (uint8_t)encrypted_len;
    if (encrypted_len <= HRMS_COMM_MAX_PAYLOAD_SIZE) {
      memcpy(packet.payload, encrypted_data, encrypted_len);
    }
  } else {
    // Encryption failed - abort transmission for security
    return false;
  }
  
  hrms_communication_hub_set_checksum(&packet);
  
  // Send the packet as raw data through the communication hub
  return hrms_communication_hub_send((uint8_t*)&packet, sizeof(packet));
}