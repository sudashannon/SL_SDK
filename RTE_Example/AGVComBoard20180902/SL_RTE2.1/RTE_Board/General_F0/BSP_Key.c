#include "BSP_Key.h"
static Key_Handle_t KeyHandle[KEY_N] =
{
 {.KeyName = KEY_0, .KeyPort = GPIOA, .KeyPin = GPIO_Pin_0, .KeyClk = RCC_AHBPeriph_GPIOA,.KeyPressValue = 0},
};
static StateMachine_t KeySM = 
{
	.StateNum = 0,
	.RunningState = 0,
	.FunctionListTable = (void *)0,
};
void Board_Key_Init(Key_Name_e KeyName)
{
	memset(KeyHandle[KeyName].KeyFIFO.Key_Buffer,0,KEY_FIFO_SIZE);
	KeyHandle[KeyName].KeyFIFO.WritePointer = 0;
	KeyHandle[KeyName].KeyFIFO.ReadPointer = 0;
}
