#include "Board_Touch.h"
void Board_Touch_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOB,C,F时钟
  // SPI Alternative-Funktions mit den IO-Pins verbinden
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
	//GPIOC5初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//PC5 设置为上拉输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化
	
  // SPI-Clock enable
	SPI_InitTypeDef  SPI_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);  
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI2, &SPI_InitStructure); 
  // SPI enable
  SPI_Cmd(SPI2, ENABLE); 
}
static void Board_Touch_SendByte(uint8_t byte)
{ 
  // warte bis senden fertig
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  // byte senden
  SPI_I2S_SendData(SPI2, byte); 			    
}
static uint16_t Board_Touch_WriteReadHalfWord(uint8_t Command)
{
	uint8_t Buffer1,Buffer2 = 0x00;
	uint16_t RetVal = 0;
	TOUCH_CS_LOW(); 		//选中触摸屏IC
	Board_Touch_SendByte(Command);//发送命令字
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET); 
  SPI_I2S_ReceiveData(SPI2);
	Board_Touch_SendByte(0x00);//发送命令字
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET); 
  Buffer1 = SPI_I2S_ReceiveData(SPI2);
	Board_Touch_SendByte(0x00);//发送命令字
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET); 
  Buffer2 = SPI_I2S_ReceiveData(SPI2);
	RetVal = (((uint16_t)Buffer1)<<8|Buffer2)>>3;
	TOUCH_CS_HIGH();		//释放片选	 
	return(RetVal); 
}
static uint16_t Board_Pen_Read(uint8_t Command)
{
	uint16_t RetVal = 0;
	uint16_t ReadValBuffer[16] = {0};
	for(uint8_t i=0;i<16;i++)
	{				 
		ReadValBuffer[i] = Board_Touch_WriteReadHalfWord(Command);
	}		
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
bool Board_TouchScan(uint16_t *x,uint16_t *y)
{
	if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)==0)
	{
		if(Board_LCD_ReadXY2(x,y))
		{
			uint16_t vx=15542,vy=11165;
			uint16_t chx=140,chy=146;
			*x = *x>chx?((uint32_t)*x-(uint32_t)chx)*1000/vx:((uint32_t)chx-(uint32_t)*x)*1000/vx;
			*y = *y>chy?((uint32_t)*y-(uint32_t)chy)*1000/vy:((uint32_t)chy-(uint32_t)*y)*1000/vy;
			uint16_t temp = *x;
			*x = 320-*y;
			*y = 240 - temp;
			return true;
		}
	}
	return false;
}
#if RTE_USE_GUI
bool GUI_TouchScan(lv_indev_data_t *data)
{
	static int16_t last_x = 0;
	static int16_t last_y = 0;
	uint16_t x = 0;
	uint16_t y = 0;
	
	if(Board_TouchScan(&x,&y))
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
