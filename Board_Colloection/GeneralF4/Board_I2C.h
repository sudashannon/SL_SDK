#ifndef __BOARD_I2C_H
#define __BOARD_I2C_H
#include "APP_INCLUDE.h"
#include "BSP_INCLUDE.h"
#define I2C_SCLPin  GPIO_Pin_8
#define I2C_SDAPin  GPIO_Pin_9
#define I2C_GPIO    GPIOB
#ifdef STM32F4
#define I2C_GPIOCLK RCC_AHB1Periph_GPIOB
#endif
#ifdef STM32F1
#define I2C_GPIOCLK RCC_APB2Periph_GPIOB
#endif
#define SoftI2C_SDARead() GPIO_ReadInputDataBit(I2C_GPIO,I2C_SDAPin)
#define SoftI2C_SDASetHigh()	GPIO_SetBits(I2C_GPIO, I2C_SDAPin)
#define SoftI2C_SDASetLow()	  GPIO_ResetBits(I2C_GPIO, I2C_SDAPin)
#define SoftI2C_SCLSetHigh()	GPIO_SetBits(I2C_GPIO, I2C_SCLPin)
#define SoftI2C_SCLSetLow()	  GPIO_ResetBits(I2C_GPIO, I2C_SCLPin)
void SoftI2C_Init(void);
void SoftI2C_StartSignal(void);
void SoftI2C_StopSignal(void);
uint8_t SoftI2C_WaitAck(void);
uint8_t SoftI2C_ReadByte(uint8_t ack);
void SoftI2C_SendByte(uint8_t txd);
#endif
