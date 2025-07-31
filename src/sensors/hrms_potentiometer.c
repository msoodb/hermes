/**
 * Belfhym Potentiometer Module
 * Masoud Bolhassani <masoud.bolhassani@gmail.com>
 * GPLv3
 */

#include "hrms_config.h"

#include "hrms_potentiometer.h"
#include "hrms_adc.h"
#include "hrms_pins.h"
#include <stdbool.h>

void hrms_potentiometer_init(void) {
  // Just init ADC for now (PA6 pin)
  hrms_adc_init();
}

bool hrms_potentiometer_read(hrms_potentiometer_data_t *data) {
  if (!data)
    return false;

  uint16_t raw_value = 0;
  int ret = hrms_adc_read(HRMS_POTENTIOMETER_ADC_CHANNEL, &raw_value);
  if (ret != 0)
    return false;

  data->raw_value = raw_value;
  return true;
}

