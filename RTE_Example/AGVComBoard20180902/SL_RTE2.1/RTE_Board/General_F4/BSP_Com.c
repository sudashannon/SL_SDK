#include "BSP_Com.h"
static BSP_COM_Handle_t ComControlHandle[COM_N]=
{
  {
		.ComName = COM_1,
		.USARTx = USART1,
		.ComPort = GPIOA, 
		.TxPinsource = GPIO_PinSource9, 
		.RxPinsource = GPIO_PinSource10, 
		.ComAF = GPIO_AF_USART1, 
		.ComPintx = GPIO_Pin_9, 
		.ComPinrx = GPIO_Pin_10, 
		.GpioClk = RCC_AHB1Periph_GPIOA,
		.DMAStreamx = DMA2_Stream2,
		.DMARxChannel = DMA_Channel_4,
		.DmaClk = RCC_AHB1Periph_DMA2,
		.DataBufferLen = 32,
		.DataQueneLen = 64,
	}, 
};
static void BSP_COM_HardInit(BSP_COM_NAME_e com_name,uint32_t baudrate)
{ 	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	if(ComControlHandle[com_name].USARTx==USART1)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	else if(ComControlHandle[com_name].USARTx==USART2)
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	else if(ComControlHandle[com_name].USARTx==USART3)
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	else if(ComControlHandle[com_name].USARTx==UART4)
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
	RCC_AHB1PeriphClockCmd(ComControlHandle[com_name].GpioClk, ENABLE);
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(ComControlHandle[com_name].ComPort,ComControlHandle[com_name].TxPinsource,ComControlHandle[com_name].ComAF);
	GPIO_PinAFConfig(ComControlHandle[com_name].ComPort,ComControlHandle[com_name].RxPinsource,ComControlHandle[com_name].ComAF);
	USART_DeInit(ComControlHandle[com_name].USARTx); 
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	// TX-Pin
	GPIO_InitStructure.GPIO_Pin = ComControlHandle[com_name].ComPintx; 
	GPIO_Init(ComControlHandle[com_name].ComPort, &GPIO_InitStructure); 
	// RX-Pin
	GPIO_InitStructure.GPIO_Pin = ComControlHandle[com_name].ComPinrx;
	GPIO_Init(ComControlHandle[com_name].ComPort, &GPIO_InitStructure); 
	// Oversampling
	USART_OverSampling8Cmd(ComControlHandle[com_name].USARTx, ENABLE);

	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(ComControlHandle[com_name].USARTx, &USART_InitStructure);  
	USART_Cmd(ComControlHandle[com_name].USARTx, ENABLE);	
	USART_ClearFlag(ComControlHandle[com_name].USARTx, USART_FLAG_TC);
}
static void BSP_COM_InterruptInit(BSP_COM_NAME_e com_name)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ITConfig(ComControlHandle[com_name].USARTx, USART_IT_IDLE, ENABLE);
	if (ComControlHandle[com_name].USARTx == USART1) {
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	
	}
	else if(ComControlHandle[com_name].USARTx == USART2) {
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	
	}	
	else if(ComControlHandle[com_name].USARTx == USART3) {
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	
	}	
	else if(ComControlHandle[com_name].USARTx == UART4) {
		NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
	}	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	
}
void BSP_COM_Puts(BSP_COM_NAME_e com_name, char* str) {

	/* Go through entire string */
	while (*str) {
		/* Wait to be ready, buffer empty */
		USART_WAIT(ComControlHandle[com_name].USARTx);
		/* Send data */
		USART_WRITE_DATA(ComControlHandle[com_name].USARTx, (uint16_t)(*str++));
		/* Wait to be ready, buffer empty */
		USART_WAIT(ComControlHandle[com_name].USARTx);
	}
}
void BSP_COM_Send(BSP_COM_NAME_e com_name, uint8_t* Data, uint32_t count) {
	/* Go through entire data  */
	while (count--) {
		/* Wait to be ready, buffer empty */
		USART_WAIT(ComControlHandle[com_name].USARTx);
		/* Send data */
		USART_WRITE_DATA(ComControlHandle[com_name].USARTx, (uint16_t)(*Data++));
		/* Wait to be ready, buffer empty */
		USART_WAIT(ComControlHandle[com_name].USARTx);
	}
}
void BSP_COM_Putc(BSP_COM_NAME_e com_name, volatile char c) {
	/* Check USART */
	if ((ComControlHandle[com_name].USARTx->CR1 & USART_CR1_UE)) {	
		/* Wait to be ready, buffer empty */
		USART_WAIT(ComControlHandle[com_name].USARTx);
		/* Send data */
		USART_WRITE_DATA(ComControlHandle[com_name].USARTx, (uint16_t)(c & 0x01FF));
		/* Wait to be ready, buffer empty */
		USART_WAIT(ComControlHandle[com_name].USARTx);
	}
}
static DMA_InitTypeDef  DMA_InitStructure;
static void BSP_COM_DMAInit(BSP_COM_NAME_e com_name)
{
  /* Enable the DMA clock */
  RCC_AHB1PeriphClockCmd(ComControlHandle[com_name].DmaClk, ENABLE);
	DMA_DeInit(ComControlHandle[com_name].DMAStreamx);
  DMA_InitStructure.DMA_BufferSize = ComControlHandle[com_name].DataBufferLen ;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t)&ComControlHandle[com_name].USARTx->DR;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  /* Configure RX DMA */
  DMA_InitStructure.DMA_Channel = ComControlHandle[com_name].DMARxChannel ;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
  DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)ComControlHandle[com_name].ComBuffer.pu8Databuf;
	DMA_Init(ComControlHandle[com_name].DMAStreamx,&DMA_InitStructure);
  // enable DMA-RX request
  USART_DMACmd(ComControlHandle[com_name].USARTx, USART_DMAReq_Rx, ENABLE);
  DMA_Cmd(ComControlHandle[com_name].DMAStreamx,ENABLE);
}
static void BSP_COM_DMARecReset(BSP_COM_NAME_e com_name)
{
	DMA_DeInit(ComControlHandle[com_name].DMAStreamx);
	DMA_Init(ComControlHandle[com_name].DMAStreamx,&DMA_InitStructure);
	DMA_Cmd(ComControlHandle[com_name].DMAStreamx,ENABLE);
}
void BSP_COM_Init(BSP_COM_NAME_e com_name,uint32_t baudrate) {
	//申请内存
	ComControlHandle[com_name].ComBuffer.pu8Databuf = (uint8_t *)RTE_BGetz(MEM_RTE,ComControlHandle[com_name].DataBufferLen);
	ComControlHandle[com_name].ComBuffer.u16Datalength = 0;
	RTE_MessageQuene_Init(&ComControlHandle[com_name].ComBuffer.ComQuene,ComControlHandle[com_name].DataQueneLen);
	BSP_COM_HardInit(com_name,baudrate);
	BSP_COM_InterruptInit(com_name);
	BSP_COM_DMAInit(com_name);
	USART_Cmd(ComControlHandle[com_name].USARTx, ENABLE);	
	printf("*********************************\n");
	printf("---------------------------------\n");
	printf("---- Welcome to use SL-RTE ! ----\n");
	printf("----- Type \"Help\" for help. -----\n");
	printf("------- Version:%s --------\n",RTE_VERSION);
	printf("---------------------------------\n");
	printf("*********************************\n");
}
BSP_COM_Data_t *BSP_COM_ReturnQue(BSP_COM_NAME_e com_name)
{
	return &ComControlHandle[com_name].ComBuffer;
}
static void BSP_COM_RecCallback(BSP_COM_NAME_e com_name)
{
	ComControlHandle[com_name].USARTx->SR;
	ComControlHandle[com_name].USARTx->DR;
	ComControlHandle[com_name].ComBuffer.u16Datalength = ComControlHandle[com_name].DataBufferLen - DMA_GetCurrDataCounter(ComControlHandle[com_name].DMAStreamx);            //可以接收的总长度减去剩余长度，得到接收到数据的长度
	if(ComControlHandle[com_name].ComBuffer.u16Datalength)
		RTE_MessageQuene_In(&ComControlHandle[com_name].ComBuffer.ComQuene,ComControlHandle[com_name].ComBuffer.pu8Databuf,
				ComControlHandle[com_name].ComBuffer.u16Datalength);
	memset(ComControlHandle[com_name].ComBuffer.pu8Databuf,0,ComControlHandle[com_name].ComBuffer.u16Datalength);
	ComControlHandle[com_name].ComBuffer.u16Datalength = 0;
	BSP_COM_DMARecReset(com_name);
	USART_ClearITPendingBit(ComControlHandle[com_name].USARTx, USART_IT_IDLE);    //清除串口空闲
}
void USART1_IRQHandler(void) {
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		BSP_COM_RecCallback(COM_1);
	}
}
void USART3_IRQHandler(void) {
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		BSP_COM_RecCallback(COM_3);
	}
}
