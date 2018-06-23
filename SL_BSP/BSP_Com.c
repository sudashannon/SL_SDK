#include "BSP_Com.h"
static BSP_USART_Handle_t BSP_USARTControl[COM_N]=
{
#ifdef STM32F4
  // Name  ,                           
  {COM_1 ,USART1 ,GPIOA, GPIO_AF_USART1 ,GPIO_Pin_9 , GPIO_Pin_10 ,RCC_AHB1Periph_GPIOA,
	USART_WordLength_8b,USART_StopBits_1,USART_Parity_No,USART_Mode_Rx | USART_Mode_Tx,USART_HardwareFlowControl_None,
	128}, 
  // Name  , 	
  {COM_2 ,USART2 ,GPIOA, GPIO_AF_USART2 ,GPIO_Pin_2 , GPIO_Pin_3 ,RCC_AHB1Periph_GPIOA,
	USART_WordLength_8b,USART_StopBits_1,USART_Parity_No,USART_Mode_Rx | USART_Mode_Tx,USART_HardwareFlowControl_None,
	128}, 
  // Name  , 	
  {COM_3 ,USART3 ,GPIOB, GPIO_AF_USART3 ,GPIO_Pin_10 , GPIO_Pin_11 ,RCC_AHB1Periph_GPIOB,
	USART_WordLength_8b,USART_StopBits_1,USART_Parity_No,USART_Mode_Rx | USART_Mode_Tx,USART_HardwareFlowControl_None,
	2048}, 
  // Name  , 	
  {COM_4 ,UART4 ,GPIOA, GPIO_AF_UART4 ,GPIO_Pin_0 , GPIO_Pin_1 ,RCC_AHB1Periph_GPIOA,
	USART_WordLength_8b,USART_StopBits_1,USART_Parity_No,USART_Mode_Rx | USART_Mode_Tx,USART_HardwareFlowControl_None,
	128}, 
#endif
};
static uint16_t BSP_USART_GetPinSource(uint16_t GpioPin)
{
	switch(GpioPin)
	{
		case GPIO_Pin_0:
			return GPIO_PinSource0;
		case GPIO_Pin_1:
			return GPIO_PinSource1;
		case GPIO_Pin_2:
			return GPIO_PinSource2;
		case GPIO_Pin_3:
			return GPIO_PinSource3;
		case GPIO_Pin_4:
			return GPIO_PinSource4;
		case GPIO_Pin_5:
			return GPIO_PinSource5;
		case GPIO_Pin_6:
			return GPIO_PinSource6;
		case GPIO_Pin_7:
			return GPIO_PinSource7;
		case GPIO_Pin_8:
			return GPIO_PinSource8;
		case GPIO_Pin_9:
			return GPIO_PinSource9;
		case GPIO_Pin_10:
			return GPIO_PinSource10;
		case GPIO_Pin_11:
			return GPIO_PinSource11;
		case GPIO_Pin_12:
			return GPIO_PinSource12;
		case GPIO_Pin_13:
			return GPIO_PinSource13;
		case GPIO_Pin_14:
			return GPIO_PinSource14;
		case GPIO_Pin_15:
			return GPIO_PinSource15;
	}
	return 0xFF;
}
static void BSP_USART_HardInit(BSP_USART_NAME_e usart_name,uint32_t baudrate)
{ 	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	if(BSP_USARTControl[usart_name].USARTx==USART1)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	else if(BSP_USARTControl[usart_name].USARTx==USART2)
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	else if(BSP_USARTControl[usart_name].USARTx==USART3)
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	else if(BSP_USARTControl[usart_name].USARTx==UART4)
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
#ifdef STM32F1
	RCC_APB2PeriphClockCmd(BSP_USARTControl[usart_name].GpioClk, ENABLE);
#endif
#ifdef STM32F4
	RCC_AHB1PeriphClockCmd(BSP_USARTControl[usart_name].GpioClk, ENABLE);
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(BSP_USARTControl[usart_name].ComPort,BSP_USART_GetPinSource(BSP_USARTControl[usart_name].ComPinrx),BSP_USARTControl[usart_name].ComAF);  //GPIOA9复用为USART1
	GPIO_PinAFConfig(BSP_USARTControl[usart_name].ComPort,BSP_USART_GetPinSource(BSP_USARTControl[usart_name].ComPintx),BSP_USARTControl[usart_name].ComAF); //GPIOA10复用为USART1
#endif
	USART_DeInit(BSP_USARTControl[usart_name].USARTx); 
	
#ifdef STM32F1
	GPIO_InitStructure.GPIO_Pin = BSP_USARTControl[usart_name].ComPintx; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(BSP_USARTControl[usart_name].ComPort, &GPIO_InitStructure); 
	GPIO_InitStructure.GPIO_Pin = BSP_USARTControl[usart_name].ComPinrx;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(BSP_USARTControl[usart_name].ComPort, &GPIO_InitStructure); 
#endif
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	// TX-Pin
	GPIO_InitStructure.GPIO_Pin = BSP_USARTControl[usart_name].ComPintx; 
	GPIO_Init(BSP_USARTControl[usart_name].ComPort, &GPIO_InitStructure); 
	// RX-Pin
	GPIO_InitStructure.GPIO_Pin = BSP_USARTControl[usart_name].ComPinrx;
	GPIO_Init(BSP_USARTControl[usart_name].ComPort, &GPIO_InitStructure); 
	// Oversampling
	USART_OverSampling8Cmd(BSP_USARTControl[usart_name].USARTx, ENABLE);
#ifdef STM32F4

#endif
	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = BSP_USARTControl[usart_name].WordLength;
	USART_InitStructure.USART_StopBits = BSP_USARTControl[usart_name].StopBits;
	USART_InitStructure.USART_Parity = BSP_USARTControl[usart_name].Parity;
	USART_InitStructure.USART_HardwareFlowControl = BSP_USARTControl[usart_name].HardwareFlowControl;
	USART_InitStructure.USART_Mode = BSP_USARTControl[usart_name].Mode;
	USART_Init(BSP_USARTControl[usart_name].USARTx, &USART_InitStructure);  
	USART_Cmd(BSP_USARTControl[usart_name].USARTx, ENABLE);	
	USART_ClearFlag(BSP_USARTControl[usart_name].USARTx, USART_FLAG_TC);
}
static void BSP_USART_InterruptInit(BSP_USART_NAME_e usart_name)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ITConfig(BSP_USARTControl[usart_name].USARTx, USART_IT_RXNE, ENABLE);
	if (BSP_USARTControl[usart_name].USARTx == USART1) {
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = COM1_NVIC_PRIORITY;	
	}
	else if(BSP_USARTControl[usart_name].USARTx == USART2) {
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = COM2_NVIC_PRIORITY;	
	}	
	else if(BSP_USARTControl[usart_name].USARTx == USART3) {
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = COM3_NVIC_PRIORITY;	
	}	
	else if(BSP_USARTControl[usart_name].USARTx == UART4) {
		NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = COM4_NVIC_PRIORITY;	
	}	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	
}
void BSP_USART_Init(BSP_USART_NAME_e usart_name,uint32_t baudrate) {
	BSP_USARTControl[usart_name].BSP_USARTBuffer.pu8Databuf = bgetz(BSP_USARTControl[usart_name].BufferSize);
	BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength = 0;
	APP_MessageQuene_Init(&BSP_USARTControl[usart_name].BSP_USARTBuffer.ComQuene,COM_QUENE_SIZE * BSP_USARTControl[usart_name].BufferSize);
	BSP_USART_HardInit(usart_name,baudrate);
	BSP_USART_InterruptInit(usart_name);
}
BSP_USART_Data_t *BSP_USART_ReturnQue(BSP_USART_NAME_e usart_name)
{
	return &BSP_USARTControl[usart_name].BSP_USARTBuffer;
}
void BSP_USART_Puts(BSP_USART_NAME_e usart_name, char* str) {

	/* Go through entire string */
	while (*str) {
		/* Wait to be ready, buffer empty */
		USART_WAIT(BSP_USARTControl[usart_name].USARTx);
		/* Send data */
		USART_WRITE_DATA(BSP_USARTControl[usart_name].USARTx, (uint16_t)(*str++));
		/* Wait to be ready, buffer empty */
		USART_WAIT(BSP_USARTControl[usart_name].USARTx);
	}
}
void BSP_USART_Send(BSP_USART_NAME_e usart_name, uint8_t* Data, uint32_t count) {
	/* Go through entire data  */
	while (count--) {
		/* Wait to be ready, buffer empty */
		USART_WAIT(BSP_USARTControl[usart_name].USARTx);
		/* Send data */
		USART_WRITE_DATA(BSP_USARTControl[usart_name].USARTx, (uint16_t)(*Data++));
		/* Wait to be ready, buffer empty */
		USART_WAIT(BSP_USARTControl[usart_name].USARTx);
	}
}
void BSP_USART_Putc(BSP_USART_NAME_e usart_name, volatile char c) {
	/* Check USART */
	if ((BSP_USARTControl[usart_name].USARTx->CR1 & USART_CR1_UE)) {	
		/* Wait to be ready, buffer empty */
		USART_WAIT(BSP_USARTControl[usart_name].USARTx);
		/* Send data */
		USART_WRITE_DATA(BSP_USARTControl[usart_name].USARTx, (uint16_t)(c & 0x01FF));
		/* Wait to be ready, buffer empty */
		USART_WAIT(BSP_USARTControl[usart_name].USARTx);
	}
}
static BSP_USART_NAME_e usart_timerfucid;
static void TimerUsartRecFunction(void* arg)
{
	BSP_USART_NAME_e* usart_name=(BSP_USART_NAME_e *)arg;
	APP_MessageQuene_In(&BSP_USARTControl[*usart_name].BSP_USARTBuffer.ComQuene,BSP_USARTControl[*usart_name].BSP_USARTBuffer.pu8Databuf,
					BSP_USARTControl[*usart_name].BSP_USARTBuffer.u16Datalength);
	memset(BSP_USARTControl[*usart_name].BSP_USARTBuffer.pu8Databuf,0,BSP_USARTControl[*usart_name].BSP_USARTBuffer.u16Datalength);
	BSP_USARTControl[*usart_name].BSP_USARTBuffer.u16Datalength=0;
}
static void BSP_USART_RecCallback(BSP_USART_NAME_e usart_name)
{
	usart_timerfucid=usart_name;
	BSP_USARTControl[usart_name].BSP_USARTBuffer.pu8Databuf[BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength]=USART_ReceiveData(BSP_USARTControl[usart_name].USARTx);
	if(BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength < BSP_USARTControl[usart_name].BufferSize-3)
	{
		BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength++;
		APP_SoftTimer_Create(TIMER_COM,10,0,1,TimerUsartRecFunction,(void *)&usart_timerfucid);	
	}
	else
	{
		APP_SoftTimer_Delete(TIMER_COM);
		BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength++;
		BSP_USARTControl[usart_name].BSP_USARTBuffer.pu8Databuf[BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength] = 0x55;
		BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength++;
		BSP_USARTControl[usart_name].BSP_USARTBuffer.pu8Databuf[BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength] = 0xAA;
		BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength++;
		APP_MessageQuene_In(&BSP_USARTControl[usart_name].BSP_USARTBuffer.ComQuene,BSP_USARTControl[usart_name].BSP_USARTBuffer.pu8Databuf,
						BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength);
		memset(BSP_USARTControl[usart_name].BSP_USARTBuffer.pu8Databuf,0,BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength);
		BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength=0;
	}

}
void USART1_IRQHandler(void) {
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		BSP_USART_RecCallback(COM_1);
	}
}
void USART2_IRQHandler(void) {
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		BSP_USART_RecCallback(COM_2);
	}
}
void USART3_IRQHandler(void) {
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		BSP_USART_RecCallback(COM_3);
	}
}
void UART4_IRQHandler(void) {
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	{
		BSP_USART_RecCallback(COM_4);
	}
}

