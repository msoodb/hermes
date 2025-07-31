
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_config.h"

#include "hrms_led.h"
#include "FreeRTOS.h"
#include "hrms_gpio.h"
#include "hrms_types.h"
#include "queue.h"
#include "stm32f1xx.h"
#include "task.h"
#include "hrms_pins.h"

// Internal state for blinking
#define LED_TASK_INTERVAL_MS 10

#define LED_TASK_STACK_SIZE 256
#define LED_TASK_PRIORITY 2
#define LED_QUEUE_LENGTH 1

static void hrms_led_external_on(void);
static void hrms_led_external_off(void);
static void vLedTask(void *pvParameters);

static QueueHandle_t led_command_queue = NULL;
static TaskHandle_t led_task_handle = NULL;

void hrms_led_init(void) {
  hrms_gpio_config_output((uint32_t)HRMS_LED_ONBOARD_PORT, HRMS_LED_ONBOARD_PIN);
  hrms_gpio_config_output((uint32_t)HRMS_LED_EXTERNAL_PORT, HRMS_LED_EXTERNAL_PIN);
  hrms_gpio_config_output((uint32_t)HRMS_LED_DEBUG_PORT, HRMS_LED_DEBUG_PIN);
  //hrms_gpio_set_pin((uint32_t)HRMS_LED_DEBUG_PORT, HRMS_LED_DEBUG_PIN);
      
  if (led_command_queue == NULL) {
    led_command_queue =
        xQueueCreate(LED_QUEUE_LENGTH, sizeof(hrms_led_command_t));
    configASSERT(led_command_queue != NULL);
  }

  if (led_task_handle == NULL) {
    BaseType_t result = xTaskCreate(vLedTask, "LEDTask", LED_TASK_STACK_SIZE,
                                    NULL, LED_TASK_PRIORITY, &led_task_handle);
    configASSERT(result == pdPASS);
    if (result != pdPASS) {}
  }
}

void hrms_led_apply(const hrms_led_command_t *cmd) {
  if (!cmd || !led_command_queue)
    return;
  xQueueOverwrite(led_command_queue, cmd);
}

static void hrms_led_external_on(void) {
  hrms_gpio_set_pin((uint32_t)HRMS_LED_EXTERNAL_PORT, HRMS_LED_EXTERNAL_PIN);
}

static void hrms_led_external_off(void) {
  hrms_gpio_clear_pin((uint32_t)HRMS_LED_EXTERNAL_PORT, HRMS_LED_EXTERNAL_PIN);
}

static void vLedTask(void *pvParameters) {
  (void)pvParameters;
  hrms_led_command_t current_cmd = {.mode = HRMS_LED_MODE_OFF,
				    .blink_speed_ms = 200};
  TickType_t last_toggle_tick = xTaskGetTickCount();
  bool led_state = false;

  for (;;) {
    hrms_led_command_t received_cmd;

    // Check for new command (non-blocking)
    if (xQueueReceive(led_command_queue, &received_cmd, 0) == pdPASS) {
      current_cmd = received_cmd;
    }

    switch (current_cmd.mode) {
    case HRMS_LED_MODE_OFF:
      hrms_led_external_off();
      led_state = false;
      break;

    case HRMS_LED_MODE_ON:
      hrms_led_external_on();
      led_state = true;
      break;

    case HRMS_LED_MODE_BLINK: {
      TickType_t now = xTaskGetTickCount();
      if ((now - last_toggle_tick) >=
          pdMS_TO_TICKS(current_cmd.blink_speed_ms)) {
        led_state = !led_state;
        if (led_state) {
          hrms_led_external_on();
        } else {
          hrms_led_external_off();
        }
        last_toggle_tick = now;
      }
      break;
    }

    default:
      hrms_led_external_off();
      led_state = false;
      break;
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

