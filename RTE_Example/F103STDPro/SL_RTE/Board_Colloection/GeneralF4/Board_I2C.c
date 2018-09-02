#include "Board_I2C.h"
#include "APP_SoftTimer.h"
void SoftI2C_Init(void)
{ 
	GPIO_InitTypeDef GPIO_InitStruct;
#ifdef STM32F1
	RCC_APB2PeriphClockCmd( I2C_GPIOCLK, ENABLE); 
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = I2C_SDAPin | I2C_SCLPin ;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct); 
#endif
	RCC_AHB1PeriphClockCmd( I2C_GPIOCLK, ENABLE); 
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;          //普通输出模式
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;         //推挽输出
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;     //100MHz
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;           //上拉
	GPIO_InitStruct.GPIO_Pin = I2C_SDAPin | I2C_SCLPin ;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct); 
}
static void SoftI2C_SDASetOut(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
#ifdef STM32F1
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = I2C_SDAPin;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct);  
#endif
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;          //普通输出模式
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;         //推挽输出
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;     //100MHz
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;           //上拉
	GPIO_InitStruct.GPIO_Pin = I2C_SDAPin;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct); 
}
static void SoftI2C_SDASetIn(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
#ifdef STM32F1
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = I2C_SDAPin;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct);
#endif
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;          //普通输出模式
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;     //100MHz
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;           //上拉
	GPIO_InitStruct.GPIO_Pin = I2C_SDAPin;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct); 
}
void SoftI2C_StartSignal(void)
{
	SoftI2C_SDASetOut();
	SoftI2C_SDASetHigh();  
	SoftI2C_SCLSetHigh(); 
	APP_SoftTimer_DelayUS(4);  
	SoftI2C_SDASetLow();        
	APP_SoftTimer_DelayUS(4);    
	SoftI2C_SCLSetLow();   
}
void SoftI2C_StopSignal(void)
{
	SoftI2C_SDASetOut();
	SoftI2C_SCLSetLow();    
	SoftI2C_SDASetLow(); 	
	APP_SoftTimer_DelayUS(4);               
	SoftI2C_SCLSetHigh();  
	SoftI2C_SDASetHigh(); 
	APP_SoftTimer_DelayUS(4); 
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功

uint8_t SoftI2C_WaitAck(void)
{
	SoftI2C_SDASetIn();      //SDA设置为输入  
	SoftI2C_SDASetHigh();
	APP_SoftTimer_DelayUS(1);	   
	SoftI2C_SCLSetHigh();
	APP_SoftTimer_DelayUS(1);	 
	uint8_t ucErrTime=0;
	while(SoftI2C_SDARead())
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			SoftI2C_StopSignal();
			return 1;
		}
	}
	SoftI2C_SCLSetLow();//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
static void SoftI2C_Ack(void)
{
	SoftI2C_SCLSetLow();
	SoftI2C_SDASetOut();
	SoftI2C_SDASetLow();
	APP_SoftTimer_DelayUS(2);
	SoftI2C_SCLSetHigh();
	APP_SoftTimer_DelayUS(2);
	SoftI2C_SCLSetLow();
}
//不产生ACK应答		    
static void SoftI2C_NAck(void)
{
	SoftI2C_SCLSetLow();
	SoftI2C_SDASetOut();
	SoftI2C_SDASetHigh();
	APP_SoftTimer_DelayUS(2);
	SoftI2C_SCLSetHigh();
	APP_SoftTimer_DelayUS(2);
	SoftI2C_SCLSetLow();
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void SoftI2C_SendByte(uint8_t txd)
{                        
	uint8_t t;   
	SoftI2C_SDASetOut(); 	    
	SoftI2C_SCLSetLow();//拉低时钟开始数据传输
	for(t=0;t<8;t++)
	{              
		if(((txd&0x80)>>7))
			SoftI2C_SDASetHigh();
		else
			SoftI2C_SDASetLow();
		txd<<=1; 	  
		APP_SoftTimer_DelayUS(2);   //对TEA5767这三个延时都是必须的
		SoftI2C_SCLSetHigh();
		APP_SoftTimer_DelayUS(2); 
		SoftI2C_SCLSetLow();	
		APP_SoftTimer_DelayUS(2);
	}	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8_t SoftI2C_ReadByte(uint8_t ack)
{
	uint8_t i,receive=0;
	SoftI2C_SDASetIn();//SDA设置为输入
  for(i=0;i<8;i++ )
	{
		SoftI2C_SCLSetLow(); 
		APP_SoftTimer_DelayUS(2);
		SoftI2C_SCLSetHigh();
		receive<<=1;
		if(SoftI2C_SDARead())receive++;   
		APP_SoftTimer_DelayUS(1); 
	}					 
	if (!ack)
		SoftI2C_NAck();//发送nACK
	else
		SoftI2C_Ack(); //发送ACK   
	return receive;
}
