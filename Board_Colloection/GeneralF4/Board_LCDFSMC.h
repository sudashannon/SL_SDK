#ifndef __BOARD_FSMC_H
#define __BOARD_FSMC_H
#include "APP_Include.h"
#include "BSP_Include.h"

//////////////////////////////////////////////////////////////////////////////////	 
 
//LCD驱动重要参数集
typedef struct 
{
	uint8_t DisplayDirection;
	uint16_t BackGroundColor;
	uint16_t BrushColor;
	uint16_t ID;
	uint16_t Width;
	uint16_t Height;
	uint16_t WriteGramCmd;
	uint16_t SetXCmd;
	uint16_t SetYCmd;
}LCD_Control_t;
extern LCD_Control_t LCD_Handle;
//////////////////////////////////////////////////////////////////////////////////	 

//使用NOR/SRAM的 Bank1.sector4,地址位HADDR[27,26]=11 
//A12作为数据命令区分线 
//注意设置时STM32内部会右移一位对其		    
#define  CMD_BASE        ((uint32_t)(0x6C000000 | 0x00001FFE))
#define  DATA_BASE       ((uint32_t)(0x6C000000 | 0x00002000))

#define LCD_CMD       ( * (uint16_t *) CMD_BASE )
#define LCD_DATA      ( * (uint16_t *) DATA_BASE)
	 
//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左	 

#define INIT_SCAN_DIR  L2R_U2D   //设置初始化扫描方向

//颜色值定义
#define WHITE        0xFFFF
#define BLACK        0x0000	  
#define BLUE         0x001F 
#define GREEN        0x07E0
#define BRED         0XF81F
#define GRED 			   0XFFE0
#define GBLUE			   0X07FF
#define BROWN 			 0XBC40  
#define BRRED 			 0XFC07  
#define GRAY  			 0X8430  
#define RED          0xF800
#define MAGENTA      0xF81F
#define CYAN         0x7FFF
#define YELLOW       0xFFE0
	    			
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_Value);
uint16_t LCD_ReadReg(uint16_t LCD_Reg);
void LCD_WriteRAM_Prepare(void);

void LCD_Init(void);													   	//初始化
void LCD_DisplayOn(void);													//开显示
void LCD_DisplayOff(void);												//关显示
void LCD_Clear(uint16_t Color);	 											//清屏
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);						//设置光标
void LCD_DrawPoint(uint16_t x,uint16_t y);									//画点
void LCD_Color_DrawPoint(uint16_t x,uint16_t y,uint16_t color);	//颜色画点
uint16_t  LCD_GetPoint(uint16_t x,uint16_t y); 								  //读点 

void LCD_AUTOScan_Dir(uint8_t dir);					 
void LCD_Display_Dir(uint8_t dir);						 
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height); 	

void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r);										//画圆
void LCD_Draw_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);						//画线
void LCD_Draw_Rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		   	//画矩形
void LCD_Fill_onecolor(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color);		//填充单个颜色
void LCD_Draw_Picture(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);		//填充指定颜色
void LCD_DisplayChar(uint16_t x,uint16_t y,uint8_t word,uint8_t size);						      //显示一个字符
void LCD_DisplayNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);				//显示 数字
void LCD_DisplayNum_color(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode,uint16_t brushcolor,uint16_t backcolor); //显示自定义数字
void LCD_DisplayString(uint16_t x,uint16_t y,uint8_t size,uint8_t *p);		           //显示一个12/16/24字体字符串
void LCD_DisplayString_color(uint16_t x,uint16_t y,uint8_t size,uint8_t *p,uint16_t brushcolor,uint16_t backcolor); //显示一个12/16/24字体自定义颜色的字符串
	  	   																			 
//9328 LCD寄存器  
#define R0             0x00
#define R1             0x01
#define R2             0x02
#define R3             0x03
#define R4             0x04
#define R5             0x05
#define R6             0x06
#define R7             0x07
#define R8             0x08
#define R9             0x09
#define R10            0x0A
#define R12            0x0C
#define R13            0x0D
#define R14            0x0E
#define R15            0x0F
#define R16            0x10
#define R17            0x11
#define R18            0x12
#define R19            0x13
#define R20            0x14
#define R21            0x15
#define R22            0x16
#define R23            0x17
#define R24            0x18
#define R25            0x19
#define R26            0x1A
#define R27            0x1B
#define R28            0x1C
#define R29            0x1D
#define R30            0x1E
#define R31            0x1F
#define R32            0x20
#define R33            0x21
#define R34            0x22
#define R36            0x24
#define R37            0x25
#define R40            0x28
#define R41            0x29
#define R43            0x2B
#define R45            0x2D
#define R48            0x30
#define R49            0x31
#define R50            0x32
#define R51            0x33
#define R52            0x34
#define R53            0x35
#define R54            0x36
#define R55            0x37
#define R56            0x38
#define R57            0x39
#define R59            0x3B
#define R60            0x3C
#define R61            0x3D
#define R62            0x3E
#define R63            0x3F
#define R64            0x40
#define R65            0x41
#define R66            0x42
#define R67            0x43
#define R68            0x44
#define R69            0x45
#define R70            0x46
#define R71            0x47
#define R72            0x48
#define R73            0x49
#define R74            0x4A
#define R75            0x4B
#define R76            0x4C
#define R77            0x4D
#define R78            0x4E
#define R79            0x4F
#define R80            0x50
#define R81            0x51
#define R82            0x52
#define R83            0x53
#define R96            0x60
#define R97            0x61
#define R106           0x6A
#define R118           0x76
#define R128           0x80
#define R129           0x81
#define R130           0x82
#define R131           0x83
#define R132           0x84
#define R133           0x85
#define R134           0x86
#define R135           0x87
#define R136           0x88
#define R137           0x89
#define R139           0x8B
#define R140           0x8C
#define R141           0x8D
#define R143           0x8F
#define R144           0x90
#define R145           0x91
#define R146           0x92
#define R147           0x93
#define R148           0x94
#define R149           0x95
#define R150           0x96
#define R151           0x97
#define R152           0x98
#define R153           0x99
#define R154           0x9A
#define R157           0x9D
#define R192           0xC0
#define R193           0xC1
#define R229           0xE5							  		 
#endif
