#include "hrms_config.h"

#if BLFM_ENABLED_POTENTIOMETER

#ifndef BLFM_POTENTIOMETER_H
#define BLFM_POTENTIOMETER_H

#include <stdint.h>
#include "hrms_types.h"

/**
 * Initialize potentiometer module (ADC setup etc.)
 */
void hrms_potentiometer_init(void);

/**
 * Read the potentiometer value (0-4095).
 * Returns -1 on error.
 */
bool hrms_potentiometer_read(hrms_potentiometer_data_t *data);

#endif /* BLFM_POTENTIOMETER_H */

#endif /* BLFM_ENABLED_POTENTIOMETER */
