#include "Board_DHT11.h"
#include "APP_SoftTimer.h"
DHT11Result_t DHT11Result;
static void DHT11_DataPinSetOutput(void)  
{  
	GPIO_InitTypeDef  GPIO_InitStructure;  
	RCC_APB2PeriphClockCmd(DHT11_GPIOCLK, ENABLE);     //使能PC端口时钟  
	GPIO_InitStructure.GPIO_Pin = DHT11_DataPin;                 //PC.0 端口配置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;          //推挽输出  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(DHT11_GPIO, &GPIO_InitStructure);  
}  
static void DHT11_DataPinSetInput(void)  
{       
	GPIO_InitTypeDef  GPIO_InitStructure;  
	RCC_APB2PeriphClockCmd(DHT11_GPIOCLK, ENABLE);     //使能PC端口时钟  
	GPIO_InitStructure.GPIO_Pin = DHT11_DataPin;                 //PC.0 端口配置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;          //推挽输出  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(DHT11_GPIO, &GPIO_InitStructure);  
} 
static bool DHT11_GetDataBit(void)  
{  
	uint8_t val;  
	val = GPIO_ReadInputDataBit(DHT11_GPIO, DHT11_DataPin);  
	if(val == Bit_RESET){  
		return false;  
	}else{  
		return true;  
	}  
}  
static void DHT11_SetDataBit(bool level)  
{  
	if(level == true){  
		GPIO_SetBits(DHT11_GPIO, DHT11_DataPin);  
	}else{  
		GPIO_ResetBits(DHT11_GPIO, DHT11_DataPin);  
	}  
} 
static uint8_t DHT11_ReadByte(void)  
{       
	uint8_t i;    
	uint8_t data = 0;      						 
	for(i = 0; i < 8; i++)         
	{      
		data <<= 1;     
		APP_SoftTimer_DelayUS(20);  		
		while((!DHT11_GetDataBit()));  
		APP_SoftTimer_DelayUS(25);   
		if(DHT11_GetDataBit())    {  
			data |= 0x01;  
			APP_SoftTimer_DelayUS(30);     
		} else{  
			data %= 0xFE;  
		}   
	 }        
	 return data;  
}  
static uint8_t DHT11_StartSample(void)  
{  
	DHT11_DataPinSetOutput();  
	//主机拉低18ms     
	DHT11_SetDataBit(false);  
	APP_SoftTimer_DelayMS(18);  
	DHT11_SetDataBit(true);  
	//总线由上拉电阻拉高 主机延时20us  
	APP_SoftTimer_DelayUS(20);  
	//主机设为输入 判断从机响应信号   
	DHT11_DataPinSetInput();  
	 //判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行        
	if(!DHT11_GetDataBit())         //T !        
	{  
	 //判断从机是否发出 80us 的低电平响应信号是否结束       
		 while((!DHT11_GetDataBit()));  
	 //判断从机是否发出 80us 的高电平，如发出则进入数据接收状态  
		 while((DHT11_GetDataBit()));  
		 return 1;  
	}  
	return 0;                 
} 
void DHT11_GetData(void)  
{        
	uint8_t CheckTemp,TemperHigh8Temp,TemperLow8Temp,HumHigh8Temp,HumLow8Temp;
	if(DHT11_StartSample())
	{  
		//数据接收状态           
		HumHigh8Temp = DHT11_ReadByte();  
		HumLow8Temp = DHT11_ReadByte();  
		TemperHigh8Temp = DHT11_ReadByte();  
		TemperLow8Temp = DHT11_ReadByte();  
		CheckTemp = DHT11_ReadByte();  
		/* Data transmission finishes, pull the bus high */  
		DHT11_DataPinSetOutput();     
		DHT11_SetDataBit(true);  
		//数据校验   
		DHT11Result.CheckData=(HumHigh8Temp+HumLow8Temp+TemperHigh8Temp+TemperLow8Temp);  
		if(CheckTemp == DHT11Result.CheckData)  
		{  
			DHT11Result.HumdityHigh8 = HumHigh8Temp;  
			DHT11Result.HumdityLow8 = HumLow8Temp;  
			DHT11Result.TemperHigh8 = TemperHigh8Temp;  
			DHT11Result.TemperLow8 = TemperLow8Temp;  
		}
	}
}
