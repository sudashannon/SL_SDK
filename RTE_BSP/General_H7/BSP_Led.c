#include "BSP_Led.h"
//--------------------------------------------------------------
// Definition aller LEDs
// Reihenfolge wie bei LED_NAME_t
//
// Init : [LED_OFF,LED_ON]
//--------------------------------------------------------------
static BSP_LED_Handle_t LedControlArray[LED_N]=
{
  // Name    ,PORT , PIN ,   Init
	{LED_0 ,GPIOC,GPIO_PIN_0 ,LED_OFF},   // PC0,LED0
};

//--------------------------------------------------------------
// LED ausschalten
//--------------------------------------------------------------
void BSP_LED_Off(BSP_LED_Name_t led_name)
{
	HAL_GPIO_WritePin(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin, GPIO_PIN_RESET);
}

//--------------------------------------------------------------
// LED einschalten
//--------------------------------------------------------------
void BSP_LED_On(BSP_LED_Name_t led_name)
{
	HAL_GPIO_WritePin(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin, GPIO_PIN_SET);
} 
//--------------------------------------------------------------
// LED toggeln
//--------------------------------------------------------------
void BSP_LED_Toggle(BSP_LED_Name_t led_name)
{
  LedControlArray[led_name].LedPort->ODR ^= LedControlArray[led_name].LedPin;
}
void BSP_LED_Init(void)
{
  for(BSP_LED_Name_t led_name=(BSP_LED_Name_t)0;led_name<LED_N;led_name++) 
	{
		GPIO_InitTypeDef  GPIO_InitStruct;
		__HAL_RCC_GPIOC_CLK_ENABLE();
		GPIO_InitStruct.Pin = LedControlArray[led_name].LedPin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(LedControlArray[led_name].LedPort, &GPIO_InitStruct);
    // Default Wert einstellen
    if(LedControlArray[led_name].LedInitStatus==LED_OFF) {
      BSP_LED_Off(led_name);
    }
    else {
      BSP_LED_On(led_name);
    }
	}
}


