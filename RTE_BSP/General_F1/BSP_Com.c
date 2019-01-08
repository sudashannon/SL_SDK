#include "BSP_Com.h"
static BSP_Com_Handle_t ComControlHandle[COM_N]=
{
  // Name  ,                           
  {COM_1 ,USART1 ,GPIOA, GPIO_Pin_9 , GPIO_Pin_10 ,RCC_APB2Periph_GPIOA,
	USART_WordLength_8b,USART_StopBits_1,USART_Parity_No,USART_Mode_Rx | USART_Mode_Tx,USART_HardwareFlowControl_None,
	DMA1_Channel4,DMA1_Channel5,16,128}, 
  // Name  , 	
  {COM_3 ,USART3 ,GPIOB, GPIO_Pin_10 , GPIO_Pin_11 ,RCC_APB2Periph_GPIOB,
	USART_WordLength_8b,USART_StopBits_1,USART_Parity_No,USART_Mode_Rx | USART_Mode_Tx,USART_HardwareFlowControl_None,
	DMA1_Channel2,DMA1_Channel3,16,128},  
};
static void BSP_Com_HardInit(BSP_Com_Name_e usart_name,uint32_t baudrate)
{ 	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	if(ComControlHandle[usart_name].USARTx==USART1)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	else if(ComControlHandle[usart_name].USARTx==USART2)
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	else if(ComControlHandle[usart_name].USARTx==USART3)
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	RCC_APB2PeriphClockCmd(ComControlHandle[usart_name].GpioClk, ENABLE);
	USART_DeInit(ComControlHandle[usart_name].USARTx); 
	GPIO_InitStructure.GPIO_Pin = ComControlHandle[usart_name].ComPintx; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(ComControlHandle[usart_name].ComPort, &GPIO_InitStructure); 
	GPIO_InitStructure.GPIO_Pin = ComControlHandle[usart_name].ComPinrx;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(ComControlHandle[usart_name].ComPort, &GPIO_InitStructure); 

	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = ComControlHandle[usart_name].WordLength;
	USART_InitStructure.USART_StopBits = ComControlHandle[usart_name].StopBits;
	USART_InitStructure.USART_Parity = ComControlHandle[usart_name].Parity;
	USART_InitStructure.USART_HardwareFlowControl = ComControlHandle[usart_name].HardwareFlowControl;
	USART_InitStructure.USART_Mode = ComControlHandle[usart_name].Mode;
	USART_Init(ComControlHandle[usart_name].USARTx, &USART_InitStructure);  
	USART_Cmd(ComControlHandle[usart_name].USARTx, ENABLE);	
	USART_ClearFlag(ComControlHandle[usart_name].USARTx, USART_FLAG_TC); 
}
static void BSP_Com_InterruptInit(BSP_Com_Name_e usart_name)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ITConfig(ComControlHandle[usart_name].USARTx, USART_IT_IDLE, ENABLE);
	if (ComControlHandle[usart_name].USARTx == USART1) {
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	
	}
	else if(ComControlHandle[usart_name].USARTx == USART2) {
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	
	}	
	else if(ComControlHandle[usart_name].USARTx == USART3) {
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
	}	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	
}
void BSP_DMA_Init(BSP_Com_Name_e usart_name)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输时钟
	DMA_DeInit(ComControlHandle[usart_name].DMARXChannel);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ComControlHandle[usart_name].USARTx->DR;    //DMA外设基地址 串口 具体参考手册
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ComControlHandle[usart_name].ComData.pu8Databuf;   //DMA内存基地址 存放DMA数据 接收
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;        //数据传输方向，从外设到内存（接收数据）
	DMA_InitStructure.DMA_BufferSize = ComControlHandle[usart_name].DataBufferLen;              //接收DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;          //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;   //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;   //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;           //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;             //正常模式，即满了就不在接收了，而不是循环存储
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;   //同DMA优先级很高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;              //DMA通道内存与外设传输
	DMA_Init(ComControlHandle[usart_name].DMARXChannel, &DMA_InitStructure);              //初始化DMA
	DMA_Cmd(ComControlHandle[usart_name].DMARXChannel, ENABLE);                           //启动DMA
	USART_DMACmd(ComControlHandle[usart_name].USARTx, USART_DMAReq_Rx, ENABLE);// 使能DMA串口发送和接受请求
} 
void BSP_Com_DMARecReset(BSP_Com_Name_e usart_name)
{
	DMA_Cmd(ComControlHandle[usart_name].DMARXChannel, DISABLE);
	ComControlHandle[usart_name].DMARXChannel->CNDTR = ComControlHandle[usart_name].DataBufferLen ;
	DMA_Cmd(ComControlHandle[usart_name].DMARXChannel, ENABLE);
}
void BSP_Com_Init(BSP_Com_Name_e usart_name,uint32_t baudrate) {
	//申请内存
	ComControlHandle[usart_name].ComData.pu8Databuf = RTE_MEM_Alloc0(MEM_RTE,ComControlHandle[usart_name].DataBufferLen);
	ComControlHandle[usart_name].ComData.u16Datalength = 0;
	RTE_MessageQuene_Init(&ComControlHandle[usart_name].ComData.ComQuene,ComControlHandle[usart_name].DataQueneLen);
	BSP_Com_HardInit(usart_name,baudrate);
	BSP_Com_InterruptInit(usart_name);
	BSP_DMA_Init(usart_name);
}
BSP_Com_Data_t *BSP_Com_ReturnQue(BSP_Com_Name_e usart_name)
{
	return &ComControlHandle[usart_name].ComData;
}
void BSP_Com_Putc(BSP_Com_Name_e usart_name, volatile char c) {
	/* Check USART */
	if ((ComControlHandle[usart_name].USARTx->CR1 & USART_CR1_UE)) {	
		/* Wait to be ready, buffer empty */
		USART_WAIT(ComControlHandle[usart_name].USARTx);
		/* Send data */
		USART_WRITE_DATA(ComControlHandle[usart_name].USARTx, (uint16_t)(c & 0x01FF));
		/* Wait to be ready, buffer empty */
		USART_WAIT(ComControlHandle[usart_name].USARTx);
	}
}
void BSP_Com_Puts(BSP_Com_Name_e usart_name, char* str) {
	/* Go through entire string */
	while (*str) {
		/* Wait to be ready, buffer empty */
		USART_WAIT(ComControlHandle[usart_name].USARTx);
		/* Send data */
		USART_WRITE_DATA(ComControlHandle[usart_name].USARTx, (uint16_t)(*str++));
		/* Wait to be ready, buffer empty */
		USART_WAIT(ComControlHandle[usart_name].USARTx);
	}
}
void BSP_Com_Send(BSP_Com_Name_e usart_name, uint8_t* Data, uint32_t count) {
	/* Go through entire data  */
	while (count--) {
		/* Wait to be ready, buffer empty */
		USART_WAIT(ComControlHandle[usart_name].USARTx);
		/* Send data */
		USART_WRITE_DATA(ComControlHandle[usart_name].USARTx, (uint16_t)(*Data++));
		/* Wait to be ready, buffer empty */
		USART_WAIT(ComControlHandle[usart_name].USARTx);
	}
}
static void BSP_Com_RecCallback(BSP_Com_Name_e usart_name)
{
	ComControlHandle[usart_name].USARTx->SR;
	ComControlHandle[usart_name].USARTx->DR;
	ComControlHandle[usart_name].ComData.u16Datalength = ComControlHandle[usart_name].DataBufferLen - ComControlHandle[usart_name].DMARXChannel->CNDTR;//可以接收的总长度减去剩余长度，得到接收到数据的长度
	if(ComControlHandle[usart_name].ComData.u16Datalength)
		RTE_MessageQuene_In(&ComControlHandle[usart_name].ComData.ComQuene,ComControlHandle[usart_name].ComData.pu8Databuf,
					ComControlHandle[usart_name].ComData.u16Datalength);
	memset(ComControlHandle[usart_name].ComData.pu8Databuf,0,ComControlHandle[usart_name].ComData.u16Datalength);
	ComControlHandle[usart_name].ComData.u16Datalength = 0;
	BSP_Com_DMARecReset(usart_name);
	USART_ClearITPendingBit(ComControlHandle[usart_name].USARTx, USART_IT_IDLE);    //清除串口空闲
}
void USART1_IRQHandler(void) 
{
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		BSP_Com_RecCallback(COM_1);
	}
}
void USART3_IRQHandler(void) 
{
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		BSP_Com_RecCallback(COM_3);
	}
}
