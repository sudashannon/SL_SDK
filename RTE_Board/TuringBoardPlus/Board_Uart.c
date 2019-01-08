#include "Board_Uart.h"
//NOTE: SCB_InvalidateDCache_by_Addr((uint32_t *)UartHandle[usart_name].UsartData.pu8Databuf, UartHandle[usart_name].UsartData.u16Datalength);
const Board_Uart_HardWareConfig_t UartDefaultConfig = 
{
	.BaudRate = 115200,
	.WordLength = UART_WORDLENGTH_8B,
	.StopBits = UART_STOPBITS_1,
	.Parity = UART_PARITY_NONE,
};
Board_Uart_Control_t UartHandle[USART_N] = 
{
	{
		.UsartName = USART_DEBUG,
		.Instance = USART1,
		.BufferSize = 32,
		.RingBufferSize = 64,
		.HardConfig = (Board_Uart_HardWareConfig_t *)&UartDefaultConfig,
	}
};
static Board_Uart_Name_e usart_timerfucid;
static void ComTimer_Callback(void* arg)
{
	Board_Uart_Name_e* usart_name=(Board_Uart_Name_e *)arg;
	HAL_UART_DMAStop(&UartHandle[*usart_name].UartHalHandle);
	SCB_InvalidateDCache_by_Addr((uint32_t *)UartHandle[*usart_name].UsartData.pu8Databuf, UartHandle[*usart_name].UsartData.u16Datalength);
	if(*usart_name == USART_DEBUG && UartHandle[*usart_name].UsartData.u16Datalength &&osKernelGetState () == osKernelRunning)
	{
		RTE_Shell_Input(UartHandle[*usart_name].UsartData.pu8Databuf,UartHandle[*usart_name].UsartData.u16Datalength);
	}
	else
		RTE_MessageQuene_In(&UartHandle[*usart_name].UsartData.ComQuene,UartHandle[*usart_name].UsartData.pu8Databuf,
					UartHandle[*usart_name].UsartData.u16Datalength);
	memset(UartHandle[*usart_name].UsartData.pu8Databuf,0,UartHandle[*usart_name].UsartData.u16Datalength);
	UartHandle[*usart_name].UsartData.u16Datalength = 0;
	HAL_UART_Receive_DMA(&UartHandle[*usart_name].UartHalHandle, (uint8_t *)(UartHandle[*usart_name].UsartData.pu8Databuf), 1); 
	HAL_UART_DMAResume(&UartHandle[*usart_name].UartHalHandle);
}
void Board_Uart_Init(Board_Uart_Name_e usartname,void (*MSPInitCallback)(void),void (*MSPDeInitCallback)(void))
{
	UartHandle[usartname].UsartData.pu8Databuf = RTE_MEM_Alloc0(MEM_RTE,UartHandle[usartname].BufferSize);
	RTE_AssertParam(UartHandle[usartname].UsartData.pu8Databuf);
	UartHandle[usartname].UsartData.u16Datalength = 0;
	RTE_MessageQuene_Init(&UartHandle[usartname].UsartData.ComQuene,UartHandle[usartname].RingBufferSize);
	
	UartHandle[usartname].MSPInitCallback = MSPInitCallback;
	UartHandle[usartname].MSPDeInitCallback = MSPDeInitCallback;
  UartHandle[usartname].UartHalHandle.Instance = UartHandle[usartname].Instance;
  UartHandle[usartname].UartHalHandle.Init.BaudRate = UartHandle[usartname].HardConfig->BaudRate;
  UartHandle[usartname].UartHalHandle.Init.WordLength = UartHandle[usartname].HardConfig->WordLength;
  UartHandle[usartname].UartHalHandle.Init.StopBits = UartHandle[usartname].HardConfig->StopBits;
  UartHandle[usartname].UartHalHandle.Init.Parity = UartHandle[usartname].HardConfig->Parity;
  UartHandle[usartname].UartHalHandle.Init.Mode = UART_MODE_TX_RX;
  UartHandle[usartname].UartHalHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  UartHandle[usartname].UartHalHandle.Init.OverSampling = UART_OVERSAMPLING_16;
  UartHandle[usartname].UartHalHandle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  UartHandle[usartname].UartHalHandle.Init.Prescaler = UART_PRESCALER_DIV1;
  UartHandle[usartname].UartHalHandle.Init.FIFOMode = UART_FIFOMODE_DISABLE;
  UartHandle[usartname].UartHalHandle.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
  UartHandle[usartname].UartHalHandle.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
  UartHandle[usartname].UartHalHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&UartHandle[usartname].UartHalHandle) != HAL_OK)
  {
		RTE_Assert(__FILE__, __LINE__);
  }
	RTE_RoundRobin_CreateTimer(0,"ComTimer",10,0,0,ComTimer_Callback,(void *)&usart_timerfucid);
	RTE_AssertParam(HAL_UART_Receive_DMA(&UartHandle[usartname].UartHalHandle, (uint8_t *)(UartHandle[usartname].UsartData.pu8Databuf), 1) == HAL_OK);
}
Board_Uart_Data_t *Board_Uart_ReturnQue(Board_Uart_Name_e usart_name)
{
	return &UartHandle[usart_name].UsartData;
}
void Board_Uart_RecCallback(Board_Uart_Name_e usart_name)
{
	usart_timerfucid=usart_name;
	if(UartHandle[usart_name].UsartData.u16Datalength < UartHandle[usart_name].BufferSize)
	{
		UartHandle[usart_name].UsartData.u16Datalength++;
		RTE_RoundRobin_ResetTimer(0,RTE_RoundRobin_GetTimerID(0,"ComTimer"));
		HAL_UART_Receive_DMA(&UartHandle[usart_name].UartHalHandle, (uint8_t *)(UartHandle[usart_name].UsartData.pu8Databuf+UartHandle[usart_name].UsartData.u16Datalength), 1); 
	}
	else
	{
		RTE_RoundRobin_ResetTimer(0,RTE_RoundRobin_GetTimerID(0,"ComTimer"));
		UartHandle[usart_name].UsartData.u16Datalength++;
		UartHandle[usart_name].UsartData.pu8Databuf[UartHandle[usart_name].UsartData.u16Datalength] = 0x55;
		UartHandle[usart_name].UsartData.u16Datalength++;
		UartHandle[usart_name].UsartData.pu8Databuf[UartHandle[usart_name].UsartData.u16Datalength] = 0xAA;
		UartHandle[usart_name].UsartData.u16Datalength++;
		HAL_UART_DMAStop(&UartHandle[usart_name].UartHalHandle);
		RTE_MessageQuene_In(&UartHandle[usart_name].UsartData.ComQuene,UartHandle[usart_name].UsartData.pu8Databuf,
						UartHandle[usart_name].UsartData.u16Datalength);
		memset(UartHandle[usart_name].UsartData.pu8Databuf,0,UartHandle[usart_name].UsartData.u16Datalength);
		UartHandle[usart_name].UsartData.u16Datalength = 0;
		HAL_UART_Receive_DMA(&UartHandle[usart_name].UartHalHandle, (uint8_t *)(UartHandle[usart_name].UsartData.pu8Databuf), 1); 
		HAL_UART_DMAResume(&UartHandle[usart_name].UartHalHandle);
	}
}
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	for(Board_Uart_Name_e i = (Board_Uart_Name_e)0;i<USART_N;i++)
	{
		if(huart->Instance == UartHandle[i].Instance)
		{
			UartHandle[i].MSPInitCallback();
			break;
		}
	}
}
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
	for(Board_Uart_Name_e i = (Board_Uart_Name_e)0;i<USART_N;i++)
	{
		if(huart->Instance == UartHandle[i].Instance)
		{
			UartHandle[i].MSPDeInitCallback();
			break;
		}
	}
}





