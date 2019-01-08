#ifndef __BOARD_SCCB_H
#define __BOARD_SCCB_H
#include <stdint.h>
uint8_t Board_SCCB_Probe(void);
uint8_t Board_SCCB_Read(uint8_t slv_addr, uint8_t reg_addr, uint8_t *reg_data);
uint8_t Board_SCCB_Write(uint8_t slv_addr, uint8_t reg, uint8_t data);

#endif
