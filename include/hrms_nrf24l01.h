/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_NRF24L01_H
#define HRMS_NRF24L01_H


#include "hrms_types.h"
#include <stdbool.h>
#include <stdint.h>

// nRF24L01 configuration constants
#define NRF24L01_MAX_PAYLOAD_SIZE 32
#define NRF24L01_ADDR_WIDTH 5
#define NRF24L01_MAX_CHANNEL 125

// nRF24L01 Register Map
#define NRF24L01_REG_CONFIG         0x00
#define NRF24L01_REG_EN_AA          0x01
#define NRF24L01_REG_EN_RXADDR      0x02
#define NRF24L01_REG_SETUP_AW       0x03
#define NRF24L01_REG_SETUP_RETR     0x04
#define NRF24L01_REG_RF_CH          0x05
#define NRF24L01_REG_RF_SETUP       0x06
#define NRF24L01_REG_STATUS         0x07
#define NRF24L01_REG_OBSERVE_TX     0x08
#define NRF24L01_REG_RPD            0x09
#define NRF24L01_REG_RX_ADDR_P0     0x0A
#define NRF24L01_REG_RX_ADDR_P1     0x0B
#define NRF24L01_REG_RX_ADDR_P2     0x0C
#define NRF24L01_REG_RX_ADDR_P3     0x0D
#define NRF24L01_REG_RX_ADDR_P4     0x0E
#define NRF24L01_REG_RX_ADDR_P5     0x0F
#define NRF24L01_REG_TX_ADDR        0x10
#define NRF24L01_REG_RX_PW_P0       0x11
#define NRF24L01_REG_RX_PW_P1       0x12
#define NRF24L01_REG_RX_PW_P2       0x13
#define NRF24L01_REG_RX_PW_P3       0x14
#define NRF24L01_REG_RX_PW_P4       0x15
#define NRF24L01_REG_RX_PW_P5       0x16
#define NRF24L01_REG_FIFO_STATUS    0x17

// nRF24L01 Commands
#define NRF24L01_CMD_R_REGISTER     0x00
#define NRF24L01_CMD_W_REGISTER     0x20
#define NRF24L01_CMD_R_RX_PAYLOAD   0x61
#define NRF24L01_CMD_W_TX_PAYLOAD   0xA0
#define NRF24L01_CMD_FLUSH_TX       0xE1
#define NRF24L01_CMD_FLUSH_RX       0xE2
#define NRF24L01_CMD_REUSE_TX_PL    0xE3
#define NRF24L01_CMD_NOP            0xFF

// Status register bits
#define NRF24L01_STATUS_RX_DR       0x40
#define NRF24L01_STATUS_TX_DS       0x20
#define NRF24L01_STATUS_MAX_RT      0x10
#define NRF24L01_STATUS_RX_P_NO     0x0E
#define NRF24L01_STATUS_TX_FULL     0x01

// Config register bits
#define NRF24L01_CONFIG_PWR_UP      0x02
#define NRF24L01_CONFIG_PRIM_RX     0x01

// Power and data rate options
typedef enum {
  NRF24L01_POWER_0DBM = 0,
  NRF24L01_POWER_NEG6DBM,
  NRF24L01_POWER_NEG12DBM,
  NRF24L01_POWER_NEG18DBM
} nrf24l01_power_t;

typedef enum {
  NRF24L01_DATARATE_1MBPS = 0,
  NRF24L01_DATARATE_2MBPS,
  NRF24L01_DATARATE_250KBPS
} nrf24l01_datarate_t;

// nRF24L01 configuration structure
typedef struct {
  uint8_t channel;                      // RF channel (0-125)
  nrf24l01_power_t power;               // TX power level
  nrf24l01_datarate_t datarate;         // Data rate
  uint8_t address[NRF24L01_ADDR_WIDTH]; // Device address
  bool auto_ack;                        // Enable auto-acknowledgment
  uint8_t retries;                      // Number of retries (0-15)
  uint8_t retry_delay;                  // Retry delay (0-15, in 250us steps)
} nrf24l01_config_t;

/**
 * @brief Initialize nRF24L01 module
 * @return true if initialization successful, false otherwise
 */
bool hrms_nrf24l01_init(void);

/**
 * @brief Configure nRF24L01 with specified parameters
 * @param config Pointer to configuration structure
 * @return true if configuration successful, false otherwise
 */
bool hrms_nrf24l01_configure(const nrf24l01_config_t *config);

/**
 * @brief Send data packet
 * @param data Pointer to data to send
 * @param length Length of data (max 32 bytes)
 * @return true if packet was sent successfully, false otherwise
 */
bool hrms_nrf24l01_send(const uint8_t *data, uint8_t length);

/**
 * @brief Check if data is available to receive
 * @return true if data is available, false otherwise
 */
bool hrms_nrf24l01_available(void);

/**
 * @brief Receive data packet
 * @param data Pointer to buffer to store received data
 * @param max_length Maximum length of buffer
 * @return Number of bytes received, 0 if no data available
 */
uint8_t hrms_nrf24l01_receive(uint8_t *data, uint8_t max_length);

/**
 * @brief Set module to receive mode
 */
void hrms_nrf24l01_start_listening(void);

/**
 * @brief Set module to transmit mode
 */
void hrms_nrf24l01_stop_listening(void);

/**
 * @brief Check if module is in receive mode
 * @return true if listening, false otherwise
 */
bool hrms_nrf24l01_is_listening(void);

/**
 * @brief Get status register value
 * @return Status register value
 */
uint8_t hrms_nrf24l01_get_status(void);

/**
 * @brief Clear interrupt flags
 */
void hrms_nrf24l01_clear_interrupts(void);

/**
 * @brief Power down the module
 */
void hrms_nrf24l01_power_down(void);

/**
 * @brief Power up the module
 */
void hrms_nrf24l01_power_up(void);

#endif /* HRMS_NRF24L01_H */