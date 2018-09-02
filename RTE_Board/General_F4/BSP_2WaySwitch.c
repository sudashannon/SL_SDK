#include "BSP_2WaySwitch.h"
static BSP_2WaySwitch_t Way2SwitchHandle[SWITCH_N] = 
{
	{SWITCH_01, GPIOA, GPIO_Pin_4, GPIO_Pin_5, RCC_AHB1Periph_GPIOA ,STATUS_NONE},
	{SWITCH_02, GPIOA, GPIO_Pin_6, GPIO_Pin_7, RCC_AHB1Periph_GPIOA ,STATUS_NONE},
	{SWITCH_03, GPIOC, GPIO_Pin_4, GPIO_Pin_5, RCC_AHB1Periph_GPIOC ,STATUS_NONE},
	{SWITCH_04, GPIOB, GPIO_Pin_0, GPIO_Pin_1, RCC_AHB1Periph_GPIOB ,STATUS_NONE},
	{SWITCH_05, GPIOE, GPIO_Pin_8, GPIO_Pin_9, RCC_AHB1Periph_GPIOE ,STATUS_NONE},
	{SWITCH_06, GPIOE, GPIO_Pin_10, GPIO_Pin_11, RCC_AHB1Periph_GPIOE ,STATUS_NONE},
	{SWITCH_07, GPIOE, GPIO_Pin_12, GPIO_Pin_13, RCC_AHB1Periph_GPIOE ,STATUS_NONE},
	{SWITCH_08, GPIOE, GPIO_Pin_14, GPIO_Pin_15, RCC_AHB1Periph_GPIOE ,STATUS_NONE},
	{SWITCH_09, GPIOB, GPIO_Pin_10, GPIO_Pin_11, RCC_AHB1Periph_GPIOB ,STATUS_NONE},
	{SWITCH_10, GPIOB, GPIO_Pin_12, GPIO_Pin_13, RCC_AHB1Periph_GPIOB ,STATUS_NONE},
	{SWITCH_11, GPIOB, GPIO_Pin_14, GPIO_Pin_15, RCC_AHB1Periph_GPIOB ,STATUS_NONE},
	{SWITCH_12, GPIOD, GPIO_Pin_8, GPIO_Pin_9, RCC_AHB1Periph_GPIOD ,STATUS_NONE},
	{SWITCH_13, GPIOD, GPIO_Pin_10, GPIO_Pin_11, RCC_AHB1Periph_GPIOD ,STATUS_NONE},
	{SWITCH_14, GPIOD, GPIO_Pin_12, GPIO_Pin_13, RCC_AHB1Periph_GPIOD ,STATUS_NONE},
	{SWITCH_15, GPIOD, GPIO_Pin_14, GPIO_Pin_15, RCC_AHB1Periph_GPIOD ,STATUS_NONE},
};
void BSP_2WaySwitch_Init(void)
{
	for(BSP_2WaySwtch_Name_e i = SWITCH_01;i<SWITCH_N;i++)
	{
	  GPIO_InitTypeDef  GPIO_InitStructure;
		// Clock Enable
    RCC_AHB1PeriphClockCmd(Way2SwitchHandle[i].SwitchClk, ENABLE);
    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = Way2SwitchHandle[i].NCPin|Way2SwitchHandle[i].NOPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Way2SwitchHandle[i].SwitchPort, &GPIO_InitStructure);
	}
}
BSP_2WaySwtch_Status_e BSP_2WaySwitch_ReadStatus(BSP_2WaySwtch_Name_e i)
{
	if(GPIO_ReadInputDataBit(Way2SwitchHandle[i].SwitchPort,Way2SwitchHandle[i].NCPin) == 0)
		return STATUS_NC;
	else if(GPIO_ReadInputDataBit(Way2SwitchHandle[i].SwitchPort,Way2SwitchHandle[i].NOPin) == 0)
		return STATUS_NO;
	else
		return STATUS_NONE;
}


