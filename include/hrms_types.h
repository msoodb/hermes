/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_TYPES_H
#define HRMS_TYPES_H

#include <stdbool.h>
#include <stdint.h>

//==============================================================================
// DISPLAY CONSTANTS
//==============================================================================

#define HRMS_OLED_WIDTH 128
#define HRMS_OLED_HEIGHT 32
#define HRMS_OLED_PAGES (HRMS_OLED_HEIGHT / 8)
#define HRMS_OLED_MAX_SMALL_TEXT_LEN 12
#define HRMS_OLED_MAX_BIG_TEXT_LEN 16

//==============================================================================
// COMMUNICATION
//==============================================================================

// Communication packet types
typedef enum {
  HRMS_COMM_PACKET_NONE = 0,
  HRMS_COMM_PACKET_HEARTBEAT,
  HRMS_COMM_PACKET_SENSOR_DATA,
  HRMS_COMM_PACKET_CONTROL_CMD,
  HRMS_COMM_PACKET_STATUS,
  HRMS_COMM_PACKET_CONFIG,
  HRMS_COMM_PACKET_ACK,
  HRMS_COMM_PACKET_ERROR
} hrms_comm_packet_type_t;

// Communication directions
typedef enum {
  HRMS_COMM_DIR_TX = 0,
  HRMS_COMM_DIR_RX = 1
} hrms_comm_direction_t;

// Generic communication packet structure
#define HRMS_COMM_MAX_PAYLOAD_SIZE 32
typedef struct {
  uint8_t packet_id;                           // Unique packet identifier
  hrms_comm_packet_type_t packet_type;         // Type of packet
  uint8_t source_id;                           // Source device ID
  uint8_t dest_id;                             // Destination device ID
  uint8_t payload_size;                        // Size of payload (0-32)
  uint8_t payload[HRMS_COMM_MAX_PAYLOAD_SIZE]; // Actual data
  uint16_t checksum;                           // Simple checksum for error detection
  uint32_t timestamp;                          // When packet was created/received
} hrms_comm_packet_t;

// Communication events for queue system
typedef struct {
  hrms_comm_direction_t direction;             // TX or RX
  hrms_comm_packet_t packet;                   // The actual packet
  bool success;                                // For TX: was sent successfully, For RX: was received without errors
  uint8_t retry_count;                         // Number of retries (for TX)
} hrms_comm_event_t;

// Communication statistics
typedef struct {
  uint32_t packets_sent;
  uint32_t packets_received;
  uint32_t packets_failed;
  uint32_t last_rx_timestamp;
  uint32_t last_tx_timestamp;
} hrms_comm_stats_t;

//==============================================================================
// SENSORS
//==============================================================================

// Event definitions
typedef enum {
  HRMS_MODE_BUTTON_EVENT_PRESSED,
  HRMS_MODE_BUTTON_EVENT_RELEASED
} hrms_mode_button_event_type_t;

typedef struct {
  hrms_mode_button_event_type_t event_type;
  uint32_t timestamp;
} hrms_mode_button_event_t;

typedef struct {
  int16_t acc_x;
  int16_t acc_y;
  int16_t acc_z;
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;
} hrms_imu_data_t;

typedef struct {
  uint16_t raw_value;  // raw ADC reading from potentiometer (0-4095)
} hrms_potentiometer_data_t;

typedef struct {
  int32_t temperature_mc;
} hrms_temperature_data_t;

typedef struct {
  int16_t x_axis;      // -1000 to +1000 (left to right)
  int16_t y_axis;      // -1000 to +1000 (down to up)
  bool button_pressed; // true if SW button is pressed
} hrms_joystick_data_t;



typedef struct {
  hrms_imu_data_t imu;
  hrms_temperature_data_t temperature;
  hrms_potentiometer_data_t potentiometer;
  hrms_joystick_data_t joystick;
} hrms_sensor_data_t;

//==============================================================================
// BIGSOUND
//==============================================================================

typedef enum { BIGSOUND_EVENT_DETECTED = 1 } hrms_bigsound_event_type_t;

typedef struct {
  uint32_t timestamp;                    // Tick count when event happened
  hrms_bigsound_event_type_t event_type; // Type of bigsound event
} hrms_bigsound_event_t;

//==============================================================================
// ACTUATORS
//==============================================================================



typedef enum {
  HRMS_OLED_ICON_NONE = 0,
  HRMS_OLED_ICON_HEART,
  HRMS_OLED_ICON_SMILEY,
  HRMS_OLED_ICON_STAR,
} hrms_oled_icon_t;

typedef struct {
  hrms_oled_icon_t icon1;  // top-left icon
  hrms_oled_icon_t icon2;  // top-left icon (next to icon1)
  hrms_oled_icon_t icon3;  // top-right icon
  hrms_oled_icon_t icon4;  // top-right icon (next to icon3)

  char smalltext1[HRMS_OLED_MAX_SMALL_TEXT_LEN];
  char bigtext[HRMS_OLED_MAX_BIG_TEXT_LEN];
  char smalltext2[HRMS_OLED_MAX_SMALL_TEXT_LEN];

  uint8_t invert;          // invert display
  uint8_t progress_percent; // optional progress bar

} hrms_oled_command_t;

typedef struct {
  bool active;
  uint8_t pattern_id;
  uint16_t duration_ms;
  uint8_t volume;
} hrms_alarm_command_t;

typedef enum {
  HRMS_LED_MODE_OFF = 0,
  HRMS_LED_MODE_ON,
  HRMS_LED_MODE_BLINK,
} hrms_led_mode_t;

typedef struct {
  hrms_led_mode_t mode;    // off, on, or blink
  uint16_t blink_speed_ms; // blinking speed in milliseconds (period)
  uint8_t pattern_id; // blink pattern id (e.g., 0=solid, 1=short pulses, 2=SOS,
                      // etc)
  uint8_t
      brightness; // brightness level (0-255), optional if hardware supports PWM
} hrms_led_command_t;

// Radio command removed - ready for new communication implementation

typedef struct {
  hrms_oled_command_t oled;
  hrms_led_command_t led;
  hrms_alarm_command_t alarm;
} hrms_actuator_command_t;

#endif // HRMS_TYPES_H
