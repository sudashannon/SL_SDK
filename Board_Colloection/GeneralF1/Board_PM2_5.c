#include "Board_PM2_5.h"
#include "Board_SoftCom.h"
#include "BSP_ComF1.h"
#include "APP_StdLib.h"
PM2_5Data_t PM2_5ModuleData;
void PM2_5Module_Init(void)
{
	PM2_5ModuleData.RecBuffer = bget(8);
}
void PM2_5Module_Poll(void)
{
	if(APP_RingBuffer_DeQuene(&SoftComData.ComQuene,(uint8_t *)PM2_5ModuleData.RecBuffer,(uint16_t *)&PM2_5ModuleData.RecLength)==QUE_NO_ERR)
	{
		if(PM2_5ModuleData.RecLength==7)
		{
				if(PM2_5ModuleData.RecBuffer[5]==PM2_5ModuleData.RecBuffer[1]+PM2_5ModuleData.RecBuffer[2]
						+PM2_5ModuleData.RecBuffer[3]+PM2_5ModuleData.RecBuffer[4])
				{
					float OutVoltage = 0.0;
					OutVoltage = (float)((PM2_5ModuleData.RecBuffer[1] * 256) + PM2_5ModuleData.RecBuffer[2]) * 2.5 / 1024;
					int k_val = 0;
					static uint8_t DataCnt = 0;
					if(OutVoltage <= 0.045) k_val = 200;
					else if(OutVoltage <= 0.048) k_val = 400;
					else if(OutVoltage <= 0.051) k_val = 600;
					else if(OutVoltage <= 0.054) k_val = 750;
					else if(OutVoltage <= 0.058) k_val = 900;
					else if(OutVoltage <= 0.064) k_val = 1000;
					else if(OutVoltage <= 0.070) k_val = 1250;
					else if(OutVoltage <= 0.075) k_val = 1400;
					else if(OutVoltage <= 0.080) k_val = 1700;
					else if(OutVoltage <= 0.085) k_val = 1800;
					else if(OutVoltage <= 0.090) k_val = 1900;
					else if(OutVoltage <= 0.100) k_val = 2000;
					else if(OutVoltage <= 0.110) k_val = 2200;
					else k_val = 2200;
					if(DataCnt<32)
					{
						PM2_5ModuleData.PPMArray[DataCnt] = OutVoltage*k_val;
						DataCnt++;
					}
					else
					{
						for(uint8_t i=0;i<32;i++)
						{
							PM2_5ModuleData.PPM = PM2_5ModuleData.PPM+PM2_5ModuleData.PPMArray[i];
						}
						PM2_5ModuleData.PPM = PM2_5ModuleData.PPM>>5;
						DataCnt = 0;
						memset(PM2_5ModuleData.PPMArray,0,32);
					}
				}
		}
		memset(PM2_5ModuleData.RecBuffer,0,8);
		PM2_5ModuleData.RecLength = 0; 
	}
}
