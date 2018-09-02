#include "Board_SR501.h"
#include "APP_SoftTimer.h"
SR501Result_t SR501Result;
void SR501_Init(void)  
{       
	GPIO_InitTypeDef  GPIO_InitStructure;  
	RCC_APB2PeriphClockCmd(SR501_GPIOCLK, ENABLE);     //使能PC端口时钟  
	GPIO_InitStructure.GPIO_Pin = SR501_DataPin;                 //PC.0 端口配置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;          //推挽输出  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(SR501_GPIO, &GPIO_InitStructure);  
}
void SR501_Check(void)
{
	static uint32_t IfPeopleStartTick,NonePeopleStartTick;
	switch(SR501Result.SR501State)
	{
		case IDLE:
		{
			if(GPIO_ReadInputDataBit(SR501_GPIO, SR501_DataPin))
			{
				IfPeopleStartTick = APP_SoftTimer_GetTick();
				SR501Result.SR501State = TIMER;
			}
			else
			{
				if(APP_SoftTimer_GetTick() - NonePeopleStartTick >= 5000)
				{
					SR501Result.IfPeople = false;
				}
				SR501Result.SR501State = IDLE;
			}
		}break;
		case TIMER:
		{
			if(GPIO_ReadInputDataBit(SR501_GPIO, SR501_DataPin))
			{
			
			}
			else
			{
				SR501Result.HighTime = APP_SoftTimer_GetTick() - IfPeopleStartTick;
				if(SR501Result.HighTime>2000)
				{
					SR501Result.IfPeople = true;
				}
				SR501Result.HighTime = 0;
				IfPeopleStartTick = 0;
				SR501Result.SR501State = END;
			}
		}break;
		case END:
		{
			if(GPIO_ReadInputDataBit(SR501_GPIO, SR501_DataPin))
			{
				SR501Result.SR501State = END;
			}
			else
			{
				NonePeopleStartTick = APP_SoftTimer_GetTick();
				SR501Result.SR501State = IDLE;
			}
		}break;
		default:
			break;
	}
}


