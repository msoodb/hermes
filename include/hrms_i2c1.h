
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_I2C1_H
#define HRMS_I2C1_H

#include <stdint.h>
#include <stddef.h>

void hrms_i2c1_init(void);
int hrms_i2c1_write(uint8_t addr, const uint8_t *data, size_t len);
int hrms_i2c1_write_byte(uint8_t addr, uint8_t reg, uint8_t data);
int hrms_i2c1_read_bytes(uint8_t addr, uint8_t reg, uint8_t *buf, size_t len);

#endif // HRMS_I2C1_H
