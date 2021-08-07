#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include "RTE_Components.h"
#include CMSIS_device_header
#include "rte_include.h"
#include "hal_include.h"

extern void bsp_lcd_init(void);
extern void bsp_lcd_put_pixel(uint16_t x, uint16_t y, uint16_t color);
extern void bsp_lcd_fill_frame_slow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
#endif
