#ifndef __CAN_SEND_H
#define __CAN_SEND_H
#include "BSP_CAN.h"
extern void CANAGVModeControl(bool AGVMode);
extern bool CANDataSend(uint8_t CanCommand,uint8_t DestFloor,uint8_t CurrentFloor);
#endif
