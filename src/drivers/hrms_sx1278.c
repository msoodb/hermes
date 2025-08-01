/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Hermes.
 *
 * Hermes is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_sx1278.h"
#include "hrms_pins.h"
#include "hrms_gpio.h"
#include "hrms_spi.h"
#include "hrms_delay.h"
#include "FreeRTOS.h"
#include "task.h"

// SX1278 Register definitions
#define SX1278_REG_FIFO                 0x00
#define SX1278_REG_OP_MODE              0x01
#define SX1278_REG_FRF_MSB              0x06
#define SX1278_REG_FRF_MID              0x07
#define SX1278_REG_FRF_LSB              0x08
#define SX1278_REG_PA_CONFIG            0x09
#define SX1278_REG_LR_DATARATE          0x1E
#define SX1278_REG_LR_MODEMCONFIG1      0x1D
#define SX1278_REG_LR_MODEMCONFIG2      0x1E
#define SX1278_REG_LR_SYMBTIMEOUTLSB    0x1F
#define SX1278_REG_LR_PREAMBLELENGTH    0x20
#define SX1278_REG_LR_PAYLOADLENGTH     0x22
#define SX1278_REG_LR_MODEMCONFIG3      0x26
#define SX1278_REG_LR_PPMCORRECTION     0x27
#define SX1278_REG_LR_IRQFLAGS          0x12
#define SX1278_REG_LR_IRQFLAGSMASK      0x11
#define SX1278_REG_LR_FIFORXBASEADDR    0x0F
#define SX1278_REG_LR_FIFOTXBASEADDR    0x0E
#define SX1278_REG_LR_FIFOADDRPTR       0x0D
#define SX1278_REG_LR_FIFORXCURRENTADDR 0x10
#define SX1278_REG_LR_RXNBBYTES         0x13
#define SX1278_REG_VERSION              0x42

// Operation modes
#define SX1278_MODE_SLEEP               0x00
#define SX1278_MODE_STANDBY             0x01
#define SX1278_MODE_TX                  0x03
#define SX1278_MODE_RXCONTINUOUS        0x05
#define SX1278_MODE_LORA                0x80

// IRQ flags
#define SX1278_IRQ_RXTIMEOUT            0x80
#define SX1278_IRQ_RXDONE               0x40
#define SX1278_IRQ_PAYLOADCRCERROR      0x20
#define SX1278_IRQ_VALIDHEADER          0x10
#define SX1278_IRQ_TXDONE               0x08
#define SX1278_IRQ_CADDONE              0x04
#define SX1278_IRQ_FHSSCHANGEDCHANNEL   0x02
#define SX1278_IRQ_CADDETECTED          0x01

static bool initialized = false;

// Low-level SPI functions
static uint8_t sx1278_read_register(uint8_t addr) {
  uint8_t result;
  hrms_gpio_clear_pin((uint32_t)HRMS_LORA_NSS_PORT, HRMS_LORA_NSS_PIN);
  hrms_spi1_transfer(addr & 0x7F); // Read bit = 0
  result = hrms_spi1_transfer(0x00);
  hrms_gpio_set_pin((uint32_t)HRMS_LORA_NSS_PORT, HRMS_LORA_NSS_PIN);
  return result;
}

static void sx1278_write_register(uint8_t addr, uint8_t data) {
  hrms_gpio_clear_pin((uint32_t)HRMS_LORA_NSS_PORT, HRMS_LORA_NSS_PIN);
  hrms_spi1_transfer(addr | 0x80); // Write bit = 1
  hrms_spi1_transfer(data);
  hrms_gpio_set_pin((uint32_t)HRMS_LORA_NSS_PORT, HRMS_LORA_NSS_PIN);
}

static void sx1278_reset(void) {
  hrms_gpio_clear_pin((uint32_t)HRMS_LORA_RESET_PORT, HRMS_LORA_RESET_PIN);
  hrms_delay_ms(10);
  hrms_gpio_set_pin((uint32_t)HRMS_LORA_RESET_PORT, HRMS_LORA_RESET_PIN);
  hrms_delay_ms(10);
}

bool hrms_sx1278_init(void) {
  if (initialized) {
    return true;
  }
  
  // Initialize SPI
  if (hrms_spi1_init() != 0) {
    return false;
  }
  
  // Configure control pins
  hrms_gpio_config_output((uint32_t)HRMS_LORA_NSS_PORT, HRMS_LORA_NSS_PIN);
  hrms_gpio_config_output((uint32_t)HRMS_LORA_RESET_PORT, HRMS_LORA_RESET_PIN);
  hrms_gpio_config_input((uint32_t)HRMS_LORA_DIO0_PORT, HRMS_LORA_DIO0_PIN);
  
  // Set NSS high (inactive)
  hrms_gpio_set_pin((uint32_t)HRMS_LORA_NSS_PORT, HRMS_LORA_NSS_PIN);
  
  // Reset the module
  sx1278_reset();
  
  // Check version register
  uint8_t version = sx1278_read_register(SX1278_REG_VERSION);
  if (version != 0x12) { // SX1278 version should be 0x12
    return false;
  }
  
  // Set to sleep mode
  sx1278_write_register(SX1278_REG_OP_MODE, SX1278_MODE_SLEEP);
  vTaskDelay(pdMS_TO_TICKS(10));
  
  // Set to LoRa mode
  sx1278_write_register(SX1278_REG_OP_MODE, SX1278_MODE_SLEEP | SX1278_MODE_LORA);
  vTaskDelay(pdMS_TO_TICKS(10));
  
  // Set to standby mode
  sx1278_write_register(SX1278_REG_OP_MODE, SX1278_MODE_STANDBY | SX1278_MODE_LORA);
  vTaskDelay(pdMS_TO_TICKS(10));
  
  initialized = true;
  return true;
}

bool hrms_sx1278_configure(const sx1278_config_t *config) {
  if (!initialized || !config) {
    return false;
  }
  
  // Set frequency - avoid 64-bit division
  // Formula: FRF = (Freq * 2^19) / 32MHz
  // For 433MHz: FRF = (433 * 524288) / 32000000 = 7098368 = 0x6C4000
  uint32_t frf = (config->frequency / 1000) * 524; // Approximate calculation avoiding 64-bit division
  if (config->frequency == 433000000) frf = 0x6C4000; // Known value for 433MHz
  if (config->frequency == 868000000) frf = 0xD84000; // Known value for 868MHz
  if (config->frequency == 915000000) frf = 0xE40000; // Known value for 915MHz
  
  sx1278_write_register(SX1278_REG_FRF_MSB, (uint8_t)(frf >> 16));
  sx1278_write_register(SX1278_REG_FRF_MID, (uint8_t)(frf >> 8));
  sx1278_write_register(SX1278_REG_FRF_LSB, (uint8_t)(frf >> 0));
  
  // Set TX power
  if (config->tx_power > 17) {
    sx1278_write_register(SX1278_REG_PA_CONFIG, 0xFF); // Max power
  } else if (config->tx_power < 2) {
    sx1278_write_register(SX1278_REG_PA_CONFIG, 0x80 | 0); // Min power
  } else {
    sx1278_write_register(SX1278_REG_PA_CONFIG, 0x80 | (config->tx_power - 2));
  }
  
  // Configure modem settings
  uint8_t bw = config->bandwidth & 0x0F;
  uint8_t cr = config->coding_rate & 0x07;
  uint8_t implicit_header = 0; // Explicit header mode
  
  sx1278_write_register(SX1278_REG_LR_MODEMCONFIG1, (bw << 4) | (cr << 1) | implicit_header);
  
  uint8_t sf = config->spreading_factor & 0x0F;
  uint8_t crc_on = config->crc_enable ? 1 : 0;
  sx1278_write_register(SX1278_REG_LR_MODEMCONFIG2, (sf << 4) | (crc_on << 2));
  
  // Set preamble length
  sx1278_write_register(SX1278_REG_LR_PREAMBLELENGTH + 1, (uint8_t)(config->preamble_length >> 8));
  sx1278_write_register(SX1278_REG_LR_PREAMBLELENGTH, (uint8_t)(config->preamble_length & 0xFF));
  
  // Set FIFO base addresses
  sx1278_write_register(SX1278_REG_LR_FIFOTXBASEADDR, 0x00);
  sx1278_write_register(SX1278_REG_LR_FIFORXBASEADDR, 0x00);
  
  return true;
}

bool hrms_sx1278_send(const uint8_t *data, size_t len) {
  if (!initialized || !data || len == 0 || len > 255) {
    return false;
  }
  
  // Set to standby mode
  sx1278_write_register(SX1278_REG_OP_MODE, SX1278_MODE_STANDBY | SX1278_MODE_LORA);
  
  // Set FIFO address pointer to TX base
  sx1278_write_register(SX1278_REG_LR_FIFOADDRPTR, 0x00);
  
  // Write payload length
  sx1278_write_register(SX1278_REG_LR_PAYLOADLENGTH, (uint8_t)len);
  
  // Write data to FIFO
  hrms_gpio_clear_pin((uint32_t)HRMS_LORA_NSS_PORT, HRMS_LORA_NSS_PIN);
  hrms_spi1_transfer(SX1278_REG_FIFO | 0x80);
  for (size_t i = 0; i < len; i++) {
    hrms_spi1_transfer(data[i]);
  }
  hrms_gpio_set_pin((uint32_t)HRMS_LORA_NSS_PORT, HRMS_LORA_NSS_PIN);
  
  // Clear IRQ flags
  sx1278_write_register(SX1278_REG_LR_IRQFLAGS, 0xFF);
  
  // Set to TX mode
  sx1278_write_register(SX1278_REG_OP_MODE, SX1278_MODE_TX | SX1278_MODE_LORA);
  
  return true;
}

bool hrms_sx1278_available(void) {
  if (!initialized) {
    return false;
  }
  
  uint8_t irq_flags = sx1278_read_register(SX1278_REG_LR_IRQFLAGS);
  return (irq_flags & SX1278_IRQ_RXDONE) != 0;
}

uint8_t hrms_sx1278_receive(uint8_t *data, size_t max_len) {
  if (!initialized || !data || max_len == 0) {
    return 0;
  }
  
  uint8_t irq_flags = sx1278_read_register(SX1278_REG_LR_IRQFLAGS);
  
  if (!(irq_flags & SX1278_IRQ_RXDONE)) {
    return 0; // No data available
  }
  
  // Clear IRQ flags
  sx1278_write_register(SX1278_REG_LR_IRQFLAGS, 0xFF);
  
  // Check for CRC error
  if (irq_flags & SX1278_IRQ_PAYLOADCRCERROR) {
    return 0; // CRC error
  }
  
  // Get payload length
  uint8_t payload_length = sx1278_read_register(SX1278_REG_LR_RXNBBYTES);
  if (payload_length > max_len) {
    payload_length = max_len;
  }
  
  // Get FIFO RX current address
  uint8_t fifo_addr = sx1278_read_register(SX1278_REG_LR_FIFORXCURRENTADDR);
  sx1278_write_register(SX1278_REG_LR_FIFOADDRPTR, fifo_addr);
  
  // Read data from FIFO
  hrms_gpio_clear_pin((uint32_t)HRMS_LORA_NSS_PORT, HRMS_LORA_NSS_PIN);
  hrms_spi1_transfer(SX1278_REG_FIFO & 0x7F);
  for (uint8_t i = 0; i < payload_length; i++) {
    data[i] = hrms_spi1_transfer(0x00);
  }
  hrms_gpio_set_pin((uint32_t)HRMS_LORA_NSS_PORT, HRMS_LORA_NSS_PIN);
  
  return payload_length;
}

void hrms_sx1278_start_listening(void) {
  if (!initialized) {
    return;
  }
  
  // Clear IRQ flags
  sx1278_write_register(SX1278_REG_LR_IRQFLAGS, 0xFF);
  
  // Set FIFO RX base address
  sx1278_write_register(SX1278_REG_LR_FIFOADDRPTR, 0x00);
  
  // Set to continuous RX mode
  sx1278_write_register(SX1278_REG_OP_MODE, SX1278_MODE_RXCONTINUOUS | SX1278_MODE_LORA);
}

void hrms_sx1278_stop_listening(void) {
  if (!initialized) {
    return;
  }
  
  sx1278_write_register(SX1278_REG_OP_MODE, SX1278_MODE_STANDBY | SX1278_MODE_LORA);
}

void hrms_sx1278_clear_interrupts(void) {
  if (!initialized) {
    return;
  }
  
  sx1278_write_register(SX1278_REG_LR_IRQFLAGS, 0xFF);
}

int16_t hrms_sx1278_get_rssi(void) {
  if (!initialized) {
    return -999;
  }
  
  uint8_t rssi_reg = sx1278_read_register(0x1A); // PacketRssi register
  return -164 + rssi_reg; // Convert to dBm
}

bool hrms_sx1278_is_transmitting(void) {
  if (!initialized) {
    return false;
  }
  
  uint8_t op_mode = sx1278_read_register(SX1278_REG_OP_MODE);
  return (op_mode & 0x07) == SX1278_MODE_TX;
}