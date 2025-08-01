/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

/**
 * @file hrms_taskmanager.c
 * @brief FreeRTOS task setup and startup manager.
 *
 * Registers and launches key tasks, such as sensor polling,
 * actuator control, and safety monitoring.
 */

#include "hrms_taskmanager.h"
#include "hrms_gpio.h"
#include "hrms_joystick.h"
#include "hrms_pins.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "hrms_actuator_hub.h"
#include "hrms_controller.h"
#include "hrms_sensor_hub.h"

#include "hrms_mode_button.h"
#include "hrms_communication_hub.h"

// --- Task declarations ---
static void vSensorHubTask(void *pvParameters);
static void vControllerTask(void *pvParameters);
static void vActuatorHubTask(void *pvParameters);
static void vCommunicationHubTask(void *pvParameters);

// --- Event Handlers ---
static void handle_sensor_data(void);
static void handle_mode_button_event(void);

// ESP32 handlers removed

// --- Task and queue settings ---
#define SENSOR_HUB_TASK_STACK 256
#define CONTROLLER_TASK_STACK 256
#define ACTUATOR_HUB_TASK_STACK 256
#define COMMUNICATION_HUB_TASK_STACK 256

#define SENSOR_HUB_TASK_PRIORITY 2
#define CONTROLLER_TASK_PRIORITY 2
#define ACTUATOR_HUB_TASK_PRIORITY 2
#define COMMUNICATION_HUB_TASK_PRIORITY 1

// --- Queues ---
static QueueHandle_t xSensorDataQueue = NULL;
static QueueHandle_t xActuatorCmdQueue = NULL;
static QueueHandle_t xModeButtonQueue = NULL;
static QueueSetHandle_t xControllerQueueSet = NULL;

void hrms_taskmanager_setup(void) {

  xSensorDataQueue = xQueueCreate(5, sizeof(hrms_sensor_data_t));
  configASSERT(xSensorDataQueue != NULL);

  xActuatorCmdQueue = xQueueCreate(5, sizeof(hrms_actuator_command_t));
  configASSERT(xActuatorCmdQueue != NULL);


  xModeButtonQueue = xQueueCreate(5, sizeof(hrms_mode_button_event_t));
  configASSERT(xModeButtonQueue != NULL);

  // Queue set
  xControllerQueueSet = xQueueCreateSet(10);
  configASSERT(xControllerQueueSet != NULL);
  xQueueAddToSet(xSensorDataQueue, xControllerQueueSet);
  xQueueAddToSet(xModeButtonQueue, xControllerQueueSet);

  // Init all modules
  hrms_sensor_hub_init();
  hrms_actuator_hub_init();
  hrms_controller_init();
  hrms_communication_hub_init();
  hrms_mode_button_init(xModeButtonQueue);

  // Tasks (always run sensor and actuator hub)
  xTaskCreate(vSensorHubTask, "SensorHub", SENSOR_HUB_TASK_STACK, NULL,
              SENSOR_HUB_TASK_PRIORITY, NULL);

  xTaskCreate(vControllerTask, "Controller", CONTROLLER_TASK_STACK, NULL,
              CONTROLLER_TASK_PRIORITY, NULL);

  xTaskCreate(vActuatorHubTask, "ActuatorHub", ACTUATOR_HUB_TASK_STACK, NULL,
              ACTUATOR_HUB_TASK_PRIORITY, NULL);

  xTaskCreate(vCommunicationHubTask, "CommHub", COMMUNICATION_HUB_TASK_STACK,
              NULL, COMMUNICATION_HUB_TASK_PRIORITY, NULL);
}

void hrms_taskmanager_start(void) { vTaskStartScheduler(); }

// --- Tasks ---
static void vSensorHubTask(void *pvParameters) {
  (void)pvParameters;
  hrms_sensor_data_t sensor_data;

  static bool last_joy_button = false;

  for (;;) {
    if (hrms_sensor_hub_read(&sensor_data)) {
      xQueueSendToBack(xSensorDataQueue, &sensor_data, 0);
    }

    // Test joystick button - toggle debug LED on button press
    hrms_joystick_event_t joy_event;
    hrms_joystick_check_events(&joy_event);

    if (joy_event.event_occurred && joy_event.button_pressed &&
        !last_joy_button) {
      // Button was just pressed - toggle debug LED
      hrms_gpio_toggle_pin((uint32_t)HRMS_LED_DEBUG_PORT, HRMS_LED_DEBUG_PIN);
    }
    last_joy_button = joy_event.button_pressed;

    vTaskDelay(pdMS_TO_TICKS(200)); // Slower sensor reading for stability
  }
}

static void vControllerTask(void *pvParameters) {
  (void)pvParameters;

  // hrms_actuator_command_t command;

  for (;;) {
    QueueSetMemberHandle_t activated =
        xQueueSelectFromSet(xControllerQueueSet, pdMS_TO_TICKS(100));

    if (activated == NULL) {
      continue;
    }

    if (activated == xSensorDataQueue) {
      handle_sensor_data();
    } else if (activated == xModeButtonQueue) {
      handle_mode_button_event();
    }
  }
}

static void vActuatorHubTask(void *pvParameters) {
  (void)pvParameters;

  hrms_actuator_command_t command;

  for (;;) {
    if (xQueueReceive(xActuatorCmdQueue, &command, pdMS_TO_TICKS(10)) ==
        pdPASS) {
      hrms_actuator_hub_apply(&command);
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// --- Event Handlers ---
static void handle_sensor_data(void) {
  hrms_sensor_data_t sensor_data;
  hrms_actuator_command_t command;

  if (xQueueReceive(xSensorDataQueue, &sensor_data, 0) == pdPASS) {
    hrms_controller_process(&sensor_data, &command);
    xQueueSendToBack(xActuatorCmdQueue, &command, 0);
  }
}


static void handle_mode_button_event(void) {
  hrms_mode_button_event_t event;
  hrms_actuator_command_t command;

  if (xQueueReceive(xModeButtonQueue, &event, 0) == pdPASS) {
    hrms_controller_process_mode_button(&event, &command);
    xQueueSendToBack(xActuatorCmdQueue, &command, 0);
  }
}

static void vCommunicationHubTask(void *pvParameters) {
  (void)pvParameters;

  hrms_comm_packet_t packet;

  for (;;) {
    // Process communication hub
    hrms_communication_hub_process();

    // Check for received packets
    if (hrms_communication_hub_receive(&packet)) {
      // Handle received packet based on type or forward to controller
      // For now, just process it in the communication hub
    }

    vTaskDelay(pdMS_TO_TICKS(50)); // 50ms cycle time
  }
}
