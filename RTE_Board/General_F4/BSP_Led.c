#include "BSP_Led.h"
//--------------------------------------------------------------
// Definition aller LEDs
// Reihenfolge wie bei LED_NAME_t
//
// Init : [LED_OFF,LED_ON]
//--------------------------------------------------------------
static BSP_LED_Handle_t LedControlArray[LED_N]=
{
  // Name    ,PORT , PIN ,                        Init
  {LED_BAT ,GPIOE,GPIO_Pin_1,RCC_AHB1Periph_GPIOE,LED_OFF},   // PD2,LED0
  {LED_SIGNAL ,GPIOE,GPIO_Pin_2,RCC_AHB1Periph_GPIOE,LED_OFF},   // PD2,LED0
  {LED_STATUS ,GPIOE,GPIO_Pin_3,RCC_AHB1Periph_GPIOG,LED_OFF},   // PD2,LED0
};
//--------------------------------------------------------------
// LED ausschalten
//--------------------------------------------------------------
void BSP_LED_Off(BSP_LED_Name_t led_name)
{
	GPIO_SetBits(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin);
}

//--------------------------------------------------------------
// LED einschalten
//--------------------------------------------------------------
void BSP_LED_On(BSP_LED_Name_t led_name)
{
	GPIO_ResetBits(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin);
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
	  GPIO_InitTypeDef  GPIO_InitStructure;
		// Clock Enable
    RCC_AHB1PeriphClockCmd(LedControlArray[led_name].LedClk, ENABLE);

    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = LedControlArray[led_name].LedPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LedControlArray[led_name].LedPort, &GPIO_InitStructure);
    // Default Wert einstellen
    if(LedControlArray[led_name].LedInitStatus==LED_OFF) {
      BSP_LED_Off(led_name);
    }
    else {
      BSP_LED_On(led_name);
    }
	}
}


