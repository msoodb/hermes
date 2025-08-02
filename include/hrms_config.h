/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Hermes.
 *
 * Hermes is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_CONFIG_H
#define HRMS_CONFIG_H

// =============================================================================
// SYSTEM CONFIGURATION
// =============================================================================

// Queue timeouts (milliseconds)
#define HRMS_QUEUE_TIMEOUT_FAST         5
#define HRMS_QUEUE_TIMEOUT_NORMAL       10  
#define HRMS_QUEUE_TIMEOUT_SLOW         50

// Debounce delays
#define HRMS_BUTTON_DEBOUNCE_MS         50  // Was 100
#define HRMS_EXTI_DEBOUNCE_MS           25

// =============================================================================
// COMMUNICATION CONFIGURATION  
// =============================================================================

// nRF24L01 Settings
#define HRMS_NRF24_CHANNEL              76
#define HRMS_NRF24_RETRY_COUNT          3
#define HRMS_NRF24_RETRY_DELAY          5
#define HRMS_NRF24_POWER_LEVEL          NRF24L01_POWER_0DBM
#define HRMS_NRF24_DATA_RATE            NRF24L01_DATARATE_1MBPS

// Communication timing
#define HRMS_COMM_TX_INTERVAL_MS        500  // Was hardcoded
#define HRMS_COMM_PACKET_TIMEOUT_MS     10   // Was hardcoded

// =============================================================================
// SENSOR CONFIGURATION
// =============================================================================

// Joystick calibration
#define HRMS_JOYSTICK_CENTER_VALUE      2048
#define HRMS_JOYSTICK_DEADZONE          400   // Reduced from 600
#define HRMS_JOYSTICK_UPDATE_THRESHOLD  50    // Reduced from 100

// Sensor reading intervals
#define HRMS_SENSOR_READ_INTERVAL_MS    100   // Faster than 200ms

// =============================================================================
// ACTUATOR CONFIGURATION
// =============================================================================

// LED configuration
#define HRMS_LED_BLINK_SPEED_DEFAULT    200
#define HRMS_LED_BLINK_SPEED_FAST       100
#define HRMS_LED_BLINK_SPEED_SLOW       500

// OLED configuration
#define HRMS_OLED_UPDATE_INTERVAL_MS    50    // Was implicit
#define HRMS_OLED_REFRESH_RATE_HZ       20

// =============================================================================
// FEATURE TOGGLES
// =============================================================================

// Enable/disable features for different builds
#define HRMS_ENABLE_DEBUG_OUTPUT        1
#define HRMS_ENABLE_SELF_TEST           1
#define HRMS_ENABLE_ENCRYPTION          1
#define HRMS_ENABLE_STATISTICS          0     // Disable for memory savings

#if HRMS_ENABLE_DEBUG_OUTPUT
    #define HRMS_DEBUG_PRINT(fmt, ...) // Could add debug printing
#else
    #define HRMS_DEBUG_PRINT(fmt, ...)
#endif

#endif // HRMS_CONFIG_H