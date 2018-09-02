/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_LED_H
#define __BSP_LED_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "RTE_Include.h"
/* Exported types ------------------------------------------------------------*/
//--------------------------------------------------------------
// Liste aller LEDs
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum 
{
  LED_BAT = 0,  			 // LED0
  LED_SIGNAL = 1,  			 // LED0
  LED_STATUS =2,  			 // LED0
	LED_N,
}BSP_LED_Name_t;
//--------------------------------------------------------------
// Status einer LED
//--------------------------------------------------------------
typedef enum {
  LED_OFF = 0,  // LED AUS
  LED_ON        // LED EIN
}BSP_LED_Status_e;
//--------------------------------------------------------------
// Struktur einer LED
//--------------------------------------------------------------
typedef struct {
  BSP_LED_Name_t LedName;    // Name
  GPIO_TypeDef* LedPort; // Port
  uint16_t LedPin; // Pin
	uint32_t LedClk; // Clock
  BSP_LED_Status_e LedInitStatus;  // Init
}BSP_LED_Handle_t;
void BSP_LED_Init(void);
void BSP_LED_Toggle(BSP_LED_Name_t led_name);
void BSP_LED_On(BSP_LED_Name_t led_name);
void BSP_LED_Off(BSP_LED_Name_t led_name);
#endif
