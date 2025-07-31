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
#include "hrms_config.h"
#if HRMS_ENABLED_JOYSTICK
#include "hrms_joystick.h"
#include "hrms_gpio.h"
#include "hrms_pins.h"
#endif

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "hrms_actuator_hub.h"
#include "hrms_controller.h"
#include "hrms_sensor_hub.h"

#if HRMS_ENABLED_MODE_BUTTON
#include "hrms_mode_button.h"
#endif

#if HRMS_ENABLED_IR_REMOTE
#include "hrms_ir_remote.h"
#endif

#if HRMS_ENABLED_BIGSOUND
#include "hrms_bigsound.h"
#endif

#if HRMS_ENABLED_COMMUNICATION_HUB
#include "hrms_communication_hub.h"
#endif

// --- Task declarations ---
static void vSensorHubTask(void *pvParameters);
static void vControllerTask(void *pvParameters);
static void vActuatorHubTask(void *pvParameters);
#if HRMS_ENABLED_COMMUNICATION_HUB
static void vCommunicationHubTask(void *pvParameters);
#endif

// --- Event Handlers ---
static void handle_sensor_data(void);

#if HRMS_ENABLED_BIGSOUND
static void handle_bigsound_event(void);
#endif

#if HRMS_ENABLED_IR_REMOTE
static void handle_ir_remote_event(void);
#endif

#if HRMS_ENABLED_MODE_BUTTON
static void handle_mode_button_event(void);
#endif

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

#if HRMS_ENABLED_BIGSOUND
static QueueHandle_t xBigSoundQueue = NULL;
#endif

#if HRMS_ENABLED_IR_REMOTE
static QueueHandle_t xIRRemoteQueue = NULL;
#endif

#if HRMS_ENABLED_MODE_BUTTON
static QueueHandle_t xModeButtonQueue = NULL;
#endif

// ESP32 queue removed

static QueueSetHandle_t xControllerQueueSet = NULL;

void hrms_taskmanager_setup(void) {
  // Always create sensor + actuator command queues
  xSensorDataQueue = xQueueCreate(5, sizeof(hrms_sensor_data_t));
  configASSERT(xSensorDataQueue != NULL);

  xActuatorCmdQueue = xQueueCreate(5, sizeof(hrms_actuator_command_t));
  configASSERT(xActuatorCmdQueue != NULL);

  // Optional queues
#if HRMS_ENABLED_BIGSOUND
  xBigSoundQueue = xQueueCreate(5, sizeof(hrms_bigsound_event_t));
  configASSERT(xBigSoundQueue != NULL);
#endif

#if HRMS_ENABLED_IR_REMOTE
  xIRRemoteQueue = xQueueCreate(5, sizeof(hrms_ir_remote_event_t));
  configASSERT(xIRRemoteQueue != NULL);
#endif

#if HRMS_ENABLED_MODE_BUTTON
  xModeButtonQueue = xQueueCreate(5, sizeof(hrms_mode_button_event_t));
  configASSERT(xModeButtonQueue != NULL);
#endif

// ESP32 queue creation removed

  // Queue set
  xControllerQueueSet = xQueueCreateSet(10);
  configASSERT(xControllerQueueSet != NULL);

  xQueueAddToSet(xSensorDataQueue, xControllerQueueSet);

#if HRMS_ENABLED_BIGSOUND
  xQueueAddToSet(xBigSoundQueue, xControllerQueueSet);
#endif
#if HRMS_ENABLED_IR_REMOTE
  xQueueAddToSet(xIRRemoteQueue, xControllerQueueSet);
#endif
#if HRMS_ENABLED_MODE_BUTTON
  xQueueAddToSet(xModeButtonQueue, xControllerQueueSet);
#endif
// ESP32 queue set removed

  // Init all modules
  hrms_sensor_hub_init();
  hrms_actuator_hub_init();
  hrms_controller_init();

#if HRMS_ENABLED_COMMUNICATION_HUB
  hrms_communication_hub_init();
#endif

#if HRMS_ENABLED_MODE_BUTTON
  hrms_mode_button_init(xModeButtonQueue);
#endif

#if HRMS_ENABLED_BIGSOUND
  hrms_bigsound_init(xBigSoundQueue);
#endif

#if HRMS_ENABLED_IR_REMOTE
  hrms_ir_remote_init(xIRRemoteQueue);
#endif

// ESP32 init removed

  // Tasks (always run sensor and actuator hub)
  xTaskCreate(vSensorHubTask, "SensorHub", SENSOR_HUB_TASK_STACK, NULL,
              SENSOR_HUB_TASK_PRIORITY, NULL);

  xTaskCreate(vControllerTask, "Controller", CONTROLLER_TASK_STACK, NULL,
              CONTROLLER_TASK_PRIORITY, NULL);

  xTaskCreate(vActuatorHubTask, "ActuatorHub", ACTUATOR_HUB_TASK_STACK, NULL,
              ACTUATOR_HUB_TASK_PRIORITY, NULL);

#if HRMS_ENABLED_COMMUNICATION_HUB
  xTaskCreate(vCommunicationHubTask, "CommHub", COMMUNICATION_HUB_TASK_STACK, NULL,
              COMMUNICATION_HUB_TASK_PRIORITY, NULL);
#endif
}

void hrms_taskmanager_start(void) { vTaskStartScheduler(); }

// --- Tasks ---
static void vSensorHubTask(void *pvParameters) {
  (void)pvParameters;
  hrms_sensor_data_t sensor_data;
  
#if HRMS_ENABLED_JOYSTICK
  static bool last_joy_button = false;
#endif

  for (;;) {
    if (hrms_sensor_hub_read(&sensor_data)) {
      xQueueSendToBack(xSensorDataQueue, &sensor_data, 0);
    }
    
#if HRMS_ENABLED_JOYSTICK
    // Test joystick button - toggle debug LED on button press
    hrms_joystick_event_t joy_event;
    hrms_joystick_check_events(&joy_event);
    
    if (joy_event.event_occurred && joy_event.button_pressed && !last_joy_button) {
      // Button was just pressed - toggle debug LED
      hrms_gpio_toggle_pin((uint32_t)HRMS_LED_DEBUG_PORT, HRMS_LED_DEBUG_PIN);
    }
    last_joy_button = joy_event.button_pressed;
#endif
    
    vTaskDelay(pdMS_TO_TICKS(200));  // Slower sensor reading for stability
  }
}

static void vControllerTask(void *pvParameters) {
  (void)pvParameters;

#if HRMS_ENABLED_IR_REMOTE
  hrms_actuator_command_t command;
#endif

// ESP32 event variables removed

  for (;;) {
    QueueSetMemberHandle_t activated =
        xQueueSelectFromSet(xControllerQueueSet, pdMS_TO_TICKS(100));

    if (activated == NULL) {
#if HRMS_ENABLED_IR_REMOTE
      if (hrms_controller_check_ir_timeout(&command)) {
        xQueueSendToBack(xActuatorCmdQueue, &command, 0);
      }
#endif
// ESP32 event polling removed
      continue;
    }

    if (activated == xSensorDataQueue) {
      handle_sensor_data();
    }
#if HRMS_ENABLED_BIGSOUND
    else if (activated == xBigSoundQueue) {
      handle_bigsound_event();
    }
#endif
#if HRMS_ENABLED_IR_REMOTE
    else if (activated == xIRRemoteQueue) {
      handle_ir_remote_event();
    }
#endif
#if HRMS_ENABLED_MODE_BUTTON
    else if (activated == xModeButtonQueue) {
      handle_mode_button_event();
    }
#endif
// ESP32 event handling removed
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

#if HRMS_ENABLED_BIGSOUND
static void handle_bigsound_event(void) {
  hrms_bigsound_event_t event;
  hrms_actuator_command_t command;

  if (xQueueReceive(xBigSoundQueue, &event, 0) == pdPASS) {
    hrms_controller_process_bigsound(&event, &command);
    xQueueSendToBack(xActuatorCmdQueue, &command, 0);
  }
}
#endif

#if HRMS_ENABLED_IR_REMOTE
static void handle_ir_remote_event(void) {
  hrms_ir_remote_event_t event;
  hrms_actuator_command_t command;

  if (xQueueReceive(xIRRemoteQueue, &event, 0) == pdPASS) {
    hrms_controller_process_ir_remote(&event, &command);
    xQueueSendToBack(xActuatorCmdQueue, &command, 0);
  }
}
#endif

#if HRMS_ENABLED_MODE_BUTTON
static void handle_mode_button_event(void) {
  hrms_mode_button_event_t event;
  hrms_actuator_command_t command;

  if (xQueueReceive(xModeButtonQueue, &event, 0) == pdPASS) {
    hrms_controller_process_mode_button(&event, &command);
    xQueueSendToBack(xActuatorCmdQueue, &command, 0);
  }
}
#endif

#if HRMS_ENABLED_COMMUNICATION_HUB
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
#endif
