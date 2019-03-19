#include "BSP_Com.h"
//--------------------------------------------------------------
// Definition aller UARTs
// Reihenfolge wie bei UART_NAME_t
//--------------------------------------------------------------
static BSP_COM_Handle_t ComControlArray[COM_N] = {
	// Name, Clock               , AF-UART      ,UART  , Baud , Interrupt
  {
		COM_DEBUG,RCC_APB2Periph_USART1,GPIO_AF_USART1,USART1,USART1_IRQn,
		// PORT , PIN      , Clock              , Source
		{GPIOA,GPIO_Pin_9,RCC_AHB1Periph_GPIOA,GPIO_PinSource9}, 
		{GPIOA,GPIO_Pin_10,RCC_AHB1Periph_GPIOA,GPIO_PinSource10},
		//DMA2数据流2 通道4
		DMA2_Stream2,DMA_Channel_4,
		//BufferLen QUENELEN 
		SHELL_BUFSIZE,SHELL_BUFSIZE*2,
		0x09,
	},
  {
		COM_WIFI,RCC_APB1Periph_USART2,GPIO_AF_USART2,USART2,USART2_IRQn,
		// PORT , PIN      , Clock              , Source
		{GPIOA,GPIO_Pin_2,RCC_AHB1Periph_GPIOA,GPIO_PinSource2},
		{GPIOA,GPIO_Pin_3,RCC_AHB1Periph_GPIOA,GPIO_PinSource3},
		//DMA1数据流5 通道4
		DMA1_Stream5,DMA_Channel_4,
		//BufferLen QUENELEN 
		2048,8192,
		0x03,
	},
	// Name, Clock               , AF-UART      ,UART  , Baud , Interrupt
  {
		COM_RFID,RCC_APB1Periph_USART3,GPIO_AF_USART3,USART3,USART3_IRQn, 
		// PORT , PIN      , Clock              , Source
		{GPIOB,GPIO_Pin_10,RCC_AHB1Periph_GPIOB,GPIO_PinSource10},  
		{GPIOB,GPIO_Pin_11,RCC_AHB1Periph_GPIOB,GPIO_PinSource11},
		DMA1_Stream1,DMA_Channel_4,
		//BufferLen QUENELEN 
		64,256,
		0x04,
	},
};
//--------------------------------------------------------------
// init aller UARTs
//--------------------------------------------------------------
void BSP_COM_Init(BSP_COM_Name_e uart,uint32_t baudrate)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	// Clock enable der TX und RX Pins
	RCC_AHB1PeriphClockCmd(ComControlArray[uart].TX.CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(ComControlArray[uart].RX.CLK, ENABLE);

	// Clock enable der UART
	if((ComControlArray[uart].UART==USART1) || (ComControlArray[uart].UART==USART6)) {
		RCC_APB2PeriphClockCmd(ComControlArray[uart].CLK, ENABLE);
	}
	else {
		RCC_APB1PeriphClockCmd(ComControlArray[uart].CLK, ENABLE);
	}

	// UART Alternative-Funktions mit den IO-Pins verbinden
	GPIO_PinAFConfig(ComControlArray[uart].TX.PORT,ComControlArray[uart].TX.SOURCE,ComControlArray[uart].AF);
	GPIO_PinAFConfig(ComControlArray[uart].RX.PORT,ComControlArray[uart].RX.SOURCE,ComControlArray[uart].AF);

	// UART als Alternative-Funktion mit PushPull
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	// TX-Pin
	GPIO_InitStructure.GPIO_Pin = ComControlArray[uart].TX.PIN;
	GPIO_Init(ComControlArray[uart].TX.PORT, &GPIO_InitStructure);
	// RX-Pin
	GPIO_InitStructure.GPIO_Pin =  ComControlArray[uart].RX.PIN;
	GPIO_Init(ComControlArray[uart].RX.PORT, &GPIO_InitStructure);

	// Oversampling
	USART_OverSampling8Cmd(ComControlArray[uart].UART, ENABLE);

	// init mit Baudrate, 8Databits, 1Stopbit, keine Paritaet, kein RTS+CTS
	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(ComControlArray[uart].UART, &USART_InitStructure);
	
	USART_ClearFlag(ComControlArray[uart].UART, USART_FLAG_TC);  
	// RX-Interrupt enable
	USART_ITConfig(ComControlArray[uart].UART, USART_IT_IDLE, ENABLE);

	// enable UART Interrupt-Vector
	NVIC_InitStructure.NVIC_IRQChannel = ComControlArray[uart].INT;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ComControlArray[uart].IDLEPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// RX-Puffer vorbereiten
	//申请内存
	ComControlArray[uart].ComBuffer.pu8Databuf = (uint8_t *)Memory_Alloc0(BANK_DMA,ComControlArray[uart].DataBufferLen);
	ComControlArray[uart].ComBuffer.u16Datalength = 0;
	RTE_MessageQuene_Init(&ComControlArray[uart].ComBuffer.ComQuene,ComControlArray[uart].DataQueneLen);
	
	//DMA配置
	DMA_InitTypeDef  DMA_InitStructure;
	if((uint32_t)ComControlArray[uart].RXDMAStream>(uint32_t)DMA2)//得到当前stream是属于DMA2还是DMA1
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2时钟使能 
	else 
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA1时钟使能 
  DMA_DeInit(ComControlArray[uart].RXDMAStream);
  while (DMA_GetCmdStatus(ComControlArray[uart].RXDMAStream) != DISABLE){}//等待DMA可配置 
  /* 配置 DMA Stream */
  DMA_InitStructure.DMA_Channel = ComControlArray[uart].RXDMAChannel;  //通道选择
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ComControlArray[uart].UART->DR);//DMA外设地址
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ComControlArray[uart].ComBuffer.pu8Databuf;//DMA 存储器0地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//外设到存储器模式
  DMA_InitStructure.DMA_BufferSize = ComControlArray[uart].DataBufferLen;//数据传输量 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//存储器数据长度:8位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 使用循环模式
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//中等优先级
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
  DMA_Init(ComControlArray[uart].RXDMAStream, &DMA_InitStructure);//初始化DMA Stream
	// UART enable
	USART_DMACmd(ComControlArray[uart].UART, USART_DMAReq_Rx, ENABLE);// 使能DMA串口发送和接受请求
	USART_Cmd(ComControlArray[uart].UART, ENABLE);
	DMA_Cmd(ComControlArray[uart].RXDMAStream, ENABLE);                           //启动DMA
}
//--------------------------------------------------------------
// ein Byte per UART senden
//--------------------------------------------------------------
void BSP_COM_SendByte(BSP_COM_Name_e uart, uint16_t wert)
{
  // warten bis altes Byte gesendet wurde
  while (USART_GetFlagStatus(ComControlArray[uart].UART, USART_FLAG_TXE) == RESET);
  USART_SendData(ComControlArray[uart].UART, wert);
}

//--------------------------------------------------------------
// ein Daten Array per UART senden
// cnt = Anzahl der Daten die gesendet werden sollen
//--------------------------------------------------------------
void BSP_COM_SendArray(BSP_COM_Name_e uart, uint8_t *data, uint16_t cnt)
{
  uint32_t n;
  if(cnt==0) return;
  // sende alle Daten
  for(n=0;n<cnt;n++) {
    BSP_COM_SendByte(uart,data[n]);
  }  
}
BSP_COM_Data_t* BSP_COM_ReturnQue(BSP_COM_Name_e uart)
{
	return &ComControlArray[uart].ComBuffer;
}
static uint32_t BSP_COM_GetDMAITTransfer(BSP_COM_Name_e uart)
{
	if(ComControlArray[uart].RXDMAStream == DMA2_Stream0||ComControlArray[uart].RXDMAStream == DMA1_Stream0)
		return DMA_IT_TCIF0;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream1||ComControlArray[uart].RXDMAStream == DMA1_Stream1)
		return DMA_IT_TCIF1;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream2||ComControlArray[uart].RXDMAStream == DMA1_Stream2)
		return DMA_IT_TCIF2;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream3||ComControlArray[uart].RXDMAStream == DMA1_Stream3)
		return DMA_IT_TCIF3;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream4||ComControlArray[uart].RXDMAStream == DMA1_Stream4)
		return DMA_IT_TCIF4;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream5||ComControlArray[uart].RXDMAStream == DMA1_Stream5)
		return DMA_IT_TCIF5;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream6||ComControlArray[uart].RXDMAStream == DMA1_Stream6)
		return DMA_IT_TCIF6;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream7||ComControlArray[uart].RXDMAStream == DMA1_Stream7)
		return DMA_IT_TCIF7;
	return 0;
}	
static uint32_t BSP_COM_GetDMAITError(BSP_COM_Name_e uart)
{
	if(ComControlArray[uart].RXDMAStream == DMA2_Stream0||ComControlArray[uart].RXDMAStream == DMA1_Stream0)
		return DMA_IT_TEIF0;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream1||ComControlArray[uart].RXDMAStream == DMA1_Stream1)
		return DMA_IT_TEIF1;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream2||ComControlArray[uart].RXDMAStream == DMA1_Stream2)
		return DMA_IT_TEIF2;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream3||ComControlArray[uart].RXDMAStream == DMA1_Stream3)
		return DMA_IT_TEIF3;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream4||ComControlArray[uart].RXDMAStream == DMA1_Stream4)
		return DMA_IT_TEIF4;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream5||ComControlArray[uart].RXDMAStream == DMA1_Stream5)
		return DMA_IT_TEIF5;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream6||ComControlArray[uart].RXDMAStream == DMA1_Stream6)
		return DMA_IT_TEIF6;
	else if(ComControlArray[uart].RXDMAStream == DMA2_Stream7||ComControlArray[uart].RXDMAStream == DMA1_Stream7)
		return DMA_IT_TEIF7;
	return 0;
}	
static void BSP_COM_RecCallback(BSP_COM_Name_e uart)
{
	ComControlArray[uart].UART->SR;
	ComControlArray[uart].UART->DR;
	USART_ClearITPendingBit(ComControlArray[uart].UART, USART_IT_IDLE);    //清除串口空闲
	ComControlArray[uart].ComBuffer.u16Datalength = ComControlArray[uart].DataBufferLen - DMA_GetCurrDataCounter(ComControlArray[uart].RXDMAStream);//可以接收的总长度减去剩余长度，得到接收到数据的长度
	if(ComControlArray[uart].ComBuffer.u16Datalength)
	{
		if(uart == COM_DEBUG)
			RTE_Shell_Input(ComControlArray[uart].ComBuffer.pu8Databuf,ComControlArray[uart].ComBuffer.u16Datalength);
		else if(uart == COM_WIFI)
		{
			extern osThreadId_t ThreadIDWIFI;
			RTE_MessageQuene_In(&ComControlArray[uart].ComBuffer.ComQuene,ComControlArray[uart].ComBuffer.pu8Databuf,
						ComControlArray[uart].ComBuffer.u16Datalength);
			osThreadFlagsSet(ThreadIDWIFI,0x00000001U);
		}
		else
		{
			extern osThreadId_t ThreadIDRFID;
			RTE_MessageQuene_In(&ComControlArray[uart].ComBuffer.ComQuene,ComControlArray[uart].ComBuffer.pu8Databuf,
						ComControlArray[uart].ComBuffer.u16Datalength);
			osThreadFlagsSet(ThreadIDRFID,0x00000001U);
		}
	}
	memset(ComControlArray[uart].ComBuffer.pu8Databuf,0,ComControlArray[uart].ComBuffer.u16Datalength);
	ComControlArray[uart].ComBuffer.u16Datalength = 0;
	DMA_Cmd(ComControlArray[uart].RXDMAStream,DISABLE); //关闭DMA传输
	DMA_ClearITPendingBit(ComControlArray[uart].RXDMAStream, BSP_COM_GetDMAITTransfer(uart));	// Clear Transfer Complete flag
	DMA_ClearITPendingBit(ComControlArray[uart].RXDMAStream, BSP_COM_GetDMAITError(uart));	// Clear Transfer error flag
	DMA_SetCurrDataCounter(ComControlArray[uart].RXDMAStream,ComControlArray[uart].DataBufferLen);          //设置传输数据长度 
	DMA_Cmd(ComControlArray[uart].RXDMAStream, ENABLE);                      //开启DMA
}


//--------------------------------------------------------------
// UART1-Interrupt
//--------------------------------------------------------------
void USART1_IRQHandler(void) {
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
		BSP_COM_RecCallback(COM_DEBUG);
}
//--------------------------------------------------------------
// USART2-Interrupt
//--------------------------------------------------------------
void USART2_IRQHandler(void) {
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
		BSP_COM_RecCallback(COM_WIFI);
}
//--------------------------------------------------------------
// USART3-Interrupt
//--------------------------------------------------------------
void USART3_IRQHandler(void) {
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
		BSP_COM_RecCallback(COM_RFID);
}

////--------------------------------------------------------------
//// UART2-Interrupt
////--------------------------------------------------------------
//void USART2_IRQHandler(void) {
//  uint16_t wert;

//  if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
//    // wenn ein Byte im Empfangspuffer steht
//    wert=USART_ReceiveData(USART2);
//    // Byte speichern
//    P_UART_RX_INT(USART2_IRQn,wert);
//  }
//}



////--------------------------------------------------------------
//// UART4-Interrupt
////--------------------------------------------------------------
//void UART4_IRQHandler(void) {
//  uint16_t wert;

//  if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET) {
//    // wenn ein Byte im Empfangspuffer steht
//    wert=USART_ReceiveData(UART4);
//    // Byte speichern
//    P_UART_RX_INT(UART4_IRQn,wert);
//  }
//}

////--------------------------------------------------------------
//// UART5-Interrupt
////--------------------------------------------------------------
//void UART5_IRQHandler(void) {
//  uint16_t wert;

//  if (USART_GetITStatus(UART5, USART_IT_RXNE) == SET) {
//    // wenn ein Byte im Empfangspuffer steht
//    wert=USART_ReceiveData(UART5);
//    // Byte speichern
//    P_UART_RX_INT(UART5_IRQn,wert);
//  }
//}

////--------------------------------------------------------------
//// UART6-Interrupt
////--------------------------------------------------------------
//void USART6_IRQHandler(void) {
//  uint16_t wert;

//  if (USART_GetITStatus(USART6, USART_IT_RXNE) == SET) {
//    // wenn ein Byte im Empfangspuffer steht
//    wert=USART_ReceiveData(USART6);
//    // Byte speichern
//    P_UART_RX_INT(USART6_IRQn,wert);
//  }
//}
