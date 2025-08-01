/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */


#include "hrms_nrf24l01.h"
#include "hrms_pins.h"
#include "hrms_gpio.h"
#include "hrms_spi.h"
#include "hrms_delay.h"
#include "libc_stubs.h"

// Module state
static bool is_listening = false;
static nrf24l01_config_t current_config;

// Low-level SPI functions
static uint8_t nrf24l01_spi_transfer(uint8_t data);
static void nrf24l01_cs_low(void);
static void nrf24l01_cs_high(void);
static void nrf24l01_ce_low(void);
static void nrf24l01_ce_high(void);

// Register access functions
static uint8_t nrf24l01_read_register(uint8_t reg);
static void nrf24l01_write_register(uint8_t reg, uint8_t value);
static void nrf24l01_write_register_multi(uint8_t reg, const uint8_t *data, uint8_t length);

bool hrms_nrf24l01_init(void) {
  // Initialize GPIO pins
  hrms_gpio_config_output((uint32_t)HRMS_NRF24L01_SPI_PORT, HRMS_NRF24L01_NSS_PIN);
  hrms_gpio_config_output((uint32_t)HRMS_NRF24L01_CE_PORT, HRMS_NRF24L01_CE_PIN);
  hrms_gpio_config_input_pullup((uint32_t)HRMS_NRF24L01_IRQ_PORT, HRMS_NRF24L01_IRQ_PIN);
  
  // Initialize SPI pins
  hrms_gpio_config_alternate_pushpull((uint32_t)HRMS_NRF24L01_SPI_PORT, HRMS_NRF24L01_SCK_PIN);
  hrms_gpio_config_alternate_pushpull((uint32_t)HRMS_NRF24L01_SPI_PORT, HRMS_NRF24L01_MISO_PIN);
  hrms_gpio_config_alternate_pushpull((uint32_t)HRMS_NRF24L01_SPI_PORT, HRMS_NRF24L01_MOSI_PIN);
  
  // Set initial pin states
  nrf24l01_cs_high();
  nrf24l01_ce_low();
  
  // Initialize SPI interface (using SPI1 as fallback)
  if (!hrms_spi1_init()) {
    return false;
  }
  
  // Wait for module to power up
  hrms_delay_ms(100);
  
  // Reset the module
  nrf24l01_write_register(NRF24L01_REG_CONFIG, 0x08); // Power down
  hrms_delay_ms(5);
  
  // Check if module is responding by reading a known register
  uint8_t test_value = nrf24l01_read_register(NRF24L01_REG_CONFIG);
  if (test_value == 0xFF || test_value == 0x00) {
    return false; // Module not responding
  }
  
  // Clear interrupt flags
  hrms_nrf24l01_clear_interrupts();
  
  // Flush TX and RX FIFOs
  nrf24l01_cs_low();
  nrf24l01_spi_transfer(NRF24L01_CMD_FLUSH_TX);
  nrf24l01_cs_high();
  
  nrf24l01_cs_low();
  nrf24l01_spi_transfer(NRF24L01_CMD_FLUSH_RX);
  nrf24l01_cs_high();
  
  is_listening = false;
  
  return true;
}

bool hrms_nrf24l01_configure(const nrf24l01_config_t *config) {
  if (!config) {
    return false;
  }
  
  // Store current configuration
  memcpy(&current_config, config, sizeof(nrf24l01_config_t));
  
  // Power down for configuration
  hrms_nrf24l01_power_down();
  hrms_delay_ms(5);
  
  // Set RF channel
  nrf24l01_write_register(NRF24L01_REG_RF_CH, config->channel);
  
  // Set RF setup (power and data rate)
  uint8_t rf_setup = 0;
  switch (config->power) {
    case NRF24L01_POWER_0DBM:    rf_setup |= 0x06; break;
    case NRF24L01_POWER_NEG6DBM: rf_setup |= 0x04; break;
    case NRF24L01_POWER_NEG12DBM: rf_setup |= 0x02; break;
    case NRF24L01_POWER_NEG18DBM: rf_setup |= 0x00; break;
  }
  
  switch (config->datarate) {
    case NRF24L01_DATARATE_1MBPS:   rf_setup |= 0x00; break;
    case NRF24L01_DATARATE_2MBPS:   rf_setup |= 0x08; break;
    case NRF24L01_DATARATE_250KBPS: rf_setup |= 0x20; break;
  }
  
  nrf24l01_write_register(NRF24L01_REG_RF_SETUP, rf_setup);
  
  // Set address width (5 bytes)
  nrf24l01_write_register(NRF24L01_REG_SETUP_AW, 0x03);
  
  // Set auto-retransmit
  uint8_t setup_retr = (config->retry_delay << 4) | (config->retries & 0x0F);
  nrf24l01_write_register(NRF24L01_REG_SETUP_RETR, setup_retr);
  
  // Set addresses
  nrf24l01_write_register_multi(NRF24L01_REG_RX_ADDR_P0, config->address, NRF24L01_ADDR_WIDTH);
  nrf24l01_write_register_multi(NRF24L01_REG_TX_ADDR, config->address, NRF24L01_ADDR_WIDTH);
  
  // Set payload sizes
  nrf24l01_write_register(NRF24L01_REG_RX_PW_P0, NRF24L01_MAX_PAYLOAD_SIZE);
  
  // Enable auto-acknowledgment
  if (config->auto_ack) {
    nrf24l01_write_register(NRF24L01_REG_EN_AA, 0x01); // Enable for pipe 0
  } else {
    nrf24l01_write_register(NRF24L01_REG_EN_AA, 0x00); // Disable
  }
  
  // Enable RX pipe 0
  nrf24l01_write_register(NRF24L01_REG_EN_RXADDR, 0x01);
  
  // Power up
  hrms_nrf24l01_power_up();
  
  return true;
}

bool hrms_nrf24l01_send(const uint8_t *data, uint8_t length) {
  if (!data || length == 0 || length > NRF24L01_MAX_PAYLOAD_SIZE) {
    return false;
  }
  
  // Switch to TX mode
  hrms_nrf24l01_stop_listening();
  
  // Clear TX interrupt flags
  nrf24l01_write_register(NRF24L01_REG_STATUS, NRF24L01_STATUS_TX_DS | NRF24L01_STATUS_MAX_RT);
  
  // Load payload
  nrf24l01_cs_low();
  nrf24l01_spi_transfer(NRF24L01_CMD_W_TX_PAYLOAD);
  for (uint8_t i = 0; i < length; i++) {
    nrf24l01_spi_transfer(data[i]);
  }
  nrf24l01_cs_high();
  
  // Pulse CE to start transmission
  nrf24l01_ce_high();
  hrms_delay_us(15); // Minimum 10us pulse
  nrf24l01_ce_low();
  
  // Wait for transmission to complete (timeout after 10ms)
  uint32_t timeout = 10000; // 10ms in microseconds
  uint8_t status;
  
  while (timeout > 0) {
    status = hrms_nrf24l01_get_status();
    if (status & (NRF24L01_STATUS_TX_DS | NRF24L01_STATUS_MAX_RT)) {
      break;
    }
    hrms_delay_us(10);
    timeout -= 10;
  }
  
  // Clear interrupt flags
  nrf24l01_write_register(NRF24L01_REG_STATUS, NRF24L01_STATUS_TX_DS | NRF24L01_STATUS_MAX_RT);
  
  // Check if transmission was successful
  bool success = (status & NRF24L01_STATUS_TX_DS) != 0;
  
  return success;
}

bool hrms_nrf24l01_available(void) {
  uint8_t status = hrms_nrf24l01_get_status();
  return (status & NRF24L01_STATUS_RX_DR) != 0;
}

uint8_t hrms_nrf24l01_receive(uint8_t *data, uint8_t max_length) {
  if (!data || max_length == 0) {
    return 0;
  }
  
  if (!hrms_nrf24l01_available()) {
    return 0;
  }
  
  // Read payload size
  uint8_t payload_size = NRF24L01_MAX_PAYLOAD_SIZE;
  if (payload_size > max_length) {
    payload_size = max_length;
  }
  
  // Read payload
  nrf24l01_cs_low();
  nrf24l01_spi_transfer(NRF24L01_CMD_R_RX_PAYLOAD);
  for (uint8_t i = 0; i < payload_size; i++) {
    data[i] = nrf24l01_spi_transfer(NRF24L01_CMD_NOP);
  }
  nrf24l01_cs_high();
  
  // Clear RX interrupt flag
  nrf24l01_write_register(NRF24L01_REG_STATUS, NRF24L01_STATUS_RX_DR);
  
  return payload_size;
}

void hrms_nrf24l01_start_listening(void) {
  if (!is_listening) {
    // Set to receive mode
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    config |= NRF24L01_CONFIG_PRIM_RX | NRF24L01_CONFIG_PWR_UP;
    nrf24l01_write_register(NRF24L01_REG_CONFIG, config);
    
    // Set CE high to enable RX
    nrf24l01_ce_high();
    
    // Wait for mode change
    hrms_delay_us(150);
    
    is_listening = true;
  }
}

void hrms_nrf24l01_stop_listening(void) {
  if (is_listening) {
    // Set CE low
    nrf24l01_ce_low();
    
    // Set to transmit mode
    uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
    config &= ~NRF24L01_CONFIG_PRIM_RX;
    config |= NRF24L01_CONFIG_PWR_UP;
    nrf24l01_write_register(NRF24L01_REG_CONFIG, config);
    
    // Wait for mode change
    hrms_delay_us(150);
    
    is_listening = false;
  }
}

bool hrms_nrf24l01_is_listening(void) {
  return is_listening;
}

uint8_t hrms_nrf24l01_get_status(void) {
  nrf24l01_cs_low();
  uint8_t status = nrf24l01_spi_transfer(NRF24L01_CMD_NOP);
  nrf24l01_cs_high();
  return status;
}

void hrms_nrf24l01_clear_interrupts(void) {
  nrf24l01_write_register(NRF24L01_REG_STATUS, 
    NRF24L01_STATUS_RX_DR | NRF24L01_STATUS_TX_DS | NRF24L01_STATUS_MAX_RT);
}

void hrms_nrf24l01_power_down(void) {
  nrf24l01_ce_low();
  uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
  config &= ~NRF24L01_CONFIG_PWR_UP;
  nrf24l01_write_register(NRF24L01_REG_CONFIG, config);
  is_listening = false;
}

void hrms_nrf24l01_power_up(void) {
  uint8_t config = nrf24l01_read_register(NRF24L01_REG_CONFIG);
  config |= NRF24L01_CONFIG_PWR_UP;
  nrf24l01_write_register(NRF24L01_REG_CONFIG, config);
  hrms_delay_ms(2); // Wait for power up
}

// Low-level helper functions
static uint8_t nrf24l01_spi_transfer(uint8_t data) {
  return hrms_spi1_transfer(data);
}

static void nrf24l01_cs_low(void) {
  hrms_gpio_clear_pin((uint32_t)HRMS_NRF24L01_SPI_PORT, HRMS_NRF24L01_NSS_PIN);
}

static void nrf24l01_cs_high(void) {
  hrms_gpio_set_pin((uint32_t)HRMS_NRF24L01_SPI_PORT, HRMS_NRF24L01_NSS_PIN);
}

static void nrf24l01_ce_low(void) {
  hrms_gpio_clear_pin((uint32_t)HRMS_NRF24L01_CE_PORT, HRMS_NRF24L01_CE_PIN);
}

static void nrf24l01_ce_high(void) {
  hrms_gpio_set_pin((uint32_t)HRMS_NRF24L01_CE_PORT, HRMS_NRF24L01_CE_PIN);
}

static uint8_t nrf24l01_read_register(uint8_t reg) {
  nrf24l01_cs_low();
  nrf24l01_spi_transfer(NRF24L01_CMD_R_REGISTER | reg);
  uint8_t value = nrf24l01_spi_transfer(NRF24L01_CMD_NOP);
  nrf24l01_cs_high();
  return value;
}

static void nrf24l01_write_register(uint8_t reg, uint8_t value) {
  nrf24l01_cs_low();
  nrf24l01_spi_transfer(NRF24L01_CMD_W_REGISTER | reg);
  nrf24l01_spi_transfer(value);
  nrf24l01_cs_high();
}

static void nrf24l01_write_register_multi(uint8_t reg, const uint8_t *data, uint8_t length) {
  nrf24l01_cs_low();
  nrf24l01_spi_transfer(NRF24L01_CMD_W_REGISTER | reg);
  for (uint8_t i = 0; i < length; i++) {
    nrf24l01_spi_transfer(data[i]);
  }
  nrf24l01_cs_high();
}

