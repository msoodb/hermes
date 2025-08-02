

/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */


#include "hrms_button.h"
#include "FreeRTOS.h"
#include "hrms_exti_dispatcher.h"
#include "hrms_gpio.h"
#include "hrms_pins.h"
#include "queue.h"
#include "stm32f1xx.h"

#define DEBOUNCE_DELAY_MS 50

static QueueHandle_t button_queue = NULL;
static uint32_t last_press_tick = 0;

static void button_exti_handler(void) {
  // Immediate visual feedback to test if EXTI is working
  hrms_gpio_toggle_pin((uint32_t)HRMS_LED_DEBUG_PORT, HRMS_LED_DEBUG_PIN);
  
  if (button_queue == NULL) {
    return;
  }

  uint32_t now = xTaskGetTickCountFromISR();
  if ((now - last_press_tick) < pdMS_TO_TICKS(DEBOUNCE_DELAY_MS)) {
    return; // Too soon, ignore as bounce
  }
  last_press_tick = now;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  uint32_t pin_state = hrms_gpio_read_pin((uint32_t)HRMS_BUTTON_PORT, HRMS_BUTTON_PIN);
  hrms_button_event_t event;

  if (pin_state) {
    event.event_type = HRMS_BUTTON_EVENT_RELEASED;
  } else {
    event.event_type = HRMS_BUTTON_EVENT_PRESSED;
  }

  event.timestamp = now;

  xQueueSendFromISR(button_queue, &event, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void hrms_button_init(QueueHandle_t controller_queue) {
  button_queue = controller_queue;

  // Enable AFIO clock for EXTI
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

  // Configure as input with Pull‑Up
  hrms_gpio_config_input_pullup((uint32_t)HRMS_BUTTON_PORT, HRMS_BUTTON_PIN);

  // EXTI mapping for PA0 (EXTI0)
  AFIO->EXTICR[0] &= ~(0xF << (4 * HRMS_BUTTON_PIN));
  AFIO->EXTICR[0] |= (0x0 << (4 * HRMS_BUTTON_PIN)); // 0x0 = GPIOA

  // Trigger both edges
  EXTI->IMR |= (1 << HRMS_BUTTON_PIN);
  EXTI->RTSR |= (1 << HRMS_BUTTON_PIN);
  EXTI->FTSR |= (1 << HRMS_BUTTON_PIN);

  // Register callback
  hrms_exti_register_callback(HRMS_BUTTON_PIN, button_exti_handler);

  // Enable IRQ
  NVIC_EnableIRQ(EXTI0_IRQn);
}

