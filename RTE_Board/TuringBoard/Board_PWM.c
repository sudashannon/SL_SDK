#include "Board_PWM.h"
Board_Pwm_Control_t PwmHandle[PWM_N] = 
{
	{
		.PwmName = PWM_DCMI,
		.Instance = TIM1,
		.Channel = TIM_CHANNEL_1,
	},
};
int Board_PWM_Config(Board_Pwm_Name_e PwmName,int frequency,void (*MSPInitCallback)(void))
{
	PwmHandle[PwmName].MSPInitCallback = MSPInitCallback;
	/* TCLK (PCLK * 2) */
	int tclk = TIMx_PCLK_FREQ() * 2;
	/* Period should be even */
	int period = (tclk / frequency) - 1;

	if (PwmHandle[PwmName].TIMHalHandle.Init.Period && (PwmHandle[PwmName].TIMHalHandle.Init.Period != period)) {
		__HAL_TIM_SET_AUTORELOAD(&PwmHandle[PwmName].TIMHalHandle, period);
		__HAL_TIM_SET_COMPARE(&PwmHandle[PwmName].TIMHalHandle, PwmHandle[PwmName].Channel, period / 2);
		return 0;
	}

	/* Timer base configuration */
	PwmHandle[PwmName].TIMHalHandle.Instance           = PwmHandle[PwmName].Instance;
	PwmHandle[PwmName].TIMHalHandle.Init.Period        = period;
	PwmHandle[PwmName].TIMHalHandle.Init.Prescaler     = TIM_ETRPRESCALER_DIV1;
	PwmHandle[PwmName].TIMHalHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
	PwmHandle[PwmName].TIMHalHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

	/* Timer channel configuration */
	PwmHandle[PwmName].TIMOCHandle.Pulse       = period / 2;
	PwmHandle[PwmName].TIMOCHandle.OCMode      = TIM_OCMODE_PWM1;
	PwmHandle[PwmName].TIMOCHandle.OCPolarity  = TIM_OCPOLARITY_HIGH;
	PwmHandle[PwmName].TIMOCHandle.OCFastMode  = TIM_OCFAST_DISABLE;
	PwmHandle[PwmName].TIMOCHandle.OCIdleState = TIM_OCIDLESTATE_RESET;

	if ((HAL_TIM_PWM_Init(&PwmHandle[PwmName].TIMHalHandle) != HAL_OK)
	|| (HAL_TIM_PWM_ConfigChannel(&PwmHandle[PwmName].TIMHalHandle, &PwmHandle[PwmName].TIMOCHandle, PwmHandle[PwmName].Channel) != HAL_OK)
	|| (HAL_TIM_PWM_Start(&PwmHandle[PwmName].TIMHalHandle, PwmHandle[PwmName].Channel) != HAL_OK)) {
		return -1;
	}
	return 0;
}
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
	for(Board_Pwm_Name_e i = (Board_Pwm_Name_e)0;i<PWM_N;i++)
	{
		if(htim->Instance == PwmHandle[i].Instance)
		{
			PwmHandle[i].MSPInitCallback();
			break;
		}
	}
}
