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
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "hrms_actuator_hub.h"
#include "hrms_controller.h"
#include "hrms_sensor_hub.h"

#include "hrms_button.h"
#include "hrms_communication_hub.h"
#include "libc_stubs.h"

// --- Task declarations ---
static void vSensorHubTask(void *pvParameters);
static void vControllerTask(void *pvParameters);
static void vActuatorHubTask(void *pvParameters);
static void vCommunicationHubTask(void *pvParameters);

// --- Event Handlers ---
static void handle_sensor_data(void);
static void handle_button_event(void);

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
static QueueHandle_t xButtonEventQueue = NULL;
static QueueHandle_t xCommCmdQueue = NULL;
static QueueSetHandle_t xControllerQueueSet = NULL;

void hrms_taskmanager_setup(void) {

  xSensorDataQueue = xQueueCreate(5, sizeof(hrms_sensor_data_t));
  configASSERT(xSensorDataQueue != NULL);

  xActuatorCmdQueue = xQueueCreate(5, sizeof(hrms_actuator_command_t));
  configASSERT(xActuatorCmdQueue != NULL);

  xButtonEventQueue = xQueueCreate(5, sizeof(hrms_button_event_t));
  configASSERT(xButtonEventQueue != NULL);

  xCommCmdQueue = xQueueCreate(5, sizeof(hrms_comm_command_t));
  configASSERT(xCommCmdQueue != NULL);

  // Queue set
  xControllerQueueSet = xQueueCreateSet(10);
  configASSERT(xControllerQueueSet != NULL);
  xQueueAddToSet(xSensorDataQueue, xControllerQueueSet);
  xQueueAddToSet(xButtonEventQueue, xControllerQueueSet);

  // Init all modules
  hrms_sensor_hub_init();
  hrms_actuator_hub_init();
  hrms_controller_init();
  hrms_communication_hub_init();
  hrms_button_init(xButtonEventQueue);

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

  for (;;) {
    if (hrms_sensor_hub_read(&sensor_data)) {
      xQueueSendToBack(xSensorDataQueue, &sensor_data, 0);
    }

    vTaskDelay(pdMS_TO_TICKS(200)); // Slower sensor reading for stability
  }
}

static void vControllerTask(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    QueueSetMemberHandle_t activated =
        xQueueSelectFromSet(xControllerQueueSet, portMAX_DELAY);

    if (activated == NULL) {
      continue;
    }

    if (activated == xSensorDataQueue) {
      handle_sensor_data();
    } else if (activated == xButtonEventQueue) {
      handle_button_event();
    }
  }
}

static void vActuatorHubTask(void *pvParameters) {
  (void)pvParameters;
  hrms_actuator_command_t command;

  for (;;) {
    if (xQueueReceive(xActuatorCmdQueue, &command, pdMS_TO_TICKS(10)) ==
        pdPASS) {

      // Apply actuator commands (LED, OLED, Alarm)
      hrms_actuator_hub_apply(&command);
      
      // Forward communication commands to communication task queue
      if (command.comm.should_transmit) {
        xQueueSendToBack(xCommCmdQueue, &command.comm, 0);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

static void vCommunicationHubTask(void *pvParameters) {
  (void)pvParameters;

  hrms_comm_packet_t packet;
  hrms_comm_command_t comm_cmd;

  for (;;) {
    // Process communication hub (maintenance tasks)
    hrms_communication_hub_process();

    // Handle outgoing communication commands (TX)
    if (xQueueReceive(xCommCmdQueue, &comm_cmd, 0) == pdPASS) {
      if (comm_cmd.should_transmit) {
        hrms_communication_hub_send_joystick_data(&comm_cmd);
      }
    }

    // Check for incoming data (RX)
    uint8_t rx_buffer[sizeof(hrms_comm_packet_t)];
    size_t received_len = 0;
    if (hrms_communication_hub_receive(rx_buffer, sizeof(rx_buffer),
                                       &received_len)) {
      // Received data - convert back to packet if needed
      if (received_len == sizeof(hrms_comm_packet_t)) {
        memcpy(&packet, rx_buffer, sizeof(packet));
        // Handle received packet based on type or forward to controller
        // For now, just process it in the communication hub
      }
    }

    vTaskDelay(pdMS_TO_TICKS(50)); // 50ms cycle time
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

static void handle_button_event(void) {
  hrms_button_event_t event;
  hrms_actuator_command_t command;

  if (xQueueReceive(xButtonEventQueue, &event, 0) == pdPASS) {
    hrms_controller_process_button(&event, &command);
    xQueueSendToBack(xActuatorCmdQueue, &command, 0);
  }
}
