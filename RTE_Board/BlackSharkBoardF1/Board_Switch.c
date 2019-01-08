#include "Board_Switch.h"
const static Board_Switch_Handle_t SwitchControlArray[SWITCH_N]=
{
  // Name    ,PORT , PIN ,                        Init
	{SWITCH_1 ,GPIOC,GPIO_Pin_8,RCC_APB2Periph_GPIOC,false},  
	{SWITCH_2 ,GPIOC,GPIO_Pin_7,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_3 ,GPIOC,GPIO_Pin_9,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_4 ,GPIOB,GPIO_Pin_15,RCC_APB2Periph_GPIOB,false}, 
	{SWITCH_5 ,GPIOC,GPIO_Pin_6,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_6 ,GPIOA,GPIO_Pin_15,RCC_APB2Periph_GPIOA,false}, 
	{SWITCH_7 ,GPIOC,GPIO_Pin_15,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_8 ,GPIOB,GPIO_Pin_14,RCC_APB2Periph_GPIOB,false}, 
	{SWITCH_9 ,GPIOC,GPIO_Pin_10,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_10,GPIOB,GPIO_Pin_13,RCC_APB2Periph_GPIOB,false}, 
	{SWITCH_11,GPIOC,GPIO_Pin_13,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_12,GPIOB,GPIO_Pin_12,RCC_APB2Periph_GPIOB,false}, 
	{SWITCH_13,GPIOB,GPIO_Pin_9,RCC_APB2Periph_GPIOB,false}, 
	{SWITCH_14,GPIOC,GPIO_Pin_5,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_15,GPIOB,GPIO_Pin_8,RCC_APB2Periph_GPIOB,false}, 
	{SWITCH_16,GPIOC,GPIO_Pin_4,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_17,GPIOB,GPIO_Pin_7,RCC_APB2Periph_GPIOB,false}, 
	{SWITCH_18,GPIOA,GPIO_Pin_2,RCC_APB2Periph_GPIOA,false}, 
	{SWITCH_19,GPIOB,GPIO_Pin_6,RCC_APB2Periph_GPIOB,false}, 
	{SWITCH_20,GPIOA,GPIO_Pin_1,RCC_APB2Periph_GPIOA,false}, 
	{SWITCH_21,GPIOB,GPIO_Pin_5,RCC_APB2Periph_GPIOB,false}, 
	{SWITCH_22,GPIOA,GPIO_Pin_0,RCC_APB2Periph_GPIOA,false}, 
	{SWITCH_23,GPIOB,GPIO_Pin_4,RCC_APB2Periph_GPIOB,false}, 
	{SWITCH_24,GPIOC,GPIO_Pin_3,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_25,GPIOB,GPIO_Pin_3,RCC_APB2Periph_GPIOB,false}, 
	{SWITCH_26,GPIOC,GPIO_Pin_2,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_27,GPIOC,GPIO_Pin_12,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_28,GPIOC,GPIO_Pin_1,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_29,GPIOC,GPIO_Pin_11,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_30,GPIOC,GPIO_Pin_0,RCC_APB2Periph_GPIOC,false}, 
	{SWITCH_31,GPIOC,GPIO_Pin_14,RCC_APB2Periph_GPIOC,false}, 
};
void Board_SwitchOn(Board_Switch_e switch_name)
{
	SwitchControlArray[switch_name].SwitchPort->BSRR = SwitchControlArray[switch_name].SwitchPin;
}

void Board_SwitchOff(Board_Switch_e switch_name)
{
	SwitchControlArray[switch_name].SwitchPort->BRR = SwitchControlArray[switch_name].SwitchPin;
} 
uint8_t Board_SwitchGetState(Board_Switch_e switch_name)
{
	return GPIO_ReadOutputDataBit(SwitchControlArray[switch_name].SwitchPort,SwitchControlArray[switch_name].SwitchPin);
}
void Board_SwitchInit(void)
{
	for(Board_Switch_e switch_name=(Board_Switch_e)0;switch_name<SWITCH_N;switch_name++) 
	{
		GPIO_InitTypeDef  GPIO_InitStructure;
		// Clock Enable
		RCC_APB2PeriphClockCmd(SwitchControlArray[switch_name].SwitchClk, ENABLE);
		if(switch_name == SWITCH_6|| switch_name == SWITCH_23 || switch_name == SWITCH_25)
		{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
			GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
			GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
		}
		// Config als Digital-Ausgang
		GPIO_InitStructure.GPIO_Pin = SwitchControlArray[switch_name].SwitchPin;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_Init(SwitchControlArray[switch_name].SwitchPort, &GPIO_InitStructure);
		// Default Wert einstellen
		if(SwitchControlArray[switch_name].SwitchInitStatus==false) {
		  Board_SwitchOff(switch_name);
		}
		else {
		  Board_SwitchOn(switch_name);
		}
	}
}
