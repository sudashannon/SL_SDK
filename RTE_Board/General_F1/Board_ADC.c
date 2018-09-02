#include "Board_ADC.h"
//PA0      ADC123_IN0
//PA1      ADC123_IN1
//PA2      ADC123_IN2
//PA3      ADC123_IN3
//PA4      ADC12_IN4
//PA5      ADC12_IN5
//PA6      ADC12_IN6
//PA7      ADC12_IN7
//PB0      ADC12_IN8
//PB1      ADC12_IN9
//PC0      ADC123_IN10
//PC1      ADC123_IN11
//PC2      ADC123_IN12
//PC3      ADC123_IN13
//PC4      ADC123_IN14
//PC5      ADC123_IN15
//PF6      ADC3_IN4
//PF7      ADC3_IN5       
//PF8      ADC3_IN6 
//PF9      ADC3_IN7
//PF10     ADC3_IN8
//---外设   通道
//---ADC1  通道1
//---ADC3  通道5
//--------------------------------------------------------------
// Definition der benutzten ADC Pins (max=16)
// Reihenfolge wie bei ADC1d_NAME_t
//--------------------------------------------------------------
BoardADC_Control_t ADC1d[] = {
  //NAME  ,PORT , PIN      , CLOCK              , Kanal
//  {ADC_PA5,GPIOA,GPIO_Pin_5,RCC_APB2Periph_GPIOA,ADC_Channel_5 },   // ADC an PA5 = ADC12_IN5
  {ADC_PB0,GPIOB,GPIO_Pin_0,RCC_APB2Periph_GPIOB,ADC_Channel_8},   // ADC an PC3 = ADC123_IN13
};
//--------------------------------------------------------------
// Globale Variabeln
//--------------------------------------------------------------
volatile uint16_t ADC1_DMA_Buffer[ADC_NUM];  // Globaler ADC-Puffer
//--------------------------------------------------------------
// auslesen eines Messwertes
//--------------------------------------------------------------
uint16_t Board_ReadADC1DMA(BoardADC_Name_e adc_name)
{
  uint16_t messwert=0;

  messwert=(uint16_t)(ADC1_DMA_Buffer[adc_name]);

  return(messwert);
}
//--------------------------------------------------------------
// interne Funktion
// Init aller IO-Pins
//--------------------------------------------------------------
void Board_InitADC1IO(void) {
  GPIO_InitTypeDef      GPIO_InitStructure;
  BoardADC_Name_e adc_name;

  for(adc_name = (BoardADC_Name_e)0;adc_name<ADC_NUM;adc_name++) {
    // Clock Enable
    RCC_APB2PeriphClockCmd(ADC1d[adc_name].ADC_CLK, ENABLE);

    // Config des Pins als Analog-Eingang
    GPIO_InitStructure.GPIO_Pin = ADC1d[adc_name].ADC_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(ADC1d[adc_name].ADC_PORT, &GPIO_InitStructure);
  }
}
//--------------------------------------------------------------
// interne Funktion
// Init von DMA Nr.2
//--------------------------------------------------------------
void Board_InitADC1DMA(void)
{
  DMA_InitTypeDef       DMA_InitStructure;

  // Clock Enable
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  // DMA-Disable
  /* DMA1 channel1 configuration ---------------------------------------------*/
  // DMA1-Config	
  DMA_DeInit(ADC1_DMA_CHANNEL);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_ADDRESS;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC1_DMA_Buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = ADC_NUM;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(ADC1_DMA_CHANNEL, &DMA_InitStructure);
  /* Enable DMA1 channel1 */
  DMA_Cmd(ADC1_DMA_CHANNEL, ENABLE);
}
//--------------------------------------------------------------
// interne Funktion
// Init von ADC Nr.1
//--------------------------------------------------------------
void Board_InitADC1(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  BoardADC_Name_e adc_name;

  // Clock Enable
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(ADC1d_VORTEILER);
  // ADC-Config
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = ADC_NUM;
  ADC_Init(ADC1, &ADC_InitStructure);	

  // hinzufuegen aller ADC-Kan?le in die Regular-Group
  for(adc_name = (BoardADC_Name_e)0;adc_name<ADC_NUM;adc_name++) {
    ADC_RegularChannelConfig(ADC1, ADC1d[adc_name].ADC_CH, adc_name+1, ADC1d_SAMPLECYCLES);
  }
}
//--------------------------------------------------------------
// interne Funktion
// Enable und start vom ADC und DMA
//--------------------------------------------------------------
void Board_StartADC1(void)
{
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);	
  /* Enable ADC1 reset calibaration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));
  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
//--------------------------------------------------------------
// init vom ADC1 im DMA Mode
// und starten der zyklischen Wandlung
//--------------------------------------------------------------
void Board_InitADC1viaDMA(void)
{
  Board_InitADC1IO();
  Board_InitADC1DMA();
  Board_InitADC1();
  Board_StartADC1();
}
