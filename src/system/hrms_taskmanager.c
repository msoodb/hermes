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
#define SENSOR_HUB_TASK_STACK 384
#define CONTROLLER_TASK_STACK 512
#define ACTUATOR_HUB_TASK_STACK 384
#define COMMUNICATION_HUB_TASK_STACK 512

#define SENSOR_HUB_TASK_PRIORITY 4     // Highest - real-time data collection
#define CONTROLLER_TASK_PRIORITY 3     // High - process sensor data quickly
#define ACTUATOR_HUB_TASK_PRIORITY 2   // Medium - execute control commands
#define COMMUNICATION_HUB_TASK_PRIORITY 1 // Lowest - non-critical background

// --- Queues ---
static QueueHandle_t xSensorDataQueue = NULL;
static QueueHandle_t xActuatorCmdQueue = NULL;
static QueueHandle_t xButtonEventQueue = NULL;
static QueueHandle_t xCommCmdQueue = NULL;
static QueueSetHandle_t xControllerQueueSet = NULL;

void hrms_taskmanager_setup(void) {

  xSensorDataQueue = xQueueCreate(15, sizeof(hrms_sensor_data_t));
  configASSERT(xSensorDataQueue != NULL);

  xActuatorCmdQueue = xQueueCreate(10, sizeof(hrms_actuator_command_t));
  configASSERT(xActuatorCmdQueue != NULL);

  xButtonEventQueue = xQueueCreate(8, sizeof(hrms_button_event_t));
  configASSERT(xButtonEventQueue != NULL);

  xCommCmdQueue = xQueueCreate(10, sizeof(hrms_comm_command_t));
  configASSERT(xCommCmdQueue != NULL);

  // Queue set - sum of member queue lengths
  xControllerQueueSet = xQueueCreateSet(15 + 8); // sensor + button
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
      // Add error handling with timeout
      if (xQueueSendToBack(xSensorDataQueue, &sensor_data, pdMS_TO_TICKS(10)) != pdPASS) {
        // Queue full - sensor data lost (could add error counter here)
      }
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
        if (xQueueSendToBack(xCommCmdQueue, &command.comm, pdMS_TO_TICKS(10)) != pdPASS) {
          // Communication queue full - command lost
        }
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

    // Handle outgoing communication commands (TX) - block for efficiency
    if (xQueueReceive(xCommCmdQueue, &comm_cmd, pdMS_TO_TICKS(10)) == pdPASS) {
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

    vTaskDelay(pdMS_TO_TICKS(20)); // Reduced cycle time for better responsiveness
  }
}

// --- Event Handlers ---
static void handle_sensor_data(void) {
  hrms_sensor_data_t sensor_data;
  hrms_actuator_command_t command;

  if (xQueueReceive(xSensorDataQueue, &sensor_data, 0) == pdPASS) {
    hrms_controller_process(&sensor_data, &command);
    if (xQueueSendToBack(xActuatorCmdQueue, &command, pdMS_TO_TICKS(10)) != pdPASS) {
      // Actuator queue full - command lost
    }
  }
}

static void handle_button_event(void) {
  hrms_button_event_t event;
  hrms_actuator_command_t command;

  if (xQueueReceive(xButtonEventQueue, &event, 0) == pdPASS) {
    hrms_controller_process_button(&event, &command);
    if (xQueueSendToBack(xActuatorCmdQueue, &command, pdMS_TO_TICKS(10)) != pdPASS) {
      // Actuator queue full - command lost
    }
  }
}
