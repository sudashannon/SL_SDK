#include "BSP_E32.h"
static 	uint8_t E32RecData[128] = {0};
static void BSP_E32_HardInit(void)
{
	//MD0 MD1管脚
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//AUX管脚
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
uint16_t BSP_E32_GetSettings(void)
{
	uint8_t Data[3] = {0xC1,0xC1,0xC1};
	uint16_t RecLenth = 0;
	memset(E32RecData,0,128);
	BSP_COM_Data_t *Buffer;
	Buffer = BSP_COM_ReturnQue(COM_3);
	BSP_COM_SendArray(COM_3,Data,3);
	uint32_t StartTick = RTE_RoundRobin_GetTick();
	while(StartTick + 500 >= RTE_RoundRobin_GetTick())
	{
		if(RTE_MessageQuene_Out(&Buffer->ComQuene,(uint8_t *)E32RecData,&RecLenth)==MSG_NO_ERR)
		{
			if(RecLenth>0)
			{
				RTE_Printf("[E32]    获取设置成功: ");
				for(uint16_t i = 0;i < RecLenth;i++)
				{
					RTE_Printf("%02x ",E32RecData[i]);
				}
				RTE_Printf("\r\n");
				uint16_t Add = E32RecData[1]<<8|E32RecData[2];
				return Add;
			}
		}
	}
	return 0;
}
void BSP_E32_Init(void)
{
	BSP_E32_HardInit();
	while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)!=true)
	{
		
	}
	RTE_Printf("[E32]    模块启动成功！\r\n");
	RTE_Printf("[E32]    进入配置模式...\r\n");
	BSP_E32_ModeSet(MODE_SLEEP);
	while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)!=true)
	{
		
	}
	osDelay(500);
	uint16_t LocalAdd = 0;
	LocalAdd = atoi(ef_get_env("device_address"));
	BSP_E32_ArgsSet(LocalAdd);
	BSP_E32_GetSettings();
	BSP_E32_ModeSet(MODE_NORMAL);
	osDelay(500);
	RTE_Printf("[E32]    进入正常模式...\r\n");
	while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)!=true)
	{
		
	}
	RTE_Printf("[E32]    模块开始工作\r\n");
}
void BSP_E32_ModeSet(BSP_E32_WorkMode_e Mode)
{
	switch(Mode)
	{
		case MODE_NORMAL:
		{
			GPIO_ResetBits(GPIOA, GPIO_Pin_8);
			GPIO_ResetBits(GPIOA, GPIO_Pin_15);
		}break;
		case MODE_LOWPWR:
		{
			GPIO_ResetBits(GPIOA, GPIO_Pin_8);
			GPIO_SetBits(GPIOA, GPIO_Pin_15);
		}break;
		case MODE_WAKEUP:
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_8);
			GPIO_ResetBits(GPIOA, GPIO_Pin_15);
		}break;
		case MODE_SLEEP:
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_8);
			GPIO_SetBits(GPIOA, GPIO_Pin_15);
		}break;
		default:
		{
			
		}break;
	}
}
void BSP_E32_ArgsSet(uint16_t DeviceAdd)
{
	RTE_Printf("[E32]    准备配置模块...\r\n");
	uint8_t Data[6] = {0};
	Data[0] = 0xC0;
	Data[1] = (DeviceAdd>>8)&0xFF;
	Data[2] = (DeviceAdd)&0xFF;
	RTE_Printf("[E32]    待配置地址：%02x%02x\r\n",Data[1],Data[2]);
	Data[3] = 0x1A;//9600,2.4K,8N1
	Data[4] = 0x17;//433M,00H-1FH，对应410~441MHz
	Data[5] = 0x44;//
	BSP_COM_SendArray(COM_3,Data,6);
	while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)!=true)
	{
		
	}
	RTE_Printf("[E32]    模块配置成功...\r\n");
}
void BSP_E32_Send(uint8_t *Data,uint16_t Length)
{
	if(Length>=512)
		return;
	else
	{
		while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)!=true)
		{
			
		}
		BSP_COM_SendArray(COM_3,Data,Length);
		while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)!=true)
		{
			
		}
	}
}
