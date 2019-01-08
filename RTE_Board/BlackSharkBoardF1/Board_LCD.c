#include "Board_LCD.h"
/****************************************************************************
* 名    称：SPI2_Init(void)
* 功    能：STM32_SPI2硬件配置初始化
* 入口参数：无
* 出口参数：无
* 说    明：STM32_SPI2硬件配置初始化
****************************************************************************/
void Board_LCD_SPI2Init(void)	
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	 
	//配置SPI2管脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_14;    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(GPIOB, &GPIO_InitStructure);  

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);//片选信号为低
	GPIO_ResetBits(GPIOC, GPIO_Pin_6);//信号为低
	
	//SPI2配置选项
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2 ,ENABLE);
	   
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	//使能SPI2
	SPI_Cmd(SPI2, ENABLE);      
}
/****************************************************************************
* 名    称：uint8_t SPI_WriteReadByte(SPI_TypeDef* SPIx,uint8_t Byte)
* 功    能：STM32_硬件SPI读写一个字节数据底层函数
* 入口参数：SPIx,Byte
* 出口参数：返回总线收到的数据
* 说    明：STM32_硬件SPI读写一个字节数据底层函数
****************************************************************************/
uint8_t SPI_WriteReadByte(SPI_TypeDef* SPIx,uint8_t TxData)
{	
	unsigned int retry=0;				 
	while(SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_TXE)!=SET);
	SPIx->DR = TxData;	 	  //发送一个byte 
	retry=0;
	while((SPIx->SR&1<<0)==0) //等待接收完一个byte  
	{
		retry++;
		if(retry>2000)return 0;
	}	  						    
	return SPIx->DR;          //返回收到的数据		
} 
//******************************************************************
//函数名：  LCD_WR_REG
//功能：    向液晶屏总线写入写8位指令
//输入参数：Reg:待写入的指令值
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_WR_REG(uint16_t cmd)
{ 
	LCD_CS_CLR;
	LCD_RS_CLR; 
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)!=SET);
	SPI2->DR = cmd&0xFF;
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)!=SET);
	SPI2->DR;
	LCD_CS_SET;
}
//******************************************************************
//函数名：  LCD_WR_DATA
//功能：    向液晶屏总线写入写8位数据
//输入参数：Data:待写入的数据
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_WR_DATA(uint8_t data)
{
	LCD_CS_CLR;
	LCD_RS_SET; 
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)!=SET);
	SPI2->DR = data;
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)!=SET);
	SPI2->DR;
	LCD_CS_SET;
}
//******************************************************************
//函数名：  LCD_WriteReg
//功能：    写寄存器数据
//输入参数：LCD_Reg:寄存器地址
//			LCD_RegValue:要写入的数据
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{	
	LCD_WR_REG(LCD_Reg);  
	LCD_WR_DATA(LCD_RegValue);	    		 
}	   
//******************************************************************
//函数名：  LCD_WriteRAM_Prepare
//功能：    开始写GRAM
//			在给液晶屏传送RGB数据前，应该发送写GRAM指令
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(0x2C);
}	 
/*************************************************
函数名：LCD_SetWindows
功能：设置lcd显示窗口，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
*************************************************/
void LCD_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd)
{
	LCD_WR_REG(0x2A);	
	LCD_WR_DATA(xStar>>8);
	LCD_WR_DATA(0x00FF&xStar+2);		
	LCD_WR_DATA(xEnd>>8);
	LCD_WR_DATA(0x00FF&xEnd+2);

	LCD_WR_REG(0x2B);	
	LCD_WR_DATA(yStar>>8);
	LCD_WR_DATA(0x00FF&yStar+3);		
	LCD_WR_DATA(yEnd>>8);
	LCD_WR_DATA(0x00FF&yEnd+3);	
	LCD_WriteRAM_Prepare();	//开始写入GRAM				
}   
//******************************************************************
//函数名：  LCD_DrawPoint
//功能：    在指定位置写入一个像素点数据
//输入参数：(x,y):光标坐标
//返回值：  无
//修改记录：无
//******************************************************************
void Board_LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_SetWindows(x,y,x,y);//设置光标位置 
	LCD_CS_CLR;
	LCD_RS_SET;  
	SPI_DataSizeConfig(SPI2,SPI_DataSize_16b);
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)!=SET);
	SPI2->DR = color;
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)!=SET);
	SPI2->DR;
	SPI_DataSizeConfig(SPI2,SPI_DataSize_8b);
	LCD_CS_SET;
}
//******************************************************************
//函数名：  LCD_Clear
//功能：    LCD全屏填充清屏函数
//输入参数：Color:要清屏的填充色
//返回值：  无
//修改记录：无
//******************************************************************
void Board_LCD_FillRect(uint16_t x,uint16_t y,uint16_t x1,uint16_t y1,uint16_t color)
{
	uint16_t w = x1 - x + 1;
	uint16_t h = y1 - y + 1;
	uint32_t pixnum = w*h;
	LCD_SetWindows(x,y,x1,y1);
	LCD_CS_CLR;
	LCD_RS_SET; 
	SPI_DataSizeConfig(SPI2,SPI_DataSize_16b);
	for(uint32_t i=0;i<pixnum;i++)
	{
		while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)!=SET);
		SPI2->DR = color;
		while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)!=SET);
		SPI2->DR;
	}
	SPI_DataSizeConfig(SPI2,SPI_DataSize_8b);
	LCD_CS_SET;
}   
//******************************************************************
//函数名：  LCD_Init
//功能：    LCD初始化
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void Board_LCD_Init(void)
{  
	Board_LCD_SPI2Init();
	//************* Start Initial Sequence **********//		
	//开始初始化液晶屏
	LCD_WR_REG(0x11);//Sleep exit 
	RTE_RoundRobin_DelayMS(120);		
	//ST7735R Frame Rate
	LCD_WR_REG(0xB1); 
	LCD_WR_DATA(0x01); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x2D); 

	LCD_WR_REG(0xB2); 
	LCD_WR_DATA(0x01); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x2D); 

	LCD_WR_REG(0xB3); 
	LCD_WR_DATA(0x01); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x2D); 
	LCD_WR_DATA(0x01); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x2D); 
	
	LCD_WR_REG(0xB4); //Column inversion 
	LCD_WR_DATA(0x07); 
	
	//ST7735R Power Sequence
	LCD_WR_REG(0xC0); 
	LCD_WR_DATA(0xA2); 
	LCD_WR_DATA(0x02); 
	LCD_WR_DATA(0x84); 
	LCD_WR_REG(0xC1); 
	LCD_WR_DATA(0xC5); 

	LCD_WR_REG(0xC2); 
	LCD_WR_DATA(0x0A); 
	LCD_WR_DATA(0x00); 

	LCD_WR_REG(0xC3); 
	LCD_WR_DATA(0x8A); 
	LCD_WR_DATA(0x2A); 
	LCD_WR_REG(0xC4); 
	LCD_WR_DATA(0x8A); 
	LCD_WR_DATA(0xEE); 
	
	LCD_WR_REG(0xC5); //VCOM 
	LCD_WR_DATA(0x0E); 
	
	LCD_WR_REG(0x36); //MX, MY, RGB mode 				 
	LCD_WR_DATA(0xC8); 
	
	//ST7735R Gamma Sequence
	LCD_WR_REG(0xe0); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_DATA(0x1a); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_DATA(0x18); 
	LCD_WR_DATA(0x2f); 
	LCD_WR_DATA(0x28); 
	LCD_WR_DATA(0x20); 
	LCD_WR_DATA(0x22); 
	LCD_WR_DATA(0x1f); 
	LCD_WR_DATA(0x1b); 
	LCD_WR_DATA(0x23); 
	LCD_WR_DATA(0x37); 
	LCD_WR_DATA(0x00); 	
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x02); 
	LCD_WR_DATA(0x10); 

	LCD_WR_REG(0xe1); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_DATA(0x1b); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_DATA(0x17); 
	LCD_WR_DATA(0x33); 
	LCD_WR_DATA(0x2c); 
	LCD_WR_DATA(0x29); 
	LCD_WR_DATA(0x2e); 
	LCD_WR_DATA(0x30); 
	LCD_WR_DATA(0x30); 
	LCD_WR_DATA(0x39); 
	LCD_WR_DATA(0x3f); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x03); 
	LCD_WR_DATA(0x10);  
	
	LCD_WR_REG(0x2a);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x7f);

	LCD_WR_REG(0x2b);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x9f);
	
	LCD_WR_REG(0xF0); //Enable test command  
	LCD_WR_DATA(0x01); 
	LCD_WR_REG(0xF6); //Disable ram power save mode 
	LCD_WR_DATA(0x00); 
	
	LCD_WR_REG(0x3A); //65k mode 
	LCD_WR_DATA(0x05); 	
	LCD_WR_REG(0x29);//Display on	

	LCD_WriteReg(0x36,0xC8);//设置LCD参数	 
	RTE_RoundRobin_DelayMS(200);
}
