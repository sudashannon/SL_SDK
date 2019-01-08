#ifndef __BOARD_LCD_H
#define __BOARD_LCD_H
#include "stm32f10x.h"
#include "RTE_Include.h"
//-----------------LCD端口定义---------------- 
#define LCD_CTRL   	  	GPIOB		//定义TFT数据端口
#define LCD_CTRL2   	  GPIOC		//定义TFT数据端口
#define LCD_RS         	GPIO_Pin_6	//PB1连接至TFT --RS
#define LCD_CS        	GPIO_Pin_12 //PB11 连接至TFT --CS
//液晶控制口置1操作语句宏定义
#define	LCD_CS_SET  	LCD_CTRL->BSRR=LCD_CS    
#define	LCD_RS_SET  	LCD_CTRL2->BSRR=LCD_RS  

//液晶控制口置0操作语句宏定义
#define	LCD_CS_CLR  	LCD_CTRL->BRR=LCD_CS    
#define	LCD_RS_CLR  	LCD_CTRL2->BRR=LCD_RS 
//画笔颜色
#define WHITE       0xFFFF
#define BLACK      	0x0000	  
#define BLUE       	0x001F  
#define BRED        0XF81F
#define GRED 			 	0XFFE0
#define GBLUE			 	0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define BROWN 			0XBC40 //棕色
#define BRRED 			0XFC07 //棕红色
#define GRAY  			0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	0X841F //浅绿色
//#define LIGHTGRAY     0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 		0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE      	0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE          0X2B12 //浅棕蓝色(选择条目的反色)
extern void Board_LCD_Init(void);
extern void Board_LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color);
extern void Board_LCD_FillRect(uint16_t x,uint16_t y,uint16_t x1,uint16_t y1,uint16_t color);
#endif
