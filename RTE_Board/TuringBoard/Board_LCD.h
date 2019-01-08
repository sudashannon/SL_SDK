#ifndef __BOARD_LCD_H
#define __BOARD_LCD_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
extern void Board_LCD_Init(void);
extern void Board_LCD_DrawBuffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *buffer);
extern void Board_LCD_DrawData(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data,bool ifgray) ;
extern void Board_LCD_FillFrame(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
extern void Board_LCD_PutPixel(uint32_t x, uint32_t y, uint16_t color);
extern bool Board_LCD_TouchScan(uint16_t *x,uint16_t *y);
#if RTE_USE_GUI == 1
extern void Board_GUI_Fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color);
extern void Board_GUI_Flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_map);
extern bool Board_GUI_TouchScan(lv_indev_data_t *data);
#endif
#endif
