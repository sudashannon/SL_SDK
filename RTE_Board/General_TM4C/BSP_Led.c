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
  {LED_0 ,GPION_BASE,GPIO_PIN_0,SYSCTL_PERIPH_GPION,LED_OFF},   // PD2,LED0
};
//--------------------------------------------------------------
// LED ausschalten
//--------------------------------------------------------------
void BSP_LED_Off(BSP_LED_Name_t led_name)
{
	GPIOPinWrite(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin, 0x00);
}

//--------------------------------------------------------------
// LED einschalten
//--------------------------------------------------------------
void BSP_LED_On(BSP_LED_Name_t led_name)
{
	GPIOPinWrite(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin, 0x01);
} 
//--------------------------------------------------------------
// LED toggeln
//--------------------------------------------------------------
void BSP_LED_Toggle(BSP_LED_Name_t led_name)
{
  GPIOPinWrite(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin,
		(!GPIOPinRead(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin))&&LedControlArray[led_name].LedPin);
}
void BSP_LED_Init(void)
{
  for(BSP_LED_Name_t led_name=(BSP_LED_Name_t)0;led_name<LED_N;led_name++) 
	{
		SysCtlPeripheralEnable(LedControlArray[led_name].LedClk);
    while(!SysCtlPeripheralReady(LedControlArray[led_name].LedClk))
    {
    }
		GPIOPinTypeGPIOOutput(LedControlArray[led_name].LedPort, LedControlArray[led_name].LedPin);
    // Default Wert einstellen
    if(LedControlArray[led_name].LedInitStatus==LED_OFF) {
      BSP_LED_Off(led_name);
    }
    else {
      BSP_LED_On(led_name);
    }
	}
}


