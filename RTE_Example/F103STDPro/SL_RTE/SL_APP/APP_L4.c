#include "APP_L4.h"
#include "stm32L4xx.h"
//进入待机模式	  
void APP_SYS_Standby(void)
{ 	
	__WFI();			    //执行WFI指令,进入待机模式		 
}	     
//系统软复位   
void APP_SYS_SoftReset(void)
{   
	SCB->AIRCR =0X05FA0000|(uint32_t)0x04;	  
} 	
//开启所有中断
void APP_SYS_EnableInterrupts(void)
{
	/* Disable interrupts */
	__enable_irq();
}
void APP_SYS_DisableInterrupts(void)
{
	/* Disable interrupts */
	__disable_irq();
}
void APP_Assert(char* file, uint32_t line)
{ 
	APP_STDPrintf("[ASSERT]Wrong parameters value: file %s on line %d\n", file, line);
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
void APP_Init(void)
{
#if USE_MPU == 1
	MPU_Memory_Protection();
#endif
	APP_SoftTimer_Init();
	APP_SoftTimer_Create(TIMER_SYS,10, 1, 1, SysTimer_Callback, (void *)0);
	APP_MemInit();
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4); 
#if USE_DEBUG == 1
	APP_Debug_Init();
#endif
	HAL_Init();
}
