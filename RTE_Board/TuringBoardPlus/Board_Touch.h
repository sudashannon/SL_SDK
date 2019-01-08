#ifndef __BOARD_TOUCH_H
#define __BOARD_TOUCH_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
#define TSC2007_8_bit             0x02
#define TSC2007_12_bit		        0x00
#define Measure_X_Select          0xC0
#define Measure_Y_Select          0xD0
#define Measure_Z1_Select         0xE0
#define Measure_Z2_Select         0xF0
#define Converter_Mode            TSC2007_8_bit
typedef struct
{
  uint16_t TouchDetected;
  uint16_t x;
  uint16_t y;
  uint16_t z;
}TS_StateTypeDef;
typedef enum 
{
  TS_OK       = 0x00,
  TS_ERROR    = 0x01,
  TS_TIMEOUT  = 0x02
}TS_StatusTypeDef;
uint8_t Board_Touch_ITConfig(void);
uint8_t Board_Touch_ITGetStatus(void);
void    Board_Touch_ITClear(void);
uint8_t Board_Touch_GetState(TS_StateTypeDef *TS_State);
#if GUI_USE_LVGL
bool Board_GUI_TouchScan(lv_indev_data_t *data);
#endif
#endif
