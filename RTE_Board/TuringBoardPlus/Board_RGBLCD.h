#ifndef __BOARD_RGBLCD_H
#define __BOARD_RGBLCD_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
/** 
  * @brief  AMPIRE640480 Size  
  */    
#define  AT800480_WIDTH    ((uint16_t)800)             /* LCD PIXEL WIDTH            */
#define  AT800480_HEIGHT   ((uint16_t)480)             /* LCD PIXEL HEIGHT           */
/** 
  * @brief  AT800480 Timing  
  */ 
#define  AT800480_HSYNC            ((uint16_t)1)      /* Horizontal synchronization */
#define  AT800480_HBP              ((uint16_t)46)     /* Horizontal back porch      */
#define  AT800480_HFP              ((uint16_t)210)    /* Horizontal front porch     */
#define  AT800480_VSYNC            ((uint16_t)1)      /* Vertical synchronization   */
#define  AT800480_VBP              ((uint16_t)23)     /* Vertical back porch        */
#define  AT800480_VFP              ((uint16_t)22)     /* Vertical front porch       */
typedef struct
{
	uint8_t ActiveLayer;
	DMA2D_HandleTypeDef RGBLCDHdma2dHalHandle;
  LTDC_HandleTypeDef  RGBLCDLtdcHalHandle;
  uint32_t TextColor;
  uint32_t BackColor;
}Board_RGBLCD_Control_t;
extern Board_RGBLCD_Control_t RGBLCDHandle ;
//初始化
extern void Board_LCD_Init(void);
extern void  Board_LCD_DeInit(void);
void     Board_LCD_MspInit(LTDC_HandleTypeDef *hltdc, void *Params);
void     Board_LCD_MspDeInit(LTDC_HandleTypeDef *hltdc, void *Params);
//LTDC相关
uint32_t Board_LCD_GetXSize(void);
uint32_t Board_LCD_GetYSize(void);
void     Board_LCD_SetXSize(uint32_t imageWidthPixels);
void     Board_LCD_SetYSize(uint32_t imageHeightPixels);
/* Functions using the LTDC controller */
void     Board_LCD_LayerDefaultInit(uint16_t LayerIndex, uint32_t FrameBuffer);
void     Board_LCD_SetTransparency(uint32_t LayerIndex, uint8_t Transparency);
void     Board_LCD_SetTransparency_NoReload(uint32_t LayerIndex, uint8_t Transparency);
void     Board_LCD_SetLayerAddress(uint32_t LayerIndex, uint32_t Address);
void     Board_LCD_SetLayerAddress_NoReload(uint32_t LayerIndex, uint32_t Address);
void     Board_LCD_SetColorKeying(uint32_t LayerIndex, uint32_t RGBValue);
void     Board_LCD_SetColorKeying_NoReload(uint32_t LayerIndex, uint32_t RGBValue);
void     Board_LCD_ResetColorKeying(uint32_t LayerIndex);
void     Board_LCD_ResetColorKeying_NoReload(uint32_t LayerIndex);
void     Board_LCD_SetLayerWindow(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     Board_LCD_SetLayerWindow_NoReload(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     Board_LCD_SelectLayer(uint32_t LayerIndex);
void     Board_LCD_SetLayerVisible(uint32_t LayerIndex, FunctionalState State);
void     Board_LCD_SetLayerVisible_NoReload(uint32_t LayerIndex, FunctionalState State);
void     Board_LCD_Relaod(uint32_t ReloadType);
//基本绘图
uint32_t Board_LCD_ReadPixel(uint16_t Xpos, uint16_t Ypos);
void     Board_LCD_DrawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t pixel);
//显示相关
void     Board_LCD_DisplayOff(void);
void     Board_LCD_DisplayOn(void);
//GUI相关
#if GUI_USE_LVGL
void Board_GUI_Fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color);
void Board_GUI_Map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);
void Board_GUI_Flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2 , const lv_color_t * color_map);
void Board_GUI_Blend(lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);
void Board_GUI_FillOPA(lv_color_t * dest, uint32_t length, lv_color_t color);
#endif
#endif
