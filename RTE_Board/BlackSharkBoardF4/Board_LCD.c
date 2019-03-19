#include "Board_LCD.h"
//写寄存器函数
//regval:寄存器值
static void Board_LCD_WR_REG(uint16_t regval)
{   
	LCD->LCD_REG = regval;//写入要写的寄存器序号	 
}
//写LCD数据
//data:要写入的值
static void Board_LCD_WR_DATA(uint16_t data)
{	 
	LCD->LCD_RAM = data;		 
}
//读LCD数据
//返回值:读到的值
static uint16_t Board_LCD_RD_DATA(void)
{		
	return LCD->LCD_RAM;		 
}					   
//写寄存器
//Board_LCD_Reg:寄存器地址
//Board_LCD_RegValue:要写入的数据
static void Board_LCD_WriteReg(uint16_t Reg,uint16_t Value)
{	
	LCD->LCD_REG = Reg;		//写入要写的寄存器序号	 
	LCD->LCD_RAM = Value;//写入数据	    		 
}	   
//读寄存器
//Board_LCD_Reg:寄存器地址
//返回值:读到的数据
uint16_t Board_LCD_ReadReg(uint16_t Board_Reg)
{										   
	Board_LCD_WR_REG(Board_Reg);		//写入要读的寄存器序号	  
	return Board_LCD_RD_DATA();		//返回读到的值
}   
//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
static void Board_LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{	  
	Board_LCD_WR_REG(0X2A); 
	Board_LCD_WR_DATA(Xpos>>8); 
	Board_LCD_WR_DATA(Xpos&0XFF);	 
	Board_LCD_WR_REG(0X2B); 
	Board_LCD_WR_DATA(Ypos>>8); 
	Board_LCD_WR_DATA(Ypos&0XFF);
}
//读取个某点的颜色值	 
//x,y:坐标
//返回值:此点的颜色
uint16_t Board_LCD_ReadPoint(uint16_t x,uint16_t y)
{
 	uint16_t r=0,g=0,b=0;   
	Board_LCD_SetCursor(x,y);	    
	Board_LCD_WR_REG(0X2E);//9341/6804/3510 发送读GRAM指令
 	Board_LCD_RD_DATA();									//dummy Read	    
 	r = Board_LCD_RD_DATA();  		  						//实际坐标颜色
	b = Board_LCD_RD_DATA(); 
	g = r&0XFF;		//对于9341/5310/5510,第一次读取的是RG的值,R在前,G在后,各占8位
	g <<=8;
	return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));//ILI9341/NT35310/NT35510需要公式转换一下
}
//快速画点
//x,y:坐标
//color:颜色
void Board_LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{	   
	Board_LCD_SetCursor(x,y);
	LCD->LCD_REG = 0x2C; 
	LCD->LCD_RAM =color; 
}	 
void Board_LCD_Set_Window(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2)
{   
	LCD->LCD_REG = 0x2A;
	Board_LCD_WR_DATA(x1 >> 8);
	Board_LCD_WR_DATA(x1 & 0xff);
	Board_LCD_WR_DATA(x2 >> 8);
	Board_LCD_WR_DATA(x2 & 0xff);

	LCD->LCD_REG = 0x2B;
	Board_LCD_WR_DATA(y1 >> 8);
	Board_LCD_WR_DATA(y1  &0xff);
	Board_LCD_WR_DATA(y2 >> 8);
	Board_LCD_WR_DATA(y2  &0xff);
} 
#if RTE_USE_GUI
//在指定区域内填充单个颜色
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
#if RTE_USE_OS == 0
static volatile bool DMAFlag = false;
#else
osEventFlagsId_t EvtIDLCD;
#endif
void GUI_LCD_Fill(int32_t sx,int32_t sy,int32_t ex,int32_t ey,lv_color_t color)
{           
	uint16_t width = ex-sx+1; 			//得到填充的宽度
	uint16_t height = ey-sy+1;			//高度
	Board_LCD_Set_Window(sx,ex,sy,ey);
	LCD->LCD_REG =0x2C;	 
	uint16_t *Linebuffer = Memory_Alloc0(BANK_DMA,width*2);
	for(uint16_t i=0;i<width;i++)
		Linebuffer[i] = color.full;
	for(uint16_t i=0;i<height;)
	{
		DMA2_Stream7->PAR = (uint32_t)Linebuffer;
		DMA_SetCurrDataCounter(DMA2_Stream7,width);
#if RTE_USE_OS == 0
		DMAFlag = false;
		DMA_Cmd(DMA2_Stream7, ENABLE);
		while(DMAFlag==false);
#else
		DMA_Cmd(DMA2_Stream7, ENABLE);
		uint32_t GetFlags = osEventFlagsWait(EvtIDLCD,0x00000001U,osFlagsWaitAny,osWaitForever);
		if(GetFlags != 0x00000001U)
			continue;
		i++;
#endif
	}
	Memory_Free(BANK_DMA,Linebuffer);
}  
void GUI_LCD_Map(int32_t sx,int32_t sy,int32_t ex,int32_t ey,const lv_color_t * color_map)
{  
	uint16_t width=ex-sx+1; 			//得到填充的宽度
	uint16_t height=ey-sy+1;			//高度
	Board_LCD_Set_Window(sx,ex,sy,ey);
	LCD->LCD_REG =0x2C;	 
	uint32_t pixnum = width*height;
	while(pixnum>0)
	{
		uint32_t sendpix = pixnum;
		if(sendpix>UINT16_MAX)
			sendpix = UINT16_MAX;
		DMA2_Stream7->PAR = (uint32_t)color_map;
		DMA_SetCurrDataCounter(DMA2_Stream7,sendpix);
#if RTE_USE_OS == 0
		DMAFlag = false;
		DMA_Cmd(DMA2_Stream7, ENABLE);
		while(DMAFlag==false);
#else
		DMA_Cmd(DMA2_Stream7, ENABLE);
		uint32_t GetFlags = osEventFlagsWait(EvtIDLCD,0x00000001U,osFlagsWaitAny,osWaitForever);
		if(GetFlags != 0x00000001U)
			continue;
#endif
		pixnum = pixnum - sendpix;
	}  
} 
//在指定区域内填充指定颜色块			 
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void GUI_LCD_Flush(int32_t sx,int32_t sy,int32_t ex,int32_t ey,const lv_color_t * color_map)
{  
	uint16_t width = ex-sx+1; 			//得到填充的宽度
	uint16_t height = ey-sy+1;			//高度
	Board_LCD_Set_Window(sx,ex,sy,ey);
	LCD->LCD_REG =0x2C;	 
	uint32_t pixnum = width*height;
	while(pixnum>0)
	{
		uint32_t sendpix = pixnum;
		if(sendpix>UINT16_MAX)
			sendpix = UINT16_MAX;
		DMA2_Stream7->PAR = (uint32_t)color_map;
		DMA_SetCurrDataCounter(DMA2_Stream7,sendpix);
#if RTE_USE_OS == 0
		DMAFlag = false;
		DMA_Cmd(DMA2_Stream7, ENABLE);
		while(DMAFlag==false);
#else
		DMA_Cmd(DMA2_Stream7, ENABLE);
		uint32_t GetFlags = osEventFlagsWait(EvtIDLCD,0x00000001U,osFlagsWaitAny,osWaitForever);
		if(GetFlags != 0x00000001U)
			continue;
#endif
		pixnum = pixnum - sendpix;
	} 
	lv_flush_ready();
}  
#endif
void Board_LCD_Init(void)
{
#if RTE_USE_OS
	EvtIDLCD = osEventFlagsNew(NULL);
#endif
  GPIO_InitTypeDef  GPIO_InitStructure;
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 
	FSMC_NORSRAMTimingInitTypeDef  writeTiming;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE, ENABLE);//使能IO时钟  
  RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);//使能FSMC时钟  
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//PB1 推挽输出,控制背光
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化 //PB15 推挽输出,控制背光
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7|GPIO_Pin_8
																|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;//PD0,1,4,5,8,9,10,14,15 AF OUT
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化  
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12
																|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;//PE7~15,AF OUT
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化  

  GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);// 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);// 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource7,GPIO_AF_FSMC); //和ZET6芯片差异	
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC); 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_FSMC);//和ZET6芯片差异	
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);// 
 
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);//PE7,AF12
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);
  GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);//PE15,AF12


  readWriteTiming.FSMC_AddressSetupTime = 0XF;	 //地址建立时间（ADDSET）为16个HCLK 1/168M=6ns*16=96ns	
  readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（ADDHLD）模式A未用到	
  readWriteTiming.FSMC_DataSetupTime = 60;			//数据保存时间为60个HCLK	=6*60=360ns
  readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
  readWriteTiming.FSMC_CLKDivision = 0x00;
  readWriteTiming.FSMC_DataLatency = 0x00;
  readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 
    
	writeTiming.FSMC_AddressSetupTime =9;	      //地址建立时间（ADDSET）为9个HCLK =54ns 
  writeTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（A		
  writeTiming.FSMC_DataSetupTime = 8;		 //数据保存时间为6ns*9个HCLK=54ns
  writeTiming.FSMC_BusTurnAroundDuration = 0x00;
  writeTiming.FSMC_CLKDivision = 0x00;
  writeTiming.FSMC_DataLatency = 0x00;
  writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;//  这里我们使用NE1 
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; // 不复用数据地址
  FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//存储器数据宽度为16bit   
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//  存储器写使能
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;   
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; // 读写使用不同的时序
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable; 
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //读写时序
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //写时序
  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //初始化FSMC配置
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  // 使能BANK1 
	
 	RTE_RoundRobin_DelayMS(50); // delay 50 ms 
 	Board_LCD_WriteReg(0x0000,0x0001);
	RTE_RoundRobin_DelayMS(50); // delay 50 ms 
	uint16_t LCDID = 0;
	LCDID = Board_LCD_ReadReg(0x0000); 
  if(LCDID<0XFF||LCDID==0XFFFF||LCDID==0X9300)//读到ID不正确,新增LCDID==0X9300判断，因为9341在未被复位的情况下会被读成9300
	{	
 		//尝试9341 ID的读取		
		Board_LCD_WR_REG(0XD3);				   
		LCDID=Board_LCD_RD_DATA();	//dummy read 	
 		LCDID=Board_LCD_RD_DATA(); 	//读到0X00
  	LCDID=Board_LCD_RD_DATA();   	//读取93								   
 		LCDID<<=8;
		LCDID|=Board_LCD_RD_DATA();  	//读取41 	   			   
	}
	if(LCDID==0X9341||LCDID==0X5310||LCDID==0X5510)//如果是这三个IC,则设置WR时序为最快
	{
		//重新配置写时序控制寄存器的时序   	 							    
		FSMC_Bank1E->BWTR[6]&=~(0XF<<0);//地址建立时间（ADDSET）清零 	 
		FSMC_Bank1E->BWTR[6]&=~(0XF<<8);//数据保存时间清零
		FSMC_Bank1E->BWTR[6]|=3<<0;		//地址建立时间（ADDSET）为3个HCLK =18ns  	 
		FSMC_Bank1E->BWTR[6]|=2<<8; 	//数据保存时间为6ns*3个HCLK=18ns
	}
	if(LCDID==0X9341)	//9341初始化
	{	 
		//************* Start Initial Sequence **********// 
		Board_LCD_WR_REG(0xCF);  
		Board_LCD_WR_DATA(0x00); 
		Board_LCD_WR_DATA(0xD9); 
		Board_LCD_WR_DATA(0X30); 
		 
		Board_LCD_WR_REG(0xED);  
		Board_LCD_WR_DATA(0x64); 
		Board_LCD_WR_DATA(0x03); 
		Board_LCD_WR_DATA(0X12); 
		Board_LCD_WR_DATA(0X81); 
		 
		Board_LCD_WR_REG(0xE8);  
		Board_LCD_WR_DATA(0x85); 
		Board_LCD_WR_DATA(0x10); 
		Board_LCD_WR_DATA(0x78); 
		 
		Board_LCD_WR_REG(0xCB);  
		Board_LCD_WR_DATA(0x39); 
		Board_LCD_WR_DATA(0x2C); 
		Board_LCD_WR_DATA(0x00); 
		Board_LCD_WR_DATA(0x34); 
		Board_LCD_WR_DATA(0x02); 
		 
		Board_LCD_WR_REG(0xF7);  
		Board_LCD_WR_DATA(0x20); 
		 
		Board_LCD_WR_REG(0xEA);  
		Board_LCD_WR_DATA(0x00); 
		Board_LCD_WR_DATA(0x00); 
		 
		Board_LCD_WR_REG(0xC0);    //Power control 
		Board_LCD_WR_DATA(0x21);   //VRH[5:0] 
		 
		Board_LCD_WR_REG(0xC1);    //Power control 
		Board_LCD_WR_DATA(0x12);   //SAP[2:0];BT[3:0] 
		 
		Board_LCD_WR_REG(0xC5);    //VCM control 
		Board_LCD_WR_DATA(0x32); 
		Board_LCD_WR_DATA(0x3C); 
		 
		Board_LCD_WR_REG(0xC7);    //VCM control2 
		Board_LCD_WR_DATA(0XC1); 
		 
		Board_LCD_WR_REG(0x36);    // Memory Access Control 
		Board_LCD_WR_DATA(0xA8); 
		 
		/* Landscape 320 x 240 */
		Board_LCD_WR_REG(0x2A);
		Board_LCD_WR_DATA(0x00);
		Board_LCD_WR_DATA(0x00);
		Board_LCD_WR_DATA(0x01);
		Board_LCD_WR_DATA(0x3F);

		Board_LCD_WR_REG(0x2B);
		Board_LCD_WR_DATA(0x00);
		Board_LCD_WR_DATA(0x00);
		Board_LCD_WR_DATA(0x00);
		Board_LCD_WR_DATA(0xEF);

		Board_LCD_WR_REG(0x3A);   
		Board_LCD_WR_DATA(0x55); 

		Board_LCD_WR_REG(0xB1);   
		Board_LCD_WR_DATA(0x00);   
		Board_LCD_WR_DATA(0x10); 
		 
		Board_LCD_WR_REG(0xB6);    // Display Function Control 
		Board_LCD_WR_DATA(0x0A); 
		Board_LCD_WR_DATA(0xA2); 
		
		Board_LCD_WR_REG(0x44);
		Board_LCD_WR_DATA(0x02);

		Board_LCD_WR_REG(0xF2);    // 3Gamma Function Disable 
		Board_LCD_WR_DATA(0x00); 
		 
		Board_LCD_WR_REG(0x26);    //Gamma curve selected 
		Board_LCD_WR_DATA(0x01); 
		 
		Board_LCD_WR_REG(0xE0);    //Set Gamma 
		Board_LCD_WR_DATA(0x0F); 
		Board_LCD_WR_DATA(0x20); 
		Board_LCD_WR_DATA(0x1E); 
		Board_LCD_WR_DATA(0x09); 
		Board_LCD_WR_DATA(0x12); 
		Board_LCD_WR_DATA(0x0B); 
		Board_LCD_WR_DATA(0x50); 
		Board_LCD_WR_DATA(0XBA); 
		Board_LCD_WR_DATA(0x44); 
		Board_LCD_WR_DATA(0x09); 
		Board_LCD_WR_DATA(0x14); 
		Board_LCD_WR_DATA(0x05); 
		Board_LCD_WR_DATA(0x23); 
		Board_LCD_WR_DATA(0x21); 
		Board_LCD_WR_DATA(0x00); 
		 
		Board_LCD_WR_REG(0XE1);    //Set Gamma 
		Board_LCD_WR_DATA(0x00); 
		Board_LCD_WR_DATA(0x19); 
		Board_LCD_WR_DATA(0x19); 
		Board_LCD_WR_DATA(0x00); 
		Board_LCD_WR_DATA(0x12); 
		Board_LCD_WR_DATA(0x07); 
		Board_LCD_WR_DATA(0x2D); 
		Board_LCD_WR_DATA(0x28); 
		Board_LCD_WR_DATA(0x3F); 
		Board_LCD_WR_DATA(0x02); 
		Board_LCD_WR_DATA(0x0A); 
		Board_LCD_WR_DATA(0x08); 
		Board_LCD_WR_DATA(0x25); 
		Board_LCD_WR_DATA(0x2D); 
		Board_LCD_WR_DATA(0x0F); 
		
		Board_LCD_WR_REG(0x11);    //Exit Sleep 
		RTE_RoundRobin_DelayMS(120); 
		Board_LCD_WR_REG(0x29);    //Display on 
	}
	else
		RTE_Assert(__FILE__,__LINE__);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x10;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

	DMA_InitTypeDef DMA_InitStructure;
	/* Enable the DMA2 Clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	/* DMA2 Stream7 disable */
	DMA_DeInit(DMA2_Stream7);
  while((DMA_GetCmdStatus(DMA2_Stream7) != DISABLE))
	{  
	}
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;                              //数据流
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)0;                     //外设地址  FSMC总线写地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&LCD->LCD_RAM;            //DMA访问的数据地址 启用DMA时赋值
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToMemory;                         //外设作为数据传输的目的地
	DMA_InitStructure.DMA_BufferSize = 0;                                       //传输数据量大小 启用DMA时赋值
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;             //外设地址不增加
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;                    //内存地址自增1
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //外设数据宽度为16bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         //DMA从内存中搬运数据宽度为byte
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                               //只传送一次
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                     //(DMA传送优先级为高) 
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;                      //指定使用FIFO模式还是直接模式
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;               //指定了FIFO阈值
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;                 //内存突发传输每次转移一个数据
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;         //外设突发传输每次转移一个数据
	DMA_Init(DMA2_Stream7, &DMA_InitStructure);
	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_1);
	GUI_LCD_Fill(0,0,319,239,LV_COLOR_GREEN);
}
void DMA2_Stream7_IRQHandler(void)
{
#if RTE_USE_OS == 0
	DMAFlag = true;
#else
	osEventFlagsSet(EvtIDLCD,0x00000001U);
#endif
	DMA_ClearITPendingBit(DMA2_Stream7,DMA_IT_TCIF7);
}


