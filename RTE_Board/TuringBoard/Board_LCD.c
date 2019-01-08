#include "Board_LCD.h"
#include "Board_SPI.h"
#if RTE_USE_OS == 0
volatile bool LCDSPIReadyFlag = false;
#else 
osEventFlagsId_t EvtIDGUI;
#endif
static uint16_t Board_Pen_Read(uint8_t Command)
{
	uint16_t RetVal = 0;
  WRITE_REG(SpiHandle[SPI_LCD].SpiHalHandle.Instance->CFG1, 
	(SPI_BAUDRATEPRESCALER_256 | SpiHandle[SPI_LCD].SpiHalHandle.Init.CRCCalculation |
                                   SpiHandle[SPI_LCD].SpiHalHandle.Init.FifoThreshold | SpiHandle[SPI_LCD].SpiHalHandle.Init.DataSize));
	HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin , GPIO_PIN_RESET);
	uint16_t ReadValBuffer[16] = {0};
	for(uint8_t i=0;i<16;i++)
	{				 
		uint16_t ReadValue = 0;
		uint8_t ReadBuf1,ReadBuf2 = 0;
		HAL_SPI_Transmit(&SpiHandle[SPI_LCD].SpiHalHandle, &Command, 1, HAL_MAX_DELAY);
		HAL_SPI_Receive(&SpiHandle[SPI_LCD].SpiHalHandle, (uint8_t *)&ReadBuf1, 1, HAL_MAX_DELAY);
		HAL_SPI_Receive(&SpiHandle[SPI_LCD].SpiHalHandle, (uint8_t *)&ReadBuf2, 1, HAL_MAX_DELAY);
		ReadValue = (((uint16_t)ReadBuf1)<<8|ReadBuf2)>>3;
		ReadValBuffer[i] = ReadValue;
	}		
	HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin , GPIO_PIN_SET);
  WRITE_REG(SpiHandle[SPI_LCD].SpiHalHandle.Instance->CFG1, 
	(SpiHandle[SPI_LCD].SpiHalHandle.Init.BaudRatePrescaler | SpiHandle[SPI_LCD].SpiHalHandle.Init.CRCCalculation |
                                   SpiHandle[SPI_LCD].SpiHalHandle.Init.FifoThreshold | SpiHandle[SPI_LCD].SpiHalHandle.Init.DataSize));
	for(uint8_t i=0;i<16-1; i++)//排序
	{
		for(uint8_t j=i+1;j<16;j++)
		{
			if(ReadValBuffer[i]>ReadValBuffer[j])//升序排列
			{
				uint16_t temp = ReadValBuffer[i];
				ReadValBuffer[i] = ReadValBuffer[j];
				ReadValBuffer[j] = temp;
			}
		}
	}	  
	uint32_t Sum = 0;
	for(uint8_t i=5;i<13;i++) Sum+=ReadValBuffer[i];
	RetVal = Sum>>3;
	return RetVal;
}
static uint8_t Board_LCD_ReadXY(uint16_t *x,uint16_t *y)
{
	uint16_t xtemp,ytemp = 0;			 	 		  
	xtemp = Board_Pen_Read(0XD0);
	ytemp = Board_Pen_Read(0X90);	  												   
	if(xtemp<50||ytemp<50) return 0;//读数失败
	*x=xtemp;
	*y=ytemp;
	return 1;//读数成功
}
static uint8_t Board_LCD_ReadXY2(uint16_t *x,uint16_t *y) 
{
	uint16_t x1,y1;
 	uint16_t x2,y2;
 	uint8_t flag;    
	flag = Board_LCD_ReadXY(&x1,&y1);   
	if(flag==0)return(0);
	flag = Board_LCD_ReadXY(&x2,&y2);	   
	if(flag==0)return(0);   
	if(((x2<=x1&&x1<x2+50)||(x1<=x2&&x2<x1+50))  //前后两次采样在+-50内
	&&((y2<=y1&&y1<y2+50)||(y1<=y2&&y2<y1+50)))
	{
			*x=(x1+x2)/2;
			*y=(y1+y2)/2;
			return 1;
	}else return 0;	  
}  
bool Board_LCD_TouchScan(uint16_t *x,uint16_t *y)
{
	if(HAL_GPIO_ReadPin(LCD_PEN_GPIO_Port,LCD_PEN_Pin)==0)
	{
		if(Board_LCD_ReadXY2(x,y))
		{
			uint16_t vx=15542,vy=11165;  //比例因子，此值除以1000之后表示多少个AD值代表一个像素点
			uint16_t chx=140,chy=146;//默认像素点坐标为0时的AD起始值
			*x = *x>chx?((uint32_t)*x-(uint32_t)chx)*1000/vx:((uint32_t)chx-(uint32_t)*x)*1000/vx;
			*y = *y>chy?((uint32_t)*y-(uint32_t)chy)*1000/vy:((uint32_t)chy-(uint32_t)*y)*1000/vy;
			uint16_t temp = *x;
			*x = 320-*y;
			*y = temp;
			return true;
		}
	}
	return false;
}
uint16_t Board_LCD_Read(uint8_t cmd)
{
  WRITE_REG(SpiHandle[SPI_LCD].SpiHalHandle.Instance->CFG1, 
	(SPI_BAUDRATEPRESCALER_32 | SpiHandle[SPI_LCD].SpiHalHandle.Init.CRCCalculation |
                                   SpiHandle[SPI_LCD].SpiHalHandle.Init.FifoThreshold | SpiHandle[SPI_LCD].SpiHalHandle.Init.DataSize));
	// DC LOW
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);

	// SPI2 CS LOW
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SpiHandle[SPI_LCD].SpiHalHandle, &cmd, 1, HAL_MAX_DELAY);
	uint32_t RetVal = 0;
	HAL_SPI_Receive(&SpiHandle[SPI_LCD].SpiHalHandle, (uint8_t *)&RetVal, 4, HAL_MAX_DELAY);
	RTE_Printf("%x\r\n",RetVal);
	// SPI2 CS HIGH
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_SET);
  WRITE_REG(SpiHandle[SPI_LCD].SpiHalHandle.Instance->CFG1, 
	(SpiHandle[SPI_LCD].SpiHalHandle.Init.BaudRatePrescaler | SpiHandle[SPI_LCD].SpiHalHandle.Init.CRCCalculation |
                                   SpiHandle[SPI_LCD].SpiHalHandle.Init.FifoThreshold | SpiHandle[SPI_LCD].SpiHalHandle.Init.DataSize));
	return 0x00;
}
static void Board_LCD_WriteCommand(uint8_t cmd)
{
	// DC LOW
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);

	// SPI2 CS LOW
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SpiHandle[SPI_LCD].SpiHalHandle, &cmd, 1, HAL_MAX_DELAY);
	// SPI2 CS HIGH
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_SET);
}
static void Board_LCD_WriteData(uint8_t data)
{
	// DC HIGH
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);

	// SPI2 CS LOW
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SpiHandle[SPI_LCD].SpiHalHandle, &data, 1, HAL_MAX_DELAY);
	// SPI2 CS HIGH
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_SET);
}
static void Board_LCD_SetCursorPosition(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2)
{
	Board_LCD_WriteCommand(0x2A);
	Board_LCD_WriteData(x1 >> 8);
	Board_LCD_WriteData(x1 & 0xff);
	Board_LCD_WriteData(x2 >> 8);
	Board_LCD_WriteData(x2 & 0xff);

	Board_LCD_WriteCommand(0x2B);
	Board_LCD_WriteData(y1 >> 8);
	Board_LCD_WriteData(y1  &0xff);
	Board_LCD_WriteData(y2 >> 8);
	Board_LCD_WriteData(y2  &0xff);
}
void Board_LCD_PutPixel(uint32_t x, uint32_t y, uint16_t color)
{
	Board_LCD_SetCursorPosition(x, y, x, y);
	Board_LCD_WriteCommand(0x2C);
	color = (color) << 8 | (color) >> 8;
	// DC HIGH
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	// SPI2 CS LOW
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SpiHandle[SPI_LCD].SpiHalHandle, (uint8_t*)&color, 2, HAL_MAX_DELAY);
	// SPI2 CS HIGH
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_SET);
}
static void Board_LCD_Reset(void)
{
	// Low
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
	RTE_RoundRobin_DelayMS(50);
	// High
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	RTE_RoundRobin_DelayMS(50);
}
#if RTE_USE_OPENMV
#include "mv_core/imlib.h"
#endif
void Board_LCD_DrawData(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data,bool ifgray) 
{
	uint32_t pixnum = (w)*(h);
	uint32_t pixelremain = pixnum*2;
	uint8_t *RealData = NULL;
	Board_LCD_SetCursorPosition(x, x+w-1, y, y+h-1);
	Board_LCD_WriteCommand(0x2C);
	// DC HIGH
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	// SPI2 CS LOW
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_RESET);
	if(ifgray)
	{
#if RTE_USE_OPENMV
		uint16_t *tempbuf = fb_alloc0(w*h*2);
		RTE_AssertParam(tempbuf);
		for(uint16_t y=0;y<h;y++)
		{
			for(uint16_t x=0;x<w;x++)
			{
				uint8_t pixel = ((uint8_t*)data)[(y*w)+x];
				tempbuf[(y*w)+x] = IM_RGB565(IM_R825(pixel),IM_G826(pixel),IM_B825(pixel));
			}
		}
		fb_free();
		RealData = (uint8_t *)tempbuf;
#endif
	}
	else
	{
		RealData = (uint8_t *)data;
	}
	while(pixelremain > 0)
	{
		uint32_t sendpixel = 0;
		if(pixelremain < 65536)
		{
			sendpixel = pixelremain;
		}
		else
			sendpixel = 65536-1;
		HAL_SPI_Transmit_DMA(&SpiHandle[SPI_LCD].SpiHalHandle, (uint8_t *)(RealData), sendpixel);
#if RTE_USE_OS == 0
		LCDSPIReadyFlag = false;
		while(LCDSPIReadyFlag == false)
		{
			if(LCDSPIReadyFlag == true)
				break;
		}
#else
		osEventFlagsWait (EvtIDGUI,0x00000001ul,osFlagsWaitAny, osWaitForever);
#endif
		pixelremain -= sendpixel;
		RealData = RealData + sendpixel;
	}
	// SPI2 CS HIGH
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_SET);
}
void Board_LCD_FillFrame(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) 
{
	uint16_t *Buffer = RTE_MEM_Alloc0(MEM_RTE,(x1 - x0 +1)*2);
	color = ((color) << 8 | (color) >> 8);
	for(uint16_t i=0;i<(x1 - x0 + 1);i++)
		*(Buffer+i) = color;
	Board_LCD_SetCursorPosition(x0, x1, y0, y1);
	Board_LCD_WriteCommand(0x2C);
	// DC HIGH
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	// SPI2 CS LOW
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_RESET);
	for(uint32_t i = 0; i <(y1 - y0 + 1); i++) 
	{
		HAL_SPI_Transmit_DMA(&SpiHandle[SPI_LCD].SpiHalHandle, (uint8_t*)Buffer, 2*(x1 - x0 + 1));
#if RTE_USE_OS == 1
		osEventFlagsWait (EvtIDGUI,0x00000001ul,osFlagsWaitAny, osWaitForever);
#else
		LCDSPIReadyFlag = false;
		while(LCDSPIReadyFlag == false)
		{
			if(LCDSPIReadyFlag == true)
				break;
		}
#endif
	}
	RTE_MEM_Free(MEM_RTE,Buffer);
	// SPI2 CS HIGH
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_SET);
}
#if RTE_USE_GUI == 1
void Board_GUI_Fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
	Board_LCD_FillFrame(x1, y1, x2, y2 ,color.full & 0xFF);
}
void Board_GUI_Flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_map)
{
	Board_LCD_SetCursorPosition(x1, x2, y1, y2);
	Board_LCD_WriteCommand(0x2C);
	uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1);
	// DC HIGH
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	// SPI2 CS LOW
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_RESET);
	while(size > 0) {
		uint32_t SendPix = 0;
		if(size > UINT16_MAX/2)
			SendPix = UINT16_MAX;
		else
			SendPix = size*2;
		HAL_SPI_Transmit_DMA(&SpiHandle[SPI_LCD].SpiHalHandle, (uint8_t*)color_map, SendPix);
#if RTE_USE_OS == 1
		osEventFlagsWait (EvtIDGUI,0x00000001ul,osFlagsWaitAny, osWaitForever);
#else
		LCDSPIReadyFlag = false;
		while(LCDSPIReadyFlag == false)
		{
			if(LCDSPIReadyFlag == true)
				break;
		}
#endif
		size -= SendPix/2;
		color_map += SendPix/2;
	}
	// SPI2 CS HIGH
	HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_SET);
	lv_flush_ready();
}
bool Board_GUI_TouchScan(lv_indev_data_t *data)
{
	static int16_t last_x = 0;
	static int16_t last_y = 0;
	uint16_t x = 0;
	uint16_t y = 0;
	if(Board_LCD_TouchScan(&x,&y))
	{
		data->point.x = x;
		data->point.y = y;
		last_x = data->point.x;
		last_y = data->point.y;
		data->state = LV_INDEV_STATE_PR;
	}
	else
	{
		data->point.x = last_x;
		data->point.y = last_y;
		data->state = LV_INDEV_STATE_REL;
	}
	return false;
}
#endif

static void Board_LCD_Config(void)
{
	//************* Start Initial Sequence **********// 
	Board_LCD_WriteCommand(0xCF);  
	Board_LCD_WriteData(0x00); 
	Board_LCD_WriteData(0xD9); 
	Board_LCD_WriteData(0X30); 
	 
	Board_LCD_WriteCommand(0xED);  
	Board_LCD_WriteData(0x64); 
	Board_LCD_WriteData(0x03); 
	Board_LCD_WriteData(0X12); 
	Board_LCD_WriteData(0X81); 
	 
	Board_LCD_WriteCommand(0xE8);  
	Board_LCD_WriteData(0x85); 
	Board_LCD_WriteData(0x10); 
	Board_LCD_WriteData(0x78); 
	 
	Board_LCD_WriteCommand(0xCB);  
	Board_LCD_WriteData(0x39); 
	Board_LCD_WriteData(0x2C); 
	Board_LCD_WriteData(0x00); 
	Board_LCD_WriteData(0x34); 
	Board_LCD_WriteData(0x02); 
	 
	Board_LCD_WriteCommand(0xF7);  
	Board_LCD_WriteData(0x20); 
	 
	Board_LCD_WriteCommand(0xEA);  
	Board_LCD_WriteData(0x00); 
	Board_LCD_WriteData(0x00); 
	 
	Board_LCD_WriteCommand(0xC0);    //Power control 
	Board_LCD_WriteData(0x21);   //VRH[5:0] 
	 
	Board_LCD_WriteCommand(0xC1);    //Power control 
	Board_LCD_WriteData(0x12);   //SAP[2:0];BT[3:0] 
	 
	Board_LCD_WriteCommand(0xC5);    //VCM control 
	Board_LCD_WriteData(0x32); 
	Board_LCD_WriteData(0x3C); 
	 
	Board_LCD_WriteCommand(0xC7);    //VCM control2 
	Board_LCD_WriteData(0XC1); 
	 
	Board_LCD_WriteCommand(0x36);    // Memory Access Control 
	Board_LCD_WriteData(0xA8); 
	 
	/* Landscape 320 x 240 */
	Board_LCD_WriteCommand(0x2A);
	Board_LCD_WriteData(0x00);
	Board_LCD_WriteData(0x00);
	Board_LCD_WriteData(0x01);
	Board_LCD_WriteData(0x3F);

	Board_LCD_WriteCommand(0x2B);
	Board_LCD_WriteData(0x00);
	Board_LCD_WriteData(0x00);
	Board_LCD_WriteData(0x00);
	Board_LCD_WriteData(0xEF);

	Board_LCD_WriteCommand(0x3A);   
	Board_LCD_WriteData(0x55); 

	Board_LCD_WriteCommand(0xB1);   
	Board_LCD_WriteData(0x00);   
	Board_LCD_WriteData(0x10); 
	 
	Board_LCD_WriteCommand(0xB6);    // Display Function Control 
	Board_LCD_WriteData(0x0A); 
	Board_LCD_WriteData(0xA2); 
	
	Board_LCD_WriteCommand(0x44);
	Board_LCD_WriteData(0x02);

	Board_LCD_WriteCommand(0xF2);    // 3Gamma Function Disable 
	Board_LCD_WriteData(0x00); 
	 
	Board_LCD_WriteCommand(0x26);    //Gamma curve selected 
	Board_LCD_WriteData(0x01); 
	 
	Board_LCD_WriteCommand(0xE0);    //Set Gamma 
	Board_LCD_WriteData(0x0F); 
	Board_LCD_WriteData(0x20); 
	Board_LCD_WriteData(0x1E); 
	Board_LCD_WriteData(0x09); 
	Board_LCD_WriteData(0x12); 
	Board_LCD_WriteData(0x0B); 
	Board_LCD_WriteData(0x50); 
	Board_LCD_WriteData(0XBA); 
	Board_LCD_WriteData(0x44); 
	Board_LCD_WriteData(0x09); 
	Board_LCD_WriteData(0x14); 
	Board_LCD_WriteData(0x05); 
	Board_LCD_WriteData(0x23); 
	Board_LCD_WriteData(0x21); 
	Board_LCD_WriteData(0x00); 
	 
	Board_LCD_WriteCommand(0XE1);    //Set Gamma 
	Board_LCD_WriteData(0x00); 
	Board_LCD_WriteData(0x19); 
	Board_LCD_WriteData(0x19); 
	Board_LCD_WriteData(0x00); 
	Board_LCD_WriteData(0x12); 
	Board_LCD_WriteData(0x07); 
	Board_LCD_WriteData(0x2D); 
	Board_LCD_WriteData(0x28); 
	Board_LCD_WriteData(0x3F); 
	Board_LCD_WriteData(0x02); 
	Board_LCD_WriteData(0x0A); 
	Board_LCD_WriteData(0x08); 
	Board_LCD_WriteData(0x25); 
	Board_LCD_WriteData(0x2D); 
	Board_LCD_WriteData(0x0F); 
	
	Board_LCD_WriteCommand(0x11);    //Exit Sleep 
	RTE_RoundRobin_DelayMS(120); 
	Board_LCD_WriteCommand(0x29);    //Display on 
}
static void Board_LCD_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_CS2_GPIO_Port, LCD_CS2_Pin, GPIO_PIN_SET);
	
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI4_NSS_GPIO_Port, SPI4_NSS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : SPI4_NSS_Pin */
  GPIO_InitStruct.Pin = SPI4_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI4_NSS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_DC_Pin */
  GPIO_InitStruct.Pin = LCD_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LCD_DC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RST_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RST_GPIO_Port, &GPIO_InitStruct);
	
  /*Configure GPIO pin : PEN CS PIN */
  GPIO_InitStruct.Pin = LCD_CS2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LCD_CS2_GPIO_Port, &GPIO_InitStruct);
	
  /*Configure GPIO pin : pen */
	GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.Pin = LCD_PEN_Pin;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
//  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LCD_PEN_GPIO_Port, &GPIO_InitStructure);
  /* EXTI interrupt init*/
//  HAL_NVIC_SetPriority(LCD_IO_IRQn, LCD_IO_PreemptPriority, 0);
//  HAL_NVIC_EnableIRQ(LCD_IO_IRQn);
}
void Board_LCD_Init(void)
{
#if RTE_USE_OPENMV
	fb_alloc_init0();
#endif
#if RTE_USE_OS == 1
	EvtIDGUI = osEventFlagsNew(NULL);
#endif
	Board_LCD_GPIO_Init();
	Board_LCD_Reset();
  Board_LCD_Config();
	Board_LCD_Read(0xD3);
	Board_LCD_FillFrame(0,0,319,239,0x1234);
}
