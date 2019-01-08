#include "Board_WTD.h"
/**
 * 初始化独立看门狗
 * prer:分频数:0~7(只有低 3 位有效!)
 * 分频因子=4*2^prer.但最大值只能是 256!
 * rlr:重装载寄存器值:低 11 位有效.
 * 时间计算(大概):Tout=((4*2^prer)*rlr)/40 (ms).
 */
void Board_WTD_Init(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); /* 使能对寄存器IWDG_PR和IWDG_RLR的写操作*/
    IWDG_SetPrescaler(4);    /*设置IWDG预分频值:设置IWDG预分频值*/
    IWDG_SetReload(625);     /*设置IWDG重装载值*/
    IWDG_ReloadCounter();    /*按照IWDG重装载寄存器的值重装载IWDG计数器*/
    IWDG_Enable();        /*使能IWDG*/
}
void Board_WTD_Feed(void *par)
{		
	IWDG_ReloadCounter();    /*reload*/
}
