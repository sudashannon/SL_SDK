#ifndef __BOARD_SPI_H
#define __BOARD_SPI_H
#include "APP_INCLUDE.h"
#include "BSP_INCLUDE.h"
void Board_SPIInit(void);
void Board_SPIWrite8bit(uint8_t value);
uint8_t Board_SPIRead8bit(void);
#endif
