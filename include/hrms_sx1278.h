/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Hermes.
 *
 * Hermes is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_SX1278_H
#define HRMS_SX1278_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file hrms_sx1278.h
 * @brief SX1278 LoRa transceiver driver for Hermes
 * 
 * Low-level driver for SX1278 LoRa module providing basic radio operations.
 */

// SX1278 LoRa configuration structure
typedef struct {
  uint32_t frequency;      // Frequency in Hz (e.g., 433000000 for 433MHz)
  uint8_t spreading_factor; // 6-12 (SF6-SF12)
  uint8_t bandwidth;       // 0=7.8kHz, 1=10.4kHz, 2=15.6kHz, 3=20.8kHz, 4=31.25kHz, 5=41.7kHz, 6=62.5kHz, 7=125kHz, 8=250kHz, 9=500kHz
  uint8_t coding_rate;     // 1=4/5, 2=4/6, 3=4/7, 4=4/8
  uint8_t tx_power;        // 2-17 dBm
  uint16_t preamble_length; // Preamble length
  bool crc_enable;         // Enable CRC
} sx1278_config_t;

/**
 * Initialize SX1278 LoRa module
 * @return true if initialization successful, false otherwise
 */
bool hrms_sx1278_init(void);

/**
 * Configure SX1278 with specified parameters
 * @param config Pointer to configuration structure
 * @return true if configuration successful, false otherwise
 */
bool hrms_sx1278_configure(const sx1278_config_t *config);

/**
 * Send data via SX1278 LoRa
 * @param data Pointer to data to send
 * @param len Length of data in bytes (max 255)
 * @return true if transmission started successfully, false otherwise
 */
bool hrms_sx1278_send(const uint8_t *data, size_t len);

/**
 * Check if data is available for reception
 * @return true if data is available, false otherwise
 */
bool hrms_sx1278_available(void);

/**
 * Receive data from SX1278 LoRa
 * @param data Buffer to store received data
 * @param max_len Maximum buffer size
 * @return Number of bytes received, 0 if no data
 */
uint8_t hrms_sx1278_receive(uint8_t *data, size_t max_len);

/**
 * Start listening mode (continuous receive)
 */
void hrms_sx1278_start_listening(void);

/**
 * Stop listening mode
 */
void hrms_sx1278_stop_listening(void);

/**
 * Clear interrupt flags
 */
void hrms_sx1278_clear_interrupts(void);

/**
 * Get RSSI of last received packet
 * @return RSSI in dBm
 */
int16_t hrms_sx1278_get_rssi(void);

/**
 * Check if module is transmitting
 * @return true if transmitting, false otherwise
 */
bool hrms_sx1278_is_transmitting(void);

#endif /* HRMS_SX1278_H */