

#ifndef HRMS_OLED_H
#define HRMS_OLED_H

#include "hrms_types.h"
#include <stdint.h>

void hrms_oled_init(void);
void hrms_oled_clear(void);
void hrms_oled_flush(void);
void hrms_oled_draw_pixel(uint8_t x, uint8_t y, uint8_t color);
void hrms_oled_draw_line(int x0, int y0, int x1, int y1);
void hrms_oled_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void hrms_oled_draw_char(uint8_t x, uint8_t page, char c);
void hrms_oled_draw_text(uint8_t x, uint8_t page, const char *str);
void hrms_oled_invert(void);
void hrms_oled_draw_progress_bar(uint8_t percent);
void hrms_oled_scroll_horizontal(const char *text, uint8_t speed);
void hrms_oled_scroll_text(const char *text, uint8_t speed_ms);
void hrms_oled_blink(uint8_t times, uint16_t delay_ms);
void hrms_oled_apply(const hrms_oled_command_t *cmd);

#endif
