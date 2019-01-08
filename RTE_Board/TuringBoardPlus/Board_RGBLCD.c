#include "Board_RGBLCD.h"
#include "Board_SDRAM.h"
#if RTE_USE_OS
osEventFlagsId_t EvtIDGUI;
#endif
Board_RGBLCD_Control_t RGBLCDHandle = {0};
void Board_LCD_Init(void)
{
	Board_LCD_DeInit();
	/* Polarity */
  RGBLCDHandle.RGBLCDLtdcHalHandle.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  RGBLCDHandle.RGBLCDLtdcHalHandle.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  RGBLCDHandle.RGBLCDLtdcHalHandle.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  RGBLCDHandle.RGBLCDLtdcHalHandle.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  /* Select the used LCD */
	/* The 800x480 LCD is selected */
	/* Timing Configuration */    
	RGBLCDHandle.RGBLCDLtdcHalHandle.Init.HorizontalSync = (AT800480_HSYNC - 1);
	RGBLCDHandle.RGBLCDLtdcHalHandle.Init.VerticalSync = (AT800480_VSYNC - 1);
	RGBLCDHandle.RGBLCDLtdcHalHandle.Init.AccumulatedHBP = (AT800480_HSYNC + AT800480_HBP - 1);
	RGBLCDHandle.RGBLCDLtdcHalHandle.Init.AccumulatedVBP = (AT800480_VSYNC + AT800480_VBP - 1);  
	RGBLCDHandle.RGBLCDLtdcHalHandle.Init.AccumulatedActiveH = (AT800480_HEIGHT + AT800480_VSYNC + AT800480_VBP - 1);
	RGBLCDHandle.RGBLCDLtdcHalHandle.Init.AccumulatedActiveW = (AT800480_WIDTH + AT800480_HSYNC + AT800480_HBP - 1);
	RGBLCDHandle.RGBLCDLtdcHalHandle.Init.TotalHeigh = (AT800480_HEIGHT + AT800480_VSYNC + AT800480_VBP + AT800480_VFP - 1);
	RGBLCDHandle.RGBLCDLtdcHalHandle.Init.TotalWidth = (AT800480_WIDTH + AT800480_HSYNC + AT800480_HBP + AT800480_HFP - 1);
	RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg->ImageHeight = AT800480_HEIGHT;	
	RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg->ImageWidth  = AT800480_WIDTH;
  /* Background value */
  RGBLCDHandle.RGBLCDLtdcHalHandle.Init.Backcolor.Blue = 0;
  RGBLCDHandle.RGBLCDLtdcHalHandle.Init.Backcolor.Green = 0;
  RGBLCDHandle.RGBLCDLtdcHalHandle.Init.Backcolor.Red = 0;
  /* Polarity */
  RGBLCDHandle.RGBLCDLtdcHalHandle.Instance = LTDC;
  /* LCD clock configuration */
  static RCC_PeriphCLKInitTypeDef  periph_clk_init_struct;
	/* The 800x480 LCD is selected */
	/* LCD clock configuration */
	/* PLL3_VCO Input = HSE_VALUE/PLL3M = 25Mhz/5 = 5 Mhz */
	/* PLL3_VCO Output = PLL3_VCO Input * PLL3N  = 5*160 = 800 Mhz */
	/* PLLLCDCLK = PLL3_VCO Output/PLL3R = 800/16 = 50 Mhz */
	/* LTDC clock frequency = PLLLCDCLK = 24 Mhz */
	periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	periph_clk_init_struct.PLL3.PLL3M = 5;
	periph_clk_init_struct.PLL3.PLL3N = 160;
	periph_clk_init_struct.PLL3.PLL3P = 2;
	periph_clk_init_struct.PLL3.PLL3Q = 2;
	periph_clk_init_struct.PLL3.PLL3R = 16;
	HAL_RCCEx_PeriphCLKConfig(&periph_clk_init_struct);
  if(HAL_LTDC_GetState(&RGBLCDHandle.RGBLCDLtdcHalHandle) == HAL_LTDC_STATE_RESET)
  {
    /* Initialize the LCD Msp: this __weak function can be rewritten by the application */
    Board_LCD_MspInit(&RGBLCDHandle.RGBLCDLtdcHalHandle, NULL);
  }
  HAL_LTDC_Init(&RGBLCDHandle.RGBLCDLtdcHalHandle);
#if RTE_USE_OS
  EvtIDGUI = osEventFlagsNew(NULL);
#endif
}
/**
  * @brief  DeInitializes the LCD.
  * @retval LCD state
  */
void Board_LCD_DeInit(void)
{
  /* Initialize the RGBLCDHandle.RGBLCDLtdcHalHandle Instance parameter */
  RGBLCDHandle.RGBLCDLtdcHalHandle.Instance = LTDC;
 /* Disable LTDC block */
  __HAL_LTDC_DISABLE(&RGBLCDHandle.RGBLCDLtdcHalHandle);
  /* DeInit the LTDC */
  HAL_LTDC_DeInit(&RGBLCDHandle.RGBLCDLtdcHalHandle);
  /* DeInit the LTDC MSP : this __weak function can be rewritten by the application */
  Board_LCD_MspDeInit(&RGBLCDHandle.RGBLCDLtdcHalHandle, NULL);
}
uint32_t Board_LCD_GetXSize(void)
{
  return RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].ImageWidth;
}
uint32_t Board_LCD_GetYSize(void)
{
  return RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].ImageHeight;
}
void Board_LCD_SetXSize(uint32_t imageWidthPixels)
{
  RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].ImageWidth = imageWidthPixels;
}
void Board_LCD_SetYSize(uint32_t imageHeightPixels)
{
  RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].ImageHeight = imageHeightPixels;
}
void Board_LCD_LayerDefaultInit(uint16_t LayerIndex, uint32_t FB_Address)
{
  LTDC_LayerCfgTypeDef  layer_cfg;
  /* Layer Init */
  layer_cfg.WindowX0 = 0;
  layer_cfg.WindowX1 = Board_LCD_GetXSize();
  layer_cfg.WindowY0 = 0;
  layer_cfg.WindowY1 = Board_LCD_GetYSize();
  layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  layer_cfg.FBStartAdress = FB_Address;
  layer_cfg.Alpha = 255;
  layer_cfg.Alpha0 = 0;
  layer_cfg.Backcolor.Blue = 0;
  layer_cfg.Backcolor.Green = 0;
  layer_cfg.Backcolor.Red = 0;
  layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  layer_cfg.ImageWidth = Board_LCD_GetXSize();
  layer_cfg.ImageHeight = Board_LCD_GetYSize();
  HAL_LTDC_ConfigLayer(&RGBLCDHandle.RGBLCDLtdcHalHandle, &layer_cfg, LayerIndex);
}
void Board_LCD_SelectLayer(uint32_t LayerIndex)
{
  RGBLCDHandle.ActiveLayer = LayerIndex;
}
void Board_LCD_SetLayerVisible(uint32_t LayerIndex, FunctionalState State)
{
  if(State == ENABLE)
  {
    __HAL_LTDC_LAYER_ENABLE(&RGBLCDHandle.RGBLCDLtdcHalHandle, LayerIndex);
  }
  else
  {
    __HAL_LTDC_LAYER_DISABLE(&RGBLCDHandle.RGBLCDLtdcHalHandle, LayerIndex);
  }
  __HAL_LTDC_RELOAD_IMMEDIATE_CONFIG(&(RGBLCDHandle.RGBLCDLtdcHalHandle));
}
void Board_LCD_SetLayerVisible_NoReload(uint32_t LayerIndex, FunctionalState State)
{
  if(State == ENABLE)
  {
    __HAL_LTDC_LAYER_ENABLE(&RGBLCDHandle.RGBLCDLtdcHalHandle, LayerIndex);
  }
  else
  {
    __HAL_LTDC_LAYER_DISABLE(&RGBLCDHandle.RGBLCDLtdcHalHandle, LayerIndex);
  }
  /* Do not Sets the Reload  */
}
void Board_LCD_SetTransparency(uint32_t LayerIndex, uint8_t Transparency)
{
  HAL_LTDC_SetAlpha(&RGBLCDHandle.RGBLCDLtdcHalHandle, Transparency, LayerIndex);
}

/**
  * @brief  Configures the transparency without reloading.
  * @param  LayerIndex: Layer foreground or background.
  * @param  Transparency: Transparency
  *           This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF
  * @retval None
  */
void Board_LCD_SetTransparency_NoReload(uint32_t LayerIndex, uint8_t Transparency)
{
  HAL_LTDC_SetAlpha_NoReload(&RGBLCDHandle.RGBLCDLtdcHalHandle, Transparency, LayerIndex);
}

/**
  * @brief  Sets an LCD layer frame buffer address.
  * @param  LayerIndex: Layer foreground or background
  * @param  Address: New LCD frame buffer value
  * @retval None
  */
void Board_LCD_SetLayerAddress(uint32_t LayerIndex, uint32_t Address)
{
  HAL_LTDC_SetAddress(&RGBLCDHandle.RGBLCDLtdcHalHandle, Address, LayerIndex);
}

/**
  * @brief  Sets an LCD layer frame buffer address without reloading.
  * @param  LayerIndex: Layer foreground or background
  * @param  Address: New LCD frame buffer value
  * @retval None
  */
void Board_LCD_SetLayerAddress_NoReload(uint32_t LayerIndex, uint32_t Address)
{
  HAL_LTDC_SetAddress_NoReload(&RGBLCDHandle.RGBLCDLtdcHalHandle, Address, LayerIndex);
}

/**
  * @brief  Sets display window.
  * @param  LayerIndex: Layer index
  * @param  Xpos: LCD X position
  * @param  Ypos: LCD Y position
  * @param  Width: LCD window width
  * @param  Height: LCD window height
  * @retval None
  */
void Board_LCD_SetLayerWindow(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  /* Reconfigure the layer size */
  HAL_LTDC_SetWindowSize(&RGBLCDHandle.RGBLCDLtdcHalHandle, Width, Height, LayerIndex);

  /* Reconfigure the layer position */
  HAL_LTDC_SetWindowPosition(&RGBLCDHandle.RGBLCDLtdcHalHandle, Xpos, Ypos, LayerIndex);
}

/**
  * @brief  Sets display window without reloading.
  * @param  LayerIndex: Layer index
  * @param  Xpos: LCD X position
  * @param  Ypos: LCD Y position
  * @param  Width: LCD window width
  * @param  Height: LCD window height
  * @retval None
  */
void Board_LCD_SetLayerWindow_NoReload(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  /* Reconfigure the layer size */
  HAL_LTDC_SetWindowSize_NoReload(&RGBLCDHandle.RGBLCDLtdcHalHandle, Width, Height, LayerIndex);

  /* Reconfigure the layer position */
  HAL_LTDC_SetWindowPosition_NoReload(&RGBLCDHandle.RGBLCDLtdcHalHandle, Xpos, Ypos, LayerIndex);
}

/**
  * @brief  Configures and sets the color keying.
  * @param  LayerIndex: Layer foreground or background
  * @param  RGBValue: Color reference
  * @retval None
  */
void Board_LCD_SetColorKeying(uint32_t LayerIndex, uint32_t RGBValue)
{
  /* Configure and Enable the color Keying for LCD Layer */
  HAL_LTDC_ConfigColorKeying(&RGBLCDHandle.RGBLCDLtdcHalHandle, RGBValue, LayerIndex);
  HAL_LTDC_EnableColorKeying(&RGBLCDHandle.RGBLCDLtdcHalHandle, LayerIndex);
}

/**
  * @brief  Configures and sets the color keying without reloading.
  * @param  LayerIndex: Layer foreground or background
  * @param  RGBValue: Color reference
  * @retval None
  */
void Board_LCD_SetColorKeying_NoReload(uint32_t LayerIndex, uint32_t RGBValue)
{
  /* Configure and Enable the color Keying for LCD Layer */
  HAL_LTDC_ConfigColorKeying_NoReload(&RGBLCDHandle.RGBLCDLtdcHalHandle, RGBValue, LayerIndex);
  HAL_LTDC_EnableColorKeying_NoReload(&RGBLCDHandle.RGBLCDLtdcHalHandle, LayerIndex);
}

/**
  * @brief  Disables the color keying.
  * @param  LayerIndex: Layer foreground or background
  * @retval None
  */
void Board_LCD_ResetColorKeying(uint32_t LayerIndex)
{
  /* Disable the color Keying for LCD Layer */
  HAL_LTDC_DisableColorKeying(&RGBLCDHandle.RGBLCDLtdcHalHandle, LayerIndex);
}

/**
  * @brief  Disables the color keying without reloading.
  * @param  LayerIndex: Layer foreground or background
  * @retval None
  */
void Board_LCD_ResetColorKeying_NoReload(uint32_t LayerIndex)
{
  /* Disable the color Keying for LCD Layer */
  HAL_LTDC_DisableColorKeying_NoReload(&RGBLCDHandle.RGBLCDLtdcHalHandle, LayerIndex);
}

/**
  * @brief  Disables the color keying without reloading.
  * @param  ReloadType: can be one of the following values
  *         - LCD_RELOAD_IMMEDIATE
  *         - LCD_RELOAD_VERTICAL_BLANKING
  * @retval None
  */
void Board_LCD_Relaod(uint32_t ReloadType)
{
  HAL_LTDC_Reload (&RGBLCDHandle.RGBLCDLtdcHalHandle, ReloadType);
}
/**
  * @brief  Reads an LCD pixel.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @retval RGB pixel color
  */
uint32_t Board_LCD_ReadPixel(uint16_t Xpos, uint16_t Ypos)
{
  uint32_t ret = 0;

  if(RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_ARGB8888)
  {
    /* Read data value from SDRAM memory */
    ret = *(__IO uint32_t*) (RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].FBStartAdress + (4*(Ypos*Board_LCD_GetXSize() + Xpos)));
  }
  else if(RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB888)
  {
    /* Read data value from SDRAM memory */
    ret = (*(__IO uint32_t*) (RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].FBStartAdress + (4*(Ypos*Board_LCD_GetXSize() + Xpos))) & 0x00FFFFFF);
  }
  else if((RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565) || \
          (RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_ARGB4444) || \
          (RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_AL88))
  {
    /* Read data value from SDRAM memory */
    ret = *(__IO uint16_t*) (RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].FBStartAdress + (2*(Ypos*Board_LCD_GetXSize() + Xpos)));
  }
  else
  {
    /* Read data value from SDRAM memory */
    ret = *(__IO uint8_t*) (RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].FBStartAdress + (2*(Ypos*Board_LCD_GetXSize() + Xpos)));
  }

  return ret;
}
/**
  * @brief  Draws a pixel on LCD.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  RGB_Code: Pixel color in ARGB mode (8-8-8-8)
  * @retval None
  */
void Board_LCD_DrawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code)
{
  /* Write data value to all SDRAM memory */
  *(__IO uint32_t*) (RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].FBStartAdress + (4*(Ypos*Board_LCD_GetXSize() + Xpos))) = RGB_Code;
}
/**
  * @brief  Enables the display.
  * @retval None
  */
void Board_LCD_DisplayOn(void)
{
  /* Display On */
  __HAL_LTDC_ENABLE(&RGBLCDHandle.RGBLCDLtdcHalHandle);
}

/**
  * @brief  Disables the display.
  * @retval None
  */
void Board_LCD_DisplayOff(void)
{
  /* Display Off */
  __HAL_LTDC_DISABLE(&RGBLCDHandle.RGBLCDLtdcHalHandle);
}

/**
  * @brief  LCD backligh 
  * @retval None
  */
void Board_LCD_BackLighInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = LCD_BL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LCD_BL_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief  Initializes the LTDC MSP.
  * @param  hltdc: LTDC handle
  * @param  Params: Pointer to void
  * @retval None
  */
__weak void Board_LCD_MspInit(LTDC_HandleTypeDef *hltdc, void *Params)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable the LTDC and DMA2D clocks */
  __HAL_RCC_LTDC_CLK_ENABLE();
  __HAL_RCC_DMA2D_CLK_ENABLE();
  /*##-2- NVIC configuration  ################################################*/
  /* NVIC configuration for DMA2D transfer complete interrupt */
  HAL_NVIC_SetPriority(DMA2D_IRQn, DMA2D_PreemptPriority, 0);
  HAL_NVIC_EnableIRQ(DMA2D_IRQn);
  /* Enable GPIOs clock */
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();

  /*** LTDC Pins configuration ***/
  /* GPIOI configuration */
  gpio_init_structure.Pin       = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_HIGH;
  gpio_init_structure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOI, &gpio_init_structure);

  /* GPIOJ configuration */
  gpio_init_structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
                                  GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | \
                                  GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | \
                                  GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_HIGH;
  gpio_init_structure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOJ, &gpio_init_structure);

  /* GPIOK configuration */
  gpio_init_structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
                                  GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_HIGH;
  gpio_init_structure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOK, &gpio_init_structure);
	
	Board_LCD_BackLighInit();
	HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);//turn on backlight
}

/**
  * @brief  DeInitializes Board_LCD MSP.
  * @param  hltdc: LTDC handle
  * @param  Params: Pointer to void
  * @retval None
  */
__weak void Board_LCD_MspDeInit(LTDC_HandleTypeDef *hltdc, void *Params)
{
  GPIO_InitTypeDef  gpio_init_structure;

  /* Disable LTDC block */
  __HAL_LTDC_DISABLE(hltdc);
  /*##-1- Reset peripherals ##################################################*/
  /* Enable DMA2D reset state */
  __HAL_RCC_DMA2D_FORCE_RESET();
  /* Release DMA2D from reset state */
  __HAL_RCC_DMA2D_RELEASE_RESET();
  /* ##-2- Disable DMA2D Clock ##################################################*/
  __HAL_RCC_DMA2D_CLK_DISABLE();
  /* ##-3- Disable DMA2D IRQ in NVIC #############################################*/
  HAL_NVIC_DisableIRQ(DMA2D_IRQn);
  /* LTDC Pins deactivation */
  /* GPIOI deactivation */
  gpio_init_structure.Pin       = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_DeInit(GPIOI, gpio_init_structure.Pin);
  /* GPIOJ deactivation */
  gpio_init_structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
                                  GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | \
                                  GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | \
                                  GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_DeInit(GPIOJ, gpio_init_structure.Pin);
  /* GPIOK deactivation */
  gpio_init_structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
                                  GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  HAL_GPIO_DeInit(GPIOK, gpio_init_structure.Pin);

  /* Disable LTDC clock */
  __HAL_RCC_LTDC_CLK_DISABLE();

  /* GPIO pins clock can be shut down in the application
     by surcharging this __weak function */
}
void DMA2D_TransferComplete(DMA2D_HandleTypeDef *hdma2d)
{
#if RTE_USE_OS
  osEventFlagsSet(EvtIDGUI,0x00000001ul);
#endif
}
#if GUI_USE_LVGL
void Board_GUI_Fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
#if USE_LV_GPU
  uint32_t address,height,width = 0;
	height = y2 - y1 + 1;
	width = x2 - x1 + 1;
  /* Register to memory mode with ARGB8888 as color Mode */
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Init.Mode         = DMA2D_R2M;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Init.OutputOffset = (Board_LCD_GetXSize() - width);

	RGBLCDHandle.RGBLCDHdma2dHalHandle.XferCpltCallback  = DMA2D_TransferComplete;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Instance = DMA2D;

  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&RGBLCDHandle.RGBLCDHdma2dHalHandle) == HAL_OK)
  {
    if(HAL_DMA2D_ConfigLayer(&RGBLCDHandle.RGBLCDHdma2dHalHandle, 1) == HAL_OK)
    {
      if (HAL_DMA2D_Start_IT(&RGBLCDHandle.RGBLCDHdma2dHalHandle, color.full, (uint32_t)address, width, height) == HAL_OK)
      {
        /* Polling For DMA transfer */
        osEventFlagsWait (EvtIDGUI,0x00000001ul,osFlagsWaitAny, osWaitForever);
				SCB_CleanInvalidateDCache_by_Addr((uint32_t *)address, (width*height*4));
      }
    }
  }
#else
	/*Return if the area is out the screen*/
	if(x2 < 0) return;
	if(y2 < 0) return;
	if(x1 > LV_HOR_RES - 1) return;
	if(y1 > LV_VER_RES - 1) return;

	/*Truncate the area to the screen*/
	int32_t act_x1 = x1 < 0 ? 0 : x1;
	int32_t act_y1 = y1 < 0 ? 0 : y1;
	int32_t act_x2 = x2 > LV_HOR_RES - 1 ? LV_HOR_RES - 1 : x2;
	int32_t act_y2 = y2 > LV_VER_RES - 1 ? LV_VER_RES - 1 : y2;

	uint32_t x;
	uint32_t y;

	/*Fill the remaining area*/
	for(x = act_x1; x <= act_x2; x++) {
		for(y = act_y1; y <= act_y2; y++) {
			*((uint32_t *)RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].FBStartAdress + y * LV_HOR_RES + x) = color.full;
		}
	}
#endif
}

#if USE_LV_GPU == 0
static void CopyBuffer(const uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize)
{
	uint32_t row;
	for(row = y; row < y + ysize; row++ ) {
		memcpy(&pDst[row * 800 + x], pSrc, xsize * 4);
		pSrc += xsize;
	}
}
#endif
void Board_GUI_Flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2 , const lv_color_t * color_map)
{
#if USE_LV_GPU
  uint32_t address,height,width = 0;
	height = y2 - y1 + 1;
	width = x2 - x1 + 1;
  /* Set the address */
  address = RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].FBStartAdress + (((Board_LCD_GetXSize()*y1) + x1)*(4));
	
  /* Configure the DMA2D Mode, Color Mode and output offset */
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Init.Mode         = DMA2D_M2M_PFC;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Init.OutputOffset = Board_LCD_GetXSize() - width;

  RGBLCDHandle.RGBLCDHdma2dHalHandle.XferCpltCallback  = DMA2D_TransferComplete;
	
  /* Foreground Configuration */
  RGBLCDHandle.RGBLCDHdma2dHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].InputAlpha = 0xFF;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].InputColorMode = DMA2D_INPUT_ARGB8888;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].InputOffset = 0;
	RGBLCDHandle.RGBLCDHdma2dHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].RedBlueSwap = DMA2D_RB_REGULAR; /* No ForeGround Red/Blue swap */
	RGBLCDHandle.RGBLCDHdma2dHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */
	
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Instance = DMA2D;

  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&RGBLCDHandle.RGBLCDHdma2dHalHandle) == HAL_OK)
  {
    if(HAL_DMA2D_ConfigLayer(&RGBLCDHandle.RGBLCDHdma2dHalHandle, RGBLCDHandle.ActiveLayer) == HAL_OK)
    {
			SCB_CleanDCache_by_Addr((uint32_t *)color_map, (width*height*4));
			if (HAL_DMA2D_Start_IT(&RGBLCDHandle.RGBLCDHdma2dHalHandle, (uint32_t)color_map, (uint32_t)address, width, height) == HAL_OK)
			{
				osEventFlagsWait (EvtIDGUI,0x00000001ul,osFlagsWaitAny, osWaitForever);
				SCB_CleanInvalidateDCache_by_Addr((uint32_t *)address, (width*height*4));
			}
    }
  }
#else
	CopyBuffer((const uint32_t *)color_map, (uint32_t *)RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].FBStartAdress, x1, y1, x2 - x1 + 1, y2 - y1 + 1);
#endif
	lv_flush_ready();
}
/**
 * Put a color map to a rectangular area
 * @param x1 left coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 * @param color_p pointer to an array of colors
 */
void Board_GUI_Map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
	/*Return if the area is out the screen*/
	if(x2 < 0) return;
	if(y2 < 0) return;
	if(x1 > LV_HOR_RES - 1) return;
	if(y1 > LV_VER_RES - 1) return;

	/*Truncate the area to the screen*/
	int32_t act_x1 = x1 < 0 ? 0 : x1;
	int32_t act_y1 = y1 < 0 ? 0 : y1;
	int32_t act_x2 = x2 > LV_HOR_RES - 1 ? LV_HOR_RES - 1 : x2;
	int32_t act_y2 = y2 > LV_VER_RES - 1 ? LV_VER_RES - 1 : y2;

#if LV_VDB_DOUBLE == 0
	uint32_t y;
	for(y = act_y1; y <= act_y2; y++) {
		memcpy((uint32_t *)RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].FBStartAdress + y * LV_HOR_RES + act_x1,
				color_p,
				(act_x2 - act_x1 + 1) * sizeof(((uint32_t *)RGBLCDHandle.RGBLCDLtdcHalHandle.LayerCfg[RGBLCDHandle.ActiveLayer].FBStartAdress)[0]));
		color_p += x2 - x1 + 1;    /*Skip the parts out of the screen*/
	}
#else

	x1_flush = act_x1;
	y1_flush = act_y1;
	x2_flush = act_x2;
	y2_fill = act_y2;
	y_fill_act = act_y1;
	buf_to_flush = color_p;


	/*##-7- Start the DMA transfer using the interrupt mode #*/
	/* Configure the source, destination and buffer size DMA fields and Start DMA Stream transfer */
	/* Enable All the DMA interrupts */
	if(HAL_DMA_Start_IT(&DmaHandle,(uint32_t)buf_to_flush, (uint32_t)&my_fb[y_fill_act * LV_HOR_RES + x1_flush],
			(x2_flush - x1_flush + 1)) != HAL_OK)
	{
		while(1)
		{
		}
	}

#endif
}
#if USE_LV_GPU
/**
 * Copy pixels to destination memory using opacity
 * @param dest a memory address. Copy 'src' here.
 * @param src pointer to pixel map. Copy it to 'dest'.
 * @param length number of pixels in 'src'
 * @param opa opacity (0, OPA_TRANSP: transparent ... 255, OPA_COVER, fully cover)
 */
void Board_GUI_Blend(lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
  /* Configure the DMA2D Mode, Color Mode and output offset */
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Init.Mode         = DMA2D_M2M_BLEND;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Init.OutputOffset = 0;

  RGBLCDHandle.RGBLCDHdma2dHalHandle.XferCpltCallback  = DMA2D_TransferComplete;
	
  /* Foreground Configuration */
  RGBLCDHandle.RGBLCDHdma2dHalHandle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.LayerCfg[1].InputAlpha = opa;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.LayerCfg[1].InputOffset = 0;

  RGBLCDHandle.RGBLCDHdma2dHalHandle.Instance = DMA2D;
  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&RGBLCDHandle.RGBLCDHdma2dHalHandle) == HAL_OK)
  {
    if(HAL_DMA2D_ConfigLayer(&RGBLCDHandle.RGBLCDHdma2dHalHandle, 1) == HAL_OK)
    {
			SCB_CleanDCache_by_Addr((uint32_t *)src, (length*4));
      if (HAL_DMA2D_BlendingStart_IT(&RGBLCDHandle.RGBLCDHdma2dHalHandle, (uint32_t)src, (uint32_t)dest, (uint32_t)dest, length, 1) == HAL_OK)
      {
        osEventFlagsWait (EvtIDGUI,0x00000001ul,osFlagsWaitAny, osWaitForever);
				SCB_CleanInvalidateDCache_by_Addr((uint32_t *)dest, (length*4));
      }
    }
  }
}

/**
 * Copy pixels to destination memory using opacity
 * @param dest a memory address. Copy 'src' here.
 * @param src pointer to pixel map. Copy it to 'dest'.
 * @param length number of pixels in 'src'
 * @param opa opacity (0, OPA_TRANSP: transparent ... 255, OPA_COVER, fully cover)
 */
void Board_GUI_FillOPA(lv_color_t * dest, uint32_t length, lv_color_t color)
{
  /* Register to memory mode with ARGB8888 as color Mode */
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Init.Mode         = DMA2D_R2M;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Init.OutputOffset = 0;

	RGBLCDHandle.RGBLCDHdma2dHalHandle.XferCpltCallback  = DMA2D_TransferComplete;
  RGBLCDHandle.RGBLCDHdma2dHalHandle.Instance = DMA2D;

  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&RGBLCDHandle.RGBLCDHdma2dHalHandle) == HAL_OK)
  {
    if(HAL_DMA2D_ConfigLayer(&RGBLCDHandle.RGBLCDHdma2dHalHandle, 1) == HAL_OK)
    {
      if (HAL_DMA2D_Start_IT(&RGBLCDHandle.RGBLCDHdma2dHalHandle, color.full, (uint32_t)dest, length, 1) == HAL_OK)
      {
        /* Polling For DMA transfer */
        osEventFlagsWait (EvtIDGUI,0x00000001ul,osFlagsWaitAny, osWaitForever);
				SCB_CleanInvalidateDCache_by_Addr((uint32_t *)dest, (length*4));
      }
    }
  }
}
#endif
#endif
