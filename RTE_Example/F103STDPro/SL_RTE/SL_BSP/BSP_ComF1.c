#include "APP_SoftTimer.h"
#include "BSP_ComF1.h"
static BSP_USART_Handle_t BSP_USARTControl[COM_N]=
{
  // Name  ,                           
  {COM_1 ,USART1 ,GPIOA, GPIO_Pin_9 , GPIO_Pin_10 ,RCC_APB2Periph_GPIOA,
	USART_WordLength_8b,USART_StopBits_1,USART_Parity_No,USART_Mode_Rx | USART_Mode_Tx,USART_HardwareFlowControl_None,
	DMA1_Channel5,DMA1_Channel4,32,64}, 
  // Name  , 	
  {COM_2 ,USART2 ,GPIOA, GPIO_Pin_2 , GPIO_Pin_3 ,RCC_APB2Periph_GPIOA,
	USART_WordLength_8b,USART_StopBits_1,USART_Parity_No,USART_Mode_Rx | USART_Mode_Tx,USART_HardwareFlowControl_None,
	DMA1_Channel6,DMA1_Channel7,128,512},  
};
static void BSP_USART_HardInit(BSP_USART_NAME_e usart_name,uint32_t baudrate)
{ 	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	if(BSP_USARTControl[usart_name].USARTx==USART1)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	else if(BSP_USARTControl[usart_name].USARTx==USART2)
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	RCC_APB2PeriphClockCmd(BSP_USARTControl[usart_name].GpioClk, ENABLE);
	USART_DeInit(BSP_USARTControl[usart_name].USARTx); 
	GPIO_InitStructure.GPIO_Pin = BSP_USARTControl[usart_name].ComPintx; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(BSP_USARTControl[usart_name].ComPort, &GPIO_InitStructure); 
	GPIO_InitStructure.GPIO_Pin = BSP_USARTControl[usart_name].ComPinrx;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(BSP_USARTControl[usart_name].ComPort, &GPIO_InitStructure); 

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
	USART_ITConfig(BSP_USARTControl[usart_name].USARTx, USART_IT_IDLE, ENABLE);
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
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	
}
void BSP_DMA_Init(BSP_USART_NAME_e usart_name)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输时钟
	DMA_DeInit(BSP_USARTControl[usart_name].DMARXChannel);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&BSP_USARTControl[usart_name].USARTx->DR;    //DMA外设基地址 串口 具体参考手册
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)BSP_USARTControl[usart_name].BSP_USARTBuffer.pu8Databuf;   //DMA内存基地址 存放DMA数据 接收
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;        //数据传输方向，从外设到内存（接收数据）
	DMA_InitStructure.DMA_BufferSize = BSP_USARTControl[usart_name].DataBufferLen;              //接收DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;          //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;   //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;   //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;           //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;             //正常模式，即满了就不在接收了，而不是循环存储
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;   //同DMA优先级很高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;              //DMA通道内存与外设传输
	DMA_Init(BSP_USARTControl[usart_name].DMARXChannel, &DMA_InitStructure);              //初始化DMA
	DMA_Cmd(BSP_USARTControl[usart_name].DMARXChannel, ENABLE);                           //启动DMA
  USART_DMACmd(BSP_USARTControl[usart_name].USARTx, USART_DMAReq_Rx, ENABLE);// 使能DMA串口发送和接受请求
} 
void BSP_USART_DMARecReset(BSP_USART_NAME_e usart_name)
{
	DMA_Cmd(BSP_USARTControl[usart_name].DMARXChannel, DISABLE);
	BSP_USARTControl[usart_name].DMARXChannel->CNDTR = BSP_USARTControl[usart_name].DataBufferLen ;
	DMA_Cmd(BSP_USARTControl[usart_name].DMARXChannel, ENABLE);
}
void BSP_USART_Init(BSP_USART_NAME_e usart_name,uint32_t baudrate) {
	//申请内存
	BSP_USARTControl[usart_name].BSP_USARTBuffer.pu8Databuf = APP_BGetz(MEM_0,BSP_USARTControl[usart_name].DataBufferLen);
	BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength = 0;
	APP_MessageQuene_Init(&BSP_USARTControl[usart_name].BSP_USARTBuffer.ComQuene,BSP_USARTControl[usart_name].DataQueneLen);
	BSP_USART_HardInit(usart_name,baudrate);
	BSP_USART_InterruptInit(usart_name);
	BSP_DMA_Init(usart_name);
}
BSP_USART_Data_t *BSP_USART_ReturnQue(BSP_USART_NAME_e usart_name)
{
	return &BSP_USARTControl[usart_name].BSP_USARTBuffer;
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
static void BSP_USART_RecCallback(BSP_USART_NAME_e usart_name)
{
	BSP_USARTControl[usart_name].USARTx->SR;
	BSP_USARTControl[usart_name].USARTx->DR;
	BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength = BSP_USARTControl[usart_name].DataBufferLen - BSP_USARTControl[usart_name].DMARXChannel->CNDTR;            //可以接收的总长度减去剩余长度，得到接收到数据的长度
	APP_MessageQuene_In(&BSP_USARTControl[usart_name].BSP_USARTBuffer.ComQuene,BSP_USARTControl[usart_name].BSP_USARTBuffer.pu8Databuf,
					BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength);
	memset(BSP_USARTControl[usart_name].BSP_USARTBuffer.pu8Databuf,0,BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength);
	BSP_USARTControl[usart_name].BSP_USARTBuffer.u16Datalength = 0;
	BSP_USART_DMARecReset(usart_name);
	USART_ClearITPendingBit(BSP_USARTControl[usart_name].USARTx, USART_IT_IDLE);    //清除串口空闲
}
void USART1_IRQHandler(void) {
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		BSP_USART_RecCallback(COM_1);
	}
}
void USART2_IRQHandler(void) {
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		BSP_USART_RecCallback(COM_2);
	}
}




