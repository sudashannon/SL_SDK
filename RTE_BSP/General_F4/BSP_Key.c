#include "BSP_Key.h"
BSP_Key_Handle_t KeyHandle[KEY_N] =
{
	 {.KeyName = KEY_0,.KeyPort = GPIOB,.KeyPin = GPIO_Pin_0,.KeyClk = RCC_APB2Periph_GPIOB,.KeyPressValue = 0},
	 {.KeyName = KEY_1,.KeyPort = GPIOB,.KeyPin = GPIO_Pin_1,.KeyClk = RCC_APB2Periph_GPIOB,.KeyPressValue = 0},
};
void BSP_Key_Init(BSP_Key_Name_e KeyName)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
    // Clock Enable
    RCC_APB2PeriphClockCmd(KeyHandle[KeyName].KeyClk, ENABLE);
    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = KeyHandle[KeyName].KeyPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KeyHandle[KeyName].KeyPort, &GPIO_InitStructure);
	KeyHandle[KeyName].KeyFIFO.Key_Buffer = (BSP_Key_State_e *)RTE_MEM_Alloc0(MEM_RTE,KEY_FIFO_SIZE);
    KeyHandle[KeyName].KeyFIFO.WritePointer = 0;
    KeyHandle[KeyName].KeyFIFO.ReadPointer = 0;
}
bool BSP_Key_ReadInstant(BSP_Key_Name_e KeyName)
{
	return (GPIO_ReadInputDataBit(KeyHandle[KeyName].KeyPort,KeyHandle[KeyName].KeyPin) == KeyHandle[KeyName].KeyPressValue);
}
void BSP_Key_ClearBuffer(BSP_Key_Name_e KeyName)
{
	memset(KeyHandle[KeyName].KeyFIFO.Key_Buffer,0,KEY_FIFO_SIZE);
    KeyHandle[KeyName].KeyFIFO.WritePointer = 0;
    KeyHandle[KeyName].KeyFIFO.ReadPointer = 0;
}
BSP_Key_State_e BSP_Key_Read(BSP_Key_Name_e KeyName)
{
    if(KeyHandle[KeyName].KeyFIFO.ReadPointer == KeyHandle[KeyName].KeyFIFO.WritePointer)
        return KEY_NONE;
    else
    {
        BSP_Key_State_e ret = KeyHandle[KeyName].KeyFIFO.Key_Buffer[KeyHandle[KeyName].KeyFIFO.ReadPointer];
        KeyHandle[KeyName].KeyFIFO.ReadPointer++;
        if (KeyHandle[KeyName].KeyFIFO.ReadPointer >= KEY_FIFO_SIZE)
        {
            KeyHandle[KeyName].KeyFIFO.ReadPointer = 0;
        }
        return ret;
    }
}
void BSP_Key_Timer_Callback(void* UserParameters)
{
    for(BSP_Key_Name_e KeyName = (BSP_Key_Name_e)0;KeyName<KEY_N;KeyName++)
	{
		if(!KeyHandle[KeyName].KeyFIFO.Key_Buffer)
			return;
		switch(KeyHandle[KeyName].KeySM)
		{
			case KEY_SM_POLL:
			{
				if(GPIO_ReadInputDataBit(KeyHandle[KeyName].KeyPort,KeyHandle[KeyName].KeyPin) == KeyHandle[KeyName].KeyPressValue)
				{
					KeyHandle[KeyName].KeySM = KEY_SM_PRESS;
	#if KEY_LONGPRESS_ENABLE
					KeyHandle[KeyName].KeyStartTick = RTE_RoundRobin_GetTick();
	#endif
				}
			}break;
			case KEY_SM_PRESS:
			{
				if(GPIO_ReadInputDataBit(KeyHandle[KeyName].KeyPort,KeyHandle[KeyName].KeyPin) == KeyHandle[KeyName].KeyPressValue)
				{
					KeyHandle[KeyName].KeyFIFO.Key_Buffer[KeyHandle[KeyName].KeyFIFO.WritePointer] = KEY_PRESS;
					KeyHandle[KeyName].KeyFIFO.WritePointer++;
					KeyHandle[KeyName].KeySM = KEY_SM_RELEASE;
					if(KeyHandle[KeyName].KeyFIFO.WritePointer == KEY_FIFO_SIZE)
					{
						KeyHandle[KeyName].KeyFIFO.WritePointer = 0;
					}
				}
				else
					KeyHandle[KeyName].KeySM = KEY_SM_POLL;
			}break;
			case KEY_SM_RELEASE:
			{
				if(GPIO_ReadInputDataBit(KeyHandle[KeyName].KeyPort,KeyHandle[KeyName].KeyPin) != KeyHandle[KeyName].KeyPressValue)
				{
					KeyHandle[KeyName].KeyFIFO.Key_Buffer[KeyHandle[KeyName].KeyFIFO.WritePointer] = KEY_RELEASE;
					KeyHandle[KeyName].KeyFIFO.WritePointer++;
					KeyHandle[KeyName].KeySM = KEY_SM_POLL;
					if(KeyHandle[KeyName].KeyFIFO.WritePointer == KEY_FIFO_SIZE)
					{
						KeyHandle[KeyName].KeyFIFO.WritePointer = 0;
					}
				}
	#if KEY_LONGPRESS_ENABLE
				else if(KeyHandle[KeyName].KeyStartTick+KEY_LONGPRESS_TIME <= RTE_RoundRobin_GetTick())
				{
					KeyHandle[KeyName].KeyFIFO.Key_Buffer[KeyHandle[KeyName].KeyFIFO.WritePointer] = KEY_LONG;
					KeyHandle[KeyName].KeyFIFO.WritePointer++;
					if(KeyHandle[KeyName].KeyFIFO.WritePointer == KEY_FIFO_SIZE)
					{
						KeyHandle[KeyName].KeyFIFO.WritePointer = 0;
					}
					KeyHandle[KeyName].KeyStartTick = RTE_RoundRobin_GetTick();
				}
	#endif
			}break;
		}
	}
}
