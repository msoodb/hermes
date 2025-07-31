
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_ADC_H
#define HRMS_ADC_H

#include <stdint.h>

/**
 * Initialize ADC hardware.
 */
void hrms_adc_init(void);

/**
 * Read ADC value from the specified channel.
 * @param channel ADC channel number (0-15)
 * @param value pointer to uint16_t to store result
 * @return 0 if success, -1 if invalid channel or null pointer
 */
int hrms_adc_read(uint8_t channel, uint16_t *value);

#endif /* HRMS_ADC_H */
