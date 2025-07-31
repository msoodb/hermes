
/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Belfhym.
 *
 * Belfhym is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#include "hrms_board.h"
#include "hrms_clock.h"
#include "hrms_gpio.h"
#include "hrms_i2c1.h"
#include "hrms_uart.h"
#include "hrms_adc.h"
#include "hrms_delay.h"
#include "hrms_config.h"
#if BLFM_ENABLED_SERVO
#include "hrms_pwm.h"
#endif

void hrms_board_init(void) {
  hrms_clock_init();    // System clocks
  hrms_gpio_init();     // All GPIO modes

  // Peripheral inits
#if BLFM_ENABLED_SERVO
  hrms_pwm_init();
#endif
  hrms_uart_init();
  hrms_i2c1_init();
  hrms_adc_init();

  // Enable cycle counter
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  hrms_delay_init();
}
