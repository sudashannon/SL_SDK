#ifndef __BOARD_E2PROM_H
#define __BOARD_E2PROM_H
#include "APP_Include.h"
#include "BSP_Include.h"
//EEPROM24c02相关函数
uint8_t AT24C02_ReadByte(uint8_t ReadAddr);							     //指定地址读取一个字节
void AT24C02_WriteByte(uint8_t WriteAddr,uint8_t DataToWrite);		//指定地址写入一个字节

uint32_t Buf_4Byte(uint8_t *pBuffer,uint32_t Date_4Byte,uint8_t Byte_num,uint8_t mode);

void AT24C02_Write(uint8_t WriteAddr,uint8_t *pBuffer,uint8_t WriteNum);	//从指定地址开始写入指定长度的数据
void AT24C02_Read(uint8_t ReadAddr,uint8_t *pBuffer,uint8_t ReadNum);   	//从指定地址开始读出指定长度的数据

uint8_t AT24C02_Test(void);  //检查器件
void AT24C02_Init(void); //初始化IIC
#endif
