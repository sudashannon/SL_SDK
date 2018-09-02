#include "Board_BH1750.h"
#include "Board_I2C.h"
#include "APP_SoftTimer.h"
void BH1750_SendCmd(uint8_t command)
{
  do
	{
    I2C_StartSignal();
    I2C_SendByte(BHAddWrite);
	}while(I2C_WaitAck());
	I2C_SendByte(command);
	I2C_WaitAck();
	I2C_StopSignal();
}
uint16_t BH1750_ReadData(void)
{
	uint16_t Temp;
	I2C_StartSignal();
	I2C_SendByte(BHAddRead);
	I2C_WaitAck();
	Temp=I2C_ReadByte(1);
	Temp=Temp<<8;
	Temp+=0x00ff&I2C_ReadByte(0);
	I2C_StopSignal();
	return Temp;
}
