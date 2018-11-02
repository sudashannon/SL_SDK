#include "BSP_Systick.h"
#include "RTE_Include.h"
void BSP_Systick_Init(void)
{
	SysTick_Config(SystemCoreClock / 1000);	//Set SysTick Timer for 1ms interrupts	
}
#if RTE_USE_OS == 0
void SysTick_Handler(void)
{
	RTE_RoundRobin_TickHandler();
}
#endif
uint32_t HAL_GetTick(void)
{
  return RTE_RoundRobin_GetTick();
}
//note:在使用操作系统的情况下调用此函数进行延时 为非阻塞式延迟
void HAL_Delay(uint32_t Delay)
{
	RTE_RoundRobin_DelayMS(Delay);
}
