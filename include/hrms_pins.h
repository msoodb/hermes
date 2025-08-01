/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_PINS_H
#define HRMS_PINS_H

// =============================
// === ACTUATORS ===============
// =============================


/* --- LED MODULE --- */
#define HRMS_LED_ONBOARD_PORT GPIOC
#define HRMS_LED_ONBOARD_PIN 13

#define HRMS_LED_EXTERNAL_PORT GPIOB
#define HRMS_LED_EXTERNAL_PIN 5

#define HRMS_LED_DEBUG_PORT GPIOC
#define HRMS_LED_DEBUG_PIN 15


/* --- ALARM MODULE --- */
#define HRMS_BUZZER_PORT GPIOC
#define HRMS_BUZZER_PIN 14

// =============================
// === SENSORS =================
// =============================

/* --- POTENTIOMETER MODULE --- */
#define HRMS_POTENTIOMETER_PORT GPIOA
#define HRMS_POTENTIOMETER_PIN 6
#define HRMS_POTENTIOMETER_ADC_CHANNEL 6


/* --- MODE BUTTON MODULE --- */
#define HRMS_BUTTON_PORT GPIOA
#define HRMS_BUTTON_PIN 4

/* --- JOYSTICK MODULE --- */
#define HRMS_JOYSTICK_VRX_PORT GPIOA
#define HRMS_JOYSTICK_VRX_PIN 5
#define HRMS_JOYSTICK_VRX_ADC_CHANNEL 5

#define HRMS_JOYSTICK_VRY_PORT GPIOA
#define HRMS_JOYSTICK_VRY_PIN 6
#define HRMS_JOYSTICK_VRY_ADC_CHANNEL 6

#define HRMS_JOYSTICK_SW_PORT GPIOA
#define HRMS_JOYSTICK_SW_PIN 7

// =============================
// === PROTOCOLS ===============
// =============================

/* --- PWM PROTOCOL --- */

/* --- SPI1 PROTOCOL --- */
#define HRMS_SPI1_NSS_PORT  GPIOA
#define HRMS_SPI1_NSS_PIN   4

#define HRMS_SPI1_SCK_PORT  GPIOA
#define HRMS_SPI1_SCK_PIN   5

#define HRMS_SPI1_MISO_PORT GPIOA
#define HRMS_SPI1_MISO_PIN  6

#define HRMS_SPI1_MOSI_PORT GPIOA
#define HRMS_SPI1_MOSI_PIN  7

/* --- SX1278 LORA MODULE --- */
#define HRMS_LORA_SPI_PORT       SPI1
#define HRMS_LORA_NSS_PORT       GPIOB
#define HRMS_LORA_NSS_PIN        0
#define HRMS_LORA_RESET_PORT     GPIOB
#define HRMS_LORA_RESET_PIN      1
#define HRMS_LORA_DIO0_PORT      GPIOB
#define HRMS_LORA_DIO0_PIN       10

/* --- I2C1 PROTOCOL --- */
#define HRMS_I2C1_SCL_PORT GPIOB
#define HRMS_I2C1_SCL_PIN 6

#define HRMS_I2C1_SDA_PORT GPIOB
#define HRMS_I2C1_SDA_PIN 7




// =============================
// === COMMUNICATIONS ==========
// =============================

/* --- nRF24L01 MODULE --- */
// SPI2 pins: PB12=NSS, PB13=SCK, PB14=MISO, PB15=MOSI
#define HRMS_NRF24L01_SPI_PORT GPIOB
#define HRMS_NRF24L01_NSS_PIN 12  // Chip Select
#define HRMS_NRF24L01_SCK_PIN 13  // SPI Clock
#define HRMS_NRF24L01_MISO_PIN 14 // Master In Slave Out
#define HRMS_NRF24L01_MOSI_PIN 15 // Master Out Slave In

// Control pins
#define HRMS_NRF24L01_CE_PORT GPIOB
#define HRMS_NRF24L01_CE_PIN 11   // Chip Enable

#define HRMS_NRF24L01_IRQ_PORT GPIOB
#define HRMS_NRF24L01_IRQ_PIN 10  // Interrupt Request (optional)

/* --- Future LoRa SX1278 MODULE --- */
// Placeholder for future implementation
// Will use SPI1: PA4=NSS, PA5=SCK, PA6=MISO, PA7=MOSI
// Additional pins: PA8=RESET, PA9=DIO0, PA10=DIO1

#endif /* HRMS_PINS_H */
