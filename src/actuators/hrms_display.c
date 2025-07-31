/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 *
 * See LICENSE file for details.
 */

#include "hrms_config.h"

#include "FreeRTOS.h"
#include "task.h"
#include "hrms_display.h"
#include "hrms_gpio.h"
#include "stm32f1xx.h"
#include "hrms_pins.h"

static void lcd_pulse_enable(void);
static void lcd_write_nibble(uint8_t nibble);
static void lcd_busy_delay(void);
static void hrms_lcd_send_command(uint8_t cmd);
static void hrms_lcd_send_data(uint8_t data);
static void safe_delay_ms(uint32_t ms);

void hrms_display_init(void) {
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

  hrms_gpio_config_output((uint32_t)HRMS_LCD_RS_PORT, HRMS_LCD_RS_PIN);
  hrms_gpio_config_output((uint32_t)HRMS_LCD_E_PORT, HRMS_LCD_E_PIN);
  hrms_gpio_config_output((uint32_t)HRMS_LCD_D4_PORT, HRMS_LCD_D4_PIN);
  hrms_gpio_config_output((uint32_t)HRMS_LCD_D5_PORT, HRMS_LCD_D5_PIN);
  hrms_gpio_config_output((uint32_t)HRMS_LCD_D6_PORT, HRMS_LCD_D6_PIN);
  hrms_gpio_config_output((uint32_t)HRMS_LCD_D7_PORT, HRMS_LCD_D7_PIN);

  // Manual delay for power-on wait (~50ms)
  for (volatile int i = 0; i < 50000; i++)
    __asm__("nop");

  lcd_write_nibble(0x03);
  lcd_busy_delay();
  lcd_write_nibble(0x03);
  lcd_busy_delay();
  lcd_write_nibble(0x03);
  lcd_busy_delay();
  lcd_write_nibble(0x02); // Switch to 4-bit mode
  lcd_busy_delay();

  hrms_lcd_send_command(0x28); // Function set: 4-bit, 2-line, 5x8 font
  hrms_lcd_send_command(0x0C); // Display ON, cursor OFF
  hrms_lcd_send_command(0x06); // Entry mode set: Increment cursor
  hrms_lcd_send_command(0x01); // Clear display
  lcd_busy_delay();            // Wait manually instead of vTaskDelay

  // Optional test output
  hrms_lcd_send_data('H');
  hrms_lcd_send_data('E');
  hrms_lcd_send_data('L');
  hrms_lcd_send_data('L');
  hrms_lcd_send_data('O');
}

void hrms_display_apply(const hrms_display_command_t *cmd) {
 
  if (!cmd)
    return;

  hrms_lcd_send_command(0x01); // Clear display
  vTaskDelay(pdMS_TO_TICKS(2));

  hrms_lcd_send_command(0x80); // Line 1 start
  for (const char *p = cmd->line1; *p; p++) {
    hrms_lcd_send_data((uint8_t)*p);
  }

  hrms_lcd_send_command(0xC0); // Line 2 start
  for (const char *p = cmd->line2; *p; p++) {
    hrms_lcd_send_data((uint8_t)*p);
  }
}

static void lcd_pulse_enable(void) {
  HRMS_LCD_E_PORT->BSRR = (1 << HRMS_LCD_E_PIN); // E high
  for (volatile int i = 0; i < 200; i++)
    __asm__("nop");
  HRMS_LCD_E_PORT->BRR = (1 << HRMS_LCD_E_PIN); // E low
  for (volatile int i = 0; i < 200; i++)
    __asm__("nop");
}

static void lcd_write_nibble(uint8_t nibble) {
  HRMS_LCD_E_PORT->BRR = (1 << HRMS_LCD_D4_PIN) | (1 << HRMS_LCD_D5_PIN) | (1 << HRMS_LCD_D6_PIN) |
                  (1 << HRMS_LCD_D7_PIN);

  if (nibble & 0x01)
    HRMS_LCD_D4_PORT->BSRR = (1 << HRMS_LCD_D4_PIN);
  if (nibble & 0x02)
    HRMS_LCD_D5_PORT->BSRR = (1 << HRMS_LCD_D5_PIN);
  if (nibble & 0x04)
    HRMS_LCD_D6_PORT->BSRR = (1 << HRMS_LCD_D6_PIN);
  if (nibble & 0x08)
    HRMS_LCD_D7_PORT->BSRR = (1 << HRMS_LCD_D7_PIN);

  lcd_pulse_enable();
}

static void hrms_lcd_send_command(uint8_t cmd) {
  HRMS_LCD_E_PORT->BRR = (1 << HRMS_LCD_RS_PIN); // RS = 0 for command
  lcd_write_nibble(cmd >> 4);
  lcd_write_nibble(cmd & 0x0F);
  safe_delay_ms(2);
}

static void hrms_lcd_send_data(uint8_t data) {
  HRMS_LCD_E_PORT->BSRR = (1 << HRMS_LCD_RS_PIN); // RS = 1 for data
  lcd_write_nibble(data >> 4);
  lcd_write_nibble(data & 0x0F);
  safe_delay_ms(2);
}

static void lcd_busy_delay(void) {
  for (volatile int i = 0; i < 10000; i++)
    __asm__("nop");
}

static void safe_delay_ms(uint32_t ms) {
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    vTaskDelay(pdMS_TO_TICKS(ms));
  } else {
    // Blocking busy wait if scheduler not running
    for (volatile uint32_t i = 0; i < ms * 8000; i++) {
      __asm__("nop");
    }
  }
}

