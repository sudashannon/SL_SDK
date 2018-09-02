#ifndef __BOARD_ADC_H
#define __BOARD_ADC_H
#include "APP_INCLUDE.h"
#include "BSP_INCLUDE.h"
//--------------------------------------------------------------
// Liste aller ADC-Kan?le
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum {
//  ADC_PA5 = 0,  // PA5
  ADC_PB0 = 0,   // PC3
	ADC_NUM,
}BoardADC_Name_e;
//--------------------------------------------------------------
// Adressen der ADCs
// (siehe Seite 66+427 vom Referenz Manual)
//--------------------------------------------------------------
#define ADC_BASE_ADR        ((uint32_t)0x40012000)
#define ADC_ADC1_OFFSET     ((uint32_t)0x00000400)
//--------------------------------------------------------------
// Adressen der Register
// (siehe Seite 427+428 vom Referenz Manual)
//--------------------------------------------------------------
#define ADC_REG_DR_OFFSET         0x4C
#define ADC1_DR_ADDRESS    (ADC_BASE_ADR | ADC_ADC1_OFFSET | ADC_REG_DR_OFFSET)
//--------------------------------------------------------------
// ADC-Clock
// Max-ADC-Frq = 14MHz
// Grundfrequenz = APB2 (APB2=72MHz)
// M?gliche Vorteiler = 2,4,6,8
//--------------------------------------------------------------
//#define ADC1d_VORTEILER     RCC_PCLK2_Div2 // 
//#define ADC1d_VORTEILER     RCC_PCLK2_Div4   // 
#define ADC1d_VORTEILER     	RCC_PCLK2_Div6 // Frq = 12 MHz
//#define ADC1d_VORTEILER     RCC_PCLK2_Div8 // 
#define ADC1d_SAMPLECYCLES    ADC_SampleTime_28Cycles5 // TCONV = (28.5+ 12.5 ¸öÖÜÆÚ)/12=3.417us
//--------------------------------------------------------------
// DMA Einstellung
//--------------------------------------------------------------
#define ADC1_DMA_CHANNEL           DMA1_Channel1
//--------------------------------------------------------------
// Struktur eines ADC Kanals
//--------------------------------------------------------------
typedef struct {
  BoardADC_Name_e ADC_NAME;  // Name
  GPIO_TypeDef* ADC_PORT; // Port
  const uint16_t ADC_PIN; // Pin
  const uint32_t ADC_CLK; // Clock
  const uint8_t ADC_CH;   // ADC-Kanal
}BoardADC_Control_t;
//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void Board_InitADC1viaDMA(void);
uint16_t Board_ReadADC1DMA(BoardADC_Name_e adc_name);
#endif
