#include "APP_Include.h"
#include "stm32L4xx.h"
//进入待机模式	  
void APP_Standby(void)
{ 	
	__WFI();			    //执行WFI指令,进入待机模式		 
}	     
//系统软复位   
void APP_SoftReset(void)
{   
	SCB->AIRCR =0X05FA0000|(uint32_t)0x04;	  
} 	
//开启所有中断
void APP_EnableInterrupts(void)
{
	/* Disable interrupts */
	__enable_irq();
}
void APP_DisableInterrupts(void)
{
	/* Disable interrupts */
	__disable_irq();
}
void APP_Assert(char* file, uint32_t line)
{ 
	printf("[ASSERT]Wrong parameters value: file %s on line %d\n", file, line);
	/* 这是一个死循环，断言失败时程序会在此处死机，以便于用户查错 */
	while (1)
	{
	}
}
/* Callback function for system */
static void SysTimer_Callback(void* UserParameters) {
#if USE_DEBUG == 1
	APP_Debug_Poll();
#endif
}
void APP_RunEnvInit(void)
{
#if USE_MPU == 1
	MPU_Memory_Protection();
#endif
	APP_SoftTimer_Init();
	APP_SoftTimer_Create(TIMER_SYS,10, 1, 1, SysTimer_Callback, (void *)0);
	APP_BGet_Init();
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4); 
	HAL_Init();
}
#if USE_COM_AS_STDIN_OUT == 1
/**
  * @brief Retargets the C library msg_info function to the USART.
  * @param None
  * @retval None
  */
extern UART_HandleTypeDef hDiscoUart;
int stdout_putchar (int ch)
{
	unsigned int Enter = '\r';
  /* Place your implementation of fputc here */
  /* e.g. write a character to the serial port and Loop until the end of transmission */
	if(ch=='\n')
  while (HAL_OK != HAL_UART_Transmit(&hDiscoUart, (uint8_t *) &Enter, 1, 30000))
  {
    ;
  }
  while (HAL_OK != HAL_UART_Transmit(&hDiscoUart, (uint8_t *) &ch, 1, 30000))
  {
    ;
  }
  return ch;
}

/**
  * @brief Retargets the C library scanf function to the USART.
  * @param None
  * @retval None
  */
int stdin_getchar (void)
{
  /* Place your implementation of fgetc here */
  /* e.g. readwrite a character to the USART2 and Loop until the end of transmission */
  uint8_t ch = 0;
  while (HAL_OK != HAL_UART_Receive(&hDiscoUart, (uint8_t *)&ch, 1, 30000))
  {
    ;
  }
  return ch;
}
#endif
