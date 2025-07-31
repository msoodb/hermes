


#ifndef HRMS_STATE_H
#define HRMS_STATE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  HRMS_MODE_AUTO = 0,
  HRMS_MODE_MANUAL,
  HRMS_MODE_EMERGENCY
} hrms_mode_t;

typedef enum {
  HRMS_MOTION_FORWARD = 0,
  HRMS_MOTION_BACKWARD,
  HRMS_MOTION_ROTATE_LEFT,
  HRMS_MOTION_ROTATE_RIGHT,
  HRMS_MOTION_STOP
} hrms_motion_state_t;

typedef struct {
  hrms_mode_t current_mode;
  hrms_motion_state_t motion_state;
  bool overheat;
  bool obstacle_near;
  bool manual_override;
  uint32_t last_mode_change_time;
} hrms_system_state_t;

#endif // HRMS_STATE_H
