/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Hermes.
 *
 * Hermes is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_lora_comm.h"
#include "hrms_sx1278.h"
#include "FreeRTOS.h"
#include "task.h"

// Default LoRa configuration for 433MHz operation
static const sx1278_config_t default_config = {
  .frequency = 433000000,     // 433MHz
  .spreading_factor = 7,      // SF7 for good range/speed balance
  .bandwidth = 7,             // 125kHz
  .coding_rate = 1,           // 4/5 coding rate
  .tx_power = 17,             // Maximum power
  .preamble_length = 8,       // Standard preamble
  .crc_enable = true          // Enable CRC for reliability
};

static bool initialized = false;
static bool listening = false;

bool hrms_lora_comm_init(void) {
  if (initialized) {
    return true;
  }
  
  // Initialize SX1278 hardware
  if (!hrms_sx1278_init()) {
    return false;
  }
  
  // Configure with default settings
  if (!hrms_sx1278_configure(&default_config)) {
    return false;
  }
  
  // Start in listening mode
  hrms_sx1278_start_listening();
  listening = true;
  
  initialized = true;
  return true;
}

bool hrms_lora_comm_send(const uint8_t *data, size_t len) {
  if (!initialized || !data || len == 0) {
    return false;
  }
  
  // Stop listening temporarily
  if (listening) {
    hrms_sx1278_stop_listening();
  }
  
  // Send data
  bool success = hrms_sx1278_send(data, len);
  
  if (success) {
    // Wait for transmission to complete
    uint32_t timeout = 0;
    while (hrms_sx1278_is_transmitting() && timeout < 1000) {
      vTaskDelay(pdMS_TO_TICKS(1));
      timeout++;
    }
  }
  
  // Resume listening
  if (listening) {
    hrms_sx1278_start_listening();
  }
  
  return success;
}

bool hrms_lora_comm_receive(uint8_t *data, size_t max_len, size_t *received_len) {
  if (!initialized || !data || !received_len || max_len == 0) {
    return false;
  }
  
  *received_len = 0;
  
  // Check if data is available
  if (!hrms_sx1278_available()) {
    return false;
  }
  
  // Receive data
  uint8_t bytes_received = hrms_sx1278_receive(data, max_len);
  if (bytes_received > 0) {
    *received_len = bytes_received;
    return true;
  }
  
  return false;
}

void hrms_lora_comm_process(void) {
  if (!initialized) {
    return;
  }
  
  // Clear any pending interrupts
  hrms_sx1278_clear_interrupts();
  
  // Ensure we're in listening mode if we should be
  if (listening && !hrms_sx1278_is_transmitting()) {
    // Check if we're still in RX mode, restart if needed
    // This is a simple keepalive for the listening mode
    static uint32_t last_listening_check = 0;
    uint32_t current_time = xTaskGetTickCount();
    
    if (current_time - last_listening_check > pdMS_TO_TICKS(5000)) {
      hrms_sx1278_start_listening();
      last_listening_check = current_time;
    }
  }
}