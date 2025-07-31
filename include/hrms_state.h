


#ifndef BLFM_STATE_H
#define BLFM_STATE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  BLFM_MODE_AUTO = 0,
  BLFM_MODE_MANUAL,
  BLFM_MODE_EMERGENCY
} hrms_mode_t;

typedef enum {
  BLFM_MOTION_FORWARD = 0,
  BLFM_MOTION_BACKWARD,
  BLFM_MOTION_ROTATE_LEFT,
  BLFM_MOTION_ROTATE_RIGHT,
  BLFM_MOTION_STOP
} hrms_motion_state_t;

typedef struct {
  hrms_mode_t current_mode;
  hrms_motion_state_t motion_state;
  bool overheat;
  bool obstacle_near;
  bool manual_override;
  uint32_t last_mode_change_time;
} hrms_system_state_t;

#endif // BLFM_STATE_H
