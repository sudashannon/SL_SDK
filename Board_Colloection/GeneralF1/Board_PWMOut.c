#include "Board_PWMOut.h"
volatile static uint16_t ARRValue = 0; 
void PWM_ChannelNInit(uint8_t Channel,double Period)
{
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_Pulse = (uint16_t)ARRValue*Period;
	switch(Channel)
	{
		case 1:
		{
			/* PWM1 Mode configuration: Channel1 */
			TIM_OC1Init(TIM3, &TIM_OCInitStructure);
			TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
		}break;
		case 2:
		{
			TIM_OC2Init(TIM3, &TIM_OCInitStructure);
			TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
		}break;
		case 3:
		{
			TIM_OC3Init(TIM3, &TIM_OCInitStructure);
			TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
		}break;
		case 4:
		{
			TIM_OC4Init(TIM3, &TIM_OCInitStructure);
			TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
		}break;
		default:
			break;
	}
}
/**
  * @brief  Configure the TIM3 Ouput Channels.
  * @param  None
  * @retval None
  */
void PWM_Init(uint32_t TimerClk,uint32_t PWMFrequency)
{
  /* GPIOA and GPIOB clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                         RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
  GPIO_InitTypeDef GPIO_InitStructure;
#ifdef STM32F10X_CL
  /*GPIOB Configuration: TIM3 channel1, 2, 3 and 4 */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);	
#else
  /* GPIOA Configuration:TIM3 Channel1, 2, 3 and 4 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOA, &GPIO_InitStructure);

//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  /* Time base configuration */
	ARRValue = (uint16_t)(TimerClk/PWMFrequency)-1;
  TIM_TimeBaseStructure.TIM_Period = ARRValue;
  TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t) (SystemCoreClock / TimerClk) - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
}
void PWM_TimerStart(void)
{
  TIM_ARRPreloadConfig(TIM3, ENABLE);
  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}
void PWM_ChannelNSetPeriod(uint8_t Channel,double Period)
{
	switch(Channel)
	{
		case 1:
		{
			TIM_SetCompare1(TIM3, (uint16_t)ARRValue*Period);
		}break;
		case 2:
		{
			TIM_SetCompare2(TIM3, (uint16_t)ARRValue*Period);
		}break;
		case 3:
		{
			TIM_SetCompare3(TIM3, (uint16_t)ARRValue*Period);
		}break;
		case 4:
		{
			TIM_SetCompare4(TIM3, (uint16_t)ARRValue*Period);
		}break;
		default:
			break;
	}
}
