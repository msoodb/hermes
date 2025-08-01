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
#include "hrms_lora_comm.h"
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
  hrms_lora_comm_init();
}

bool hrms_communication_hub_send(const uint8_t *data, size_t len) {
  if (!data || len == 0) {
    return false;
  }
  
  bool success = false;
  
  // Try to send via available communication modules
  // Primary: nRF24L01
  success = hrms_nrf24_comm_send(data, len);
  
  // Backup: LoRa module for redundancy
  if (!success) {
    success = hrms_lora_comm_send(data, len);
  }
  
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
  bool received = false;
  
  // Check communication modules for received data
  // Try nRF24L01 first
  if (hrms_nrf24_comm_receive(data, max_len, received_len)) {
    received = true;
    comm_stats.packets_received++;
    comm_stats.last_rx_timestamp = xTaskGetTickCount();
  }
  
  // Try LoRa module if no data from nRF24
  if (!received && hrms_lora_comm_receive(data, max_len, received_len)) {
    received = true;
    comm_stats.packets_received++;
    comm_stats.last_rx_timestamp = xTaskGetTickCount();
  }
  
  return received;
}

void hrms_communication_hub_process(void) {
  // Process all communication modules (following sensor/actuator pattern)
  hrms_nrf24_comm_process();
  hrms_lora_comm_process();
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