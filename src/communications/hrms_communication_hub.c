/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */


#include "hrms_communication_hub.h"
#include "hrms_types.h"
#include "ORION_Config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "libc_stubs.h"

#include "hrms_nrf24l01.h"

#include "orion.h"

// Communication hub state
static hrms_comm_stats_t comm_stats = {0};
static uint8_t next_packet_id = 1;

void hrms_communication_hub_init(void) {
  // Initialize statistics
  memset(&comm_stats, 0, sizeof(comm_stats));
  
  // Initialize ORION encryption system
  ORION_Init();
  
  // Initialize enabled communication modules
  if (hrms_nrf24l01_init()) {
    // Configure nRF24L01 with default settings
    nrf24l01_config_t config = {
      .channel = 76,                    // 2.476 GHz
      .power = NRF24L01_POWER_0DBM,     // Maximum power
      .datarate = NRF24L01_DATARATE_1MBPS,
      .address = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}, // Default address
      .auto_ack = true,
      .retries = 3,
      .retry_delay = 5                  // 1.25ms
    };
    
    hrms_nrf24l01_configure(&config);
    hrms_nrf24l01_start_listening();   // Start in receive mode
  }

  // Future: Initialize LoRa SX1278 when enabled
}

bool hrms_communication_hub_send(const hrms_comm_packet_t *packet) {
  if (!packet) {
    return false;
  }

  bool success = false;
  
  // Try to send via enabled communication modules
  // Prepare packet for transmission
  uint8_t tx_buffer[sizeof(hrms_comm_packet_t)];
  memcpy(tx_buffer, packet, sizeof(hrms_comm_packet_t));
  
  // Send via nRF24L01
  hrms_nrf24l01_stop_listening();
  success = hrms_nrf24l01_send(tx_buffer, sizeof(hrms_comm_packet_t));
  hrms_nrf24l01_start_listening();

  // Update statistics
  if (success) {
    comm_stats.packets_sent++;
    comm_stats.last_tx_timestamp = xTaskGetTickCount();
  } else {
    comm_stats.packets_failed++;
  }

  return success;
}

bool hrms_communication_hub_receive(hrms_comm_packet_t *packet) {
  if (!packet) {
    return false;
  }

  bool received = false;
  
  // Check enabled communication modules for received data
  if (hrms_nrf24l01_available()) {
    uint8_t rx_buffer[sizeof(hrms_comm_packet_t)];
    uint8_t received_bytes = hrms_nrf24l01_receive(rx_buffer, sizeof(rx_buffer));
    
    if (received_bytes == sizeof(hrms_comm_packet_t)) {
      memcpy(packet, rx_buffer, sizeof(hrms_comm_packet_t));
      
      // Verify checksum
      if (hrms_communication_hub_verify_checksum(packet)) {
        received = true;
        comm_stats.packets_received++;
        comm_stats.last_rx_timestamp = xTaskGetTickCount();
      } else {
        comm_stats.packets_failed++;
      }
    }
  }

  return received;
}

void hrms_communication_hub_process(void) {
  // Periodic processing tasks
  
  // Clear any interrupt flags
  hrms_nrf24l01_clear_interrupts();

  // Future: Add periodic tasks like heartbeats, retransmissions, etc.
}

void hrms_communication_hub_get_stats(hrms_comm_stats_t *stats) {
  if (stats) {
    memcpy(stats, &comm_stats, sizeof(hrms_comm_stats_t));
  }
}

void hrms_communication_hub_create_heartbeat(hrms_comm_packet_t *packet, uint8_t source_id) {
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
  
  hrms_communication_hub_set_checksum(packet);
}

void hrms_communication_hub_set_checksum(hrms_comm_packet_t *packet) {
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

bool hrms_communication_hub_verify_checksum(const hrms_comm_packet_t *packet) {
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

