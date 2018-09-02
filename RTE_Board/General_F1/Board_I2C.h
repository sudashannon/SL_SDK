#ifndef __BOARD_I2C_H
#define __BOARD_I2C_H
#include "APP_INCLUDE.h"
#include "BSP_INCLUDE.h"
#define I2C_SCLPin  GPIO_Pin_6
#define I2C_SDAPin  GPIO_Pin_7
#define I2C_GPIO    GPIOB
#define I2C_GPIOCLK RCC_APB2Periph_GPIOB
#define I2C_SDARead() GPIO_ReadInputDataBit(I2C_GPIO,I2C_SDAPin)
#define I2C_SDASetHigh()	GPIO_SetBits(I2C_GPIO, I2C_SDAPin)
#define I2C_SDASetLow()	  GPIO_ResetBits(I2C_GPIO, I2C_SDAPin)
#define I2C_SCLSetHigh()	GPIO_SetBits(I2C_GPIO, I2C_SCLPin)
#define I2C_SCLSetLow()	  GPIO_ResetBits(I2C_GPIO, I2C_SCLPin)
void I2C_Init(void);
void I2C_StartSignal(void);
void I2C_StopSignal(void);
uint8_t I2C_WaitAck(void);
uint8_t I2C_ReadByte(uint8_t ack);
void I2C_SendByte(uint8_t txd);
#endif
