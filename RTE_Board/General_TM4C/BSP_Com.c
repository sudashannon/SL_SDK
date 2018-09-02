#include "BSP_Com.h"
static BSP_COM_Handle_t ComControlHandle[COM_N]=
{
  // Name  ,                                                            
  {
		.ComName = COM_0,
		.DataBufferLen = 32,
		.DataQueneLen = 64,
	}, 
  {
		.ComName = COM_2,
		.DataBufferLen = 32,
		.DataQueneLen = 64,
	}, 
};
static void TimerUsartRecFunction(void* arg);
static BSP_COM_NAME_e usart_timerfucid;
void BSP_COM_Init(BSP_COM_NAME_e com_name) {
	//申请内存
	ComControlHandle[com_name].ComBuffer.pu8Databuf = (uint8_t *)RTE_BGetz(MEM_RTE,ComControlHandle[com_name].DataBufferLen);
	ComControlHandle[com_name].ComBuffer.u16Datalength = 0;
	RTE_MessageQuene_Init(&ComControlHandle[com_name].ComBuffer.ComQuene,ComControlHandle[com_name].DataQueneLen);
	//创建定时器
	RTE_RoundRobin_CreateTimer("COMTimer",20,0,0,TimerUsartRecFunction,&usart_timerfucid);
	// Configure the uart.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	if(com_name==COM_0)
	{
		SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
		SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART0);
		GPIOPinConfigure(GPIO_PA0_U0RX);
		GPIOPinConfigure(GPIO_PA1_U0TX);
		GPIOPinTypeUART(GPIOA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
		//
		// Configure the UART for 115,200, 8-N-1 operation.
		//
		UARTConfigSetExpClk(UART0_BASE, SystemCoreClock, 921600,
														(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
														 UART_CONFIG_PAR_NONE));
		//
		// Enable the UART interrupt.
		//
		IntEnable(INT_UART0);
		UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT );
		NVIC_SetPriority(UART0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));
	}
  else if(com_name==COM_2)
	{
		SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
		SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART2);
		GPIOPinConfigure(GPIO_PA6_U2RX);
		GPIOPinConfigure(GPIO_PA7_U2TX);
		GPIOPinTypeUART(GPIOA_BASE, GPIO_PIN_6 | GPIO_PIN_7);
		//
		// Configure the UART for 115,200, 8-N-1 operation.
		//
		UARTConfigSetExpClk(UART2_BASE, SystemCoreClock, 115200,
														(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
														 UART_CONFIG_PAR_NONE));
		//
		// Enable the UART interrupt.
		//
		IntEnable(INT_UART2);
		UARTIntEnable(UART2_BASE, UART_INT_RX | UART_INT_RT );
		NVIC_SetPriority(UART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));
	}
	printf("*********************************\n");
	printf("---------------------------------\n");
	printf("---- Welcome to use SL-RTE ! ----\n");
	printf("----- Type \"Help\" for help. -----\n");
	printf("------- Version:%s --------\n",RTE_VERSION);
	printf("---------------------------------\n");
	printf("*********************************\n");
}
void BSP_COM_Send(BSP_COM_NAME_e com_name, uint8_t* DataArray, uint32_t count) {
	/* Go through entire data array */
	if(com_name==COM_0)
	{
		while(count--)
		{
			//
			// Write the next character to the UART.
			//
			UARTCharPutNonBlocking(UART0_BASE, *DataArray++);
		}
	}
	else if(com_name==COM_2)
	{
		while(count--)
		{
			//
			// Write the next character to the UART.
			//
			UARTCharPutNonBlocking(UART2_BASE, *DataArray++);
		}
	}
}
BSP_COM_Data_t* BSP_COM_ReturnQue(BSP_COM_NAME_e com_name)
{
	return &ComControlHandle[com_name].ComBuffer;
}
static void TimerUsartRecFunction(void* arg)
{
	BSP_COM_NAME_e* com_name=(BSP_COM_NAME_e *)arg;
	if(ComControlHandle[*com_name].ComBuffer.u16Datalength)
		RTE_MessageQuene_In(&ComControlHandle[*com_name].ComBuffer.ComQuene,ComControlHandle[*com_name].ComBuffer.pu8Databuf,
				ComControlHandle[*com_name].ComBuffer.u16Datalength);
	memset(ComControlHandle[*com_name].ComBuffer.pu8Databuf,0,ComControlHandle[*com_name].DataBufferLen);
	ComControlHandle[*com_name].ComBuffer.u16Datalength = 0;
}
static void BSP_COM_RecCallback(uint32_t g_ui32Base,BSP_COM_NAME_e com_name)
{
	usart_timerfucid=com_name;
	ComControlHandle[com_name].ComBuffer.pu8Databuf[ComControlHandle[com_name].ComBuffer.u16Datalength++]= UARTCharGetNonBlocking(g_ui32Base);
	RTE_RoundRobin_ResetTimer("COMTimer");
}
void UART0_Handler(void)
{
	uint32_t ui32Status;
	ui32Status = UARTIntStatus(UART0_BASE, true);
	UARTIntClear(UART0_BASE, ui32Status);
	while(UARTCharsAvail(UART0_BASE))
	{
		BSP_COM_RecCallback(UART0_BASE,COM_0);
	}

}
void UART2_Handler(void)
{
	uint32_t ui32Status;
	ui32Status = UARTIntStatus(UART2_BASE, true);
	UARTIntClear(UART2_BASE, ui32Status);
	while(UARTCharsAvail(UART2_BASE))
	{
		BSP_COM_RecCallback(UART2_BASE,COM_2);
	}
}


