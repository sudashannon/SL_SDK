#include "APP_Include.h" 
#include "BSP_Com.h"
#if USE_RTOSV1_API == 1
#include "..\SL_Lib3.0\ThirdParty\RTX_API\cmsis_os1.c"
#endif
int fputc(int ch, FILE *f)
{
	if(ch=='\n')
		BSP_USART_Putc(COM_1,'\r');
  BSP_USART_Putc(COM_1,ch);
  return ch;
}
void APP_Standby(void)
{ 	
	__WFI();
}	      
void APP_SoftReset(void)
{   
	SCB->AIRCR =0X05FA0000|(uint32_t)0x04;	  
} 	
void APP_InteruptDisnable(void)
{
	__disable_irq();
}
void APP_InteruptEnable(void)
{
	__enable_irq();
}
void APP_Assert(char *file, uint32_t line)
{
	APP_Printf("[ASSERT]Wrong parameters value: file %s on line %d\n", (char *)file, line);
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
#if USE_KEY == 1
	BSP_KEY_Poll();
#endif
}
void APP_RunEnvInit(void)
{
#if USE_MPU == 1
	MPU_Memory_Protection();
#endif
	APP_MemInit();
	APP_SoftTimer_Init();
	APP_SoftTimer_Create(TIMER_SYS,10, 1, 0, SysTimer_Callback, (void *)0);
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4); 
#if USE_DEBUG == 1
	APP_Debug_Init();
#endif
#if USE_KEY == 1
	BSP_KEY_Init();
#endif
#if USL_HALLIB
	HAL_Init();
#endif
}
