#ifndef __BOARD_LCD_H
#define __BOARD_LCD_H		
#include "stm32f4xx.h"
#include "RTE_Include.h"    
typedef struct
{
	__IO uint16_t LCD_REG;
	__IO uint16_t LCD_RAM;
} LCD_TypeDef; 			    
#define LCD_BASE        ((uint32_t)(0x60000000 | 0x00007FFFE))
#define LCD             ((LCD_TypeDef *) LCD_BASE)
extern void Board_LCD_Init(void);
extern void GUI_LCD_Fill(int32_t sx,int32_t sy,int32_t ex,int32_t ey,lv_color_t color);
extern void GUI_LCD_Map(int32_t sx,int32_t sy,int32_t ex,int32_t ey,const lv_color_t * color_map);
extern void GUI_LCD_Flush(int32_t sx,int32_t sy,int32_t ex,int32_t ey,const lv_color_t * color_map);
#endif
