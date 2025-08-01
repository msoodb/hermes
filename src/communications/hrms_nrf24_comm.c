/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Hermes.
 *
 * Hermes is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_nrf24_comm.h"
#include "hrms_nrf24l01.h"

// nRF24 communication module following sensor/actuator pattern
bool hrms_nrf24_comm_init(void) {
  if (!hrms_nrf24l01_init()) {
    return false;
  }
  
  // Configure nRF24L01 with default settings for Hermes RC transmitter
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
  
  return true;
}

bool hrms_nrf24_comm_send(const uint8_t *data, size_t len) {
  if (!data || len == 0) {
    return false;
  }
  
  // Send via nRF24L01
  hrms_nrf24l01_stop_listening();
  bool success = hrms_nrf24l01_send(data, len);
  hrms_nrf24l01_start_listening();
  
  return success;
}

bool hrms_nrf24_comm_receive(uint8_t *data, size_t max_len, size_t *received_len) {
  if (!data || !received_len || max_len == 0) {
    return false;
  }
  
  *received_len = 0;
  
  if (!hrms_nrf24l01_available()) {
    return false;
  }
  
  uint8_t bytes_received = hrms_nrf24l01_receive(data, max_len);
  if (bytes_received > 0) {
    *received_len = bytes_received;
    return true;
  }
  
  return false;
}

void hrms_nrf24_comm_process(void) {
  // Periodic processing for nRF24L01
  hrms_nrf24l01_clear_interrupts();
}