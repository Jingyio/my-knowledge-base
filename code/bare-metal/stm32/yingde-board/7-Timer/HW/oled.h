#ifndef __OLED_H__
#define __OLED_H__

#include "stm32f10x.h"
#include "stdlib.h"	

#define I2C_PORT_OLED   (0)   // SW I2C, 0~2

#define OLED_CMD        (0)
#define OLED_DATA       (1)
#define OLED_PAGE_SIZE  (128)
#define OLED_PAGE_NUM   (8)

void oled_reverse(uint8_t is_reversed);
void oled_rotate(uint8_t is_rotated);

void oled_display_on(void);
void oled_display_off(void);
void oled_clear(void);
void oled_init(void);

void oled_buf_draw(uint8_t x, uint8_t y, uint8_t bit);
void oled_buf_render(void);
#endif

