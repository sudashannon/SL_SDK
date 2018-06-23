#include "Board_E2PROM.h"
#include "Board_I2C.h"
#include "APP_SoftTimer.h"
/*******************************************************************************
*************************以下为EEPROM24C02读写操作******************************
*******************************************************************************/
//初始化24c02的IIC接口
void AT24C02_Init(void)
{
	SoftI2C_Init();  //IIC初始化
}
/****************************************************************************
* 名    称: uint8_t AT24C02_ReadByte(uint8_t ReadAddr)
* 功    能：在AT24C02指定地址读出一个数据
* 入口参数：ReadAddr：要读取数据所在的地址
* 返回参数：读到的8位数据
* 说    明：  
****************************************************************************/
uint8_t AT24C02_ReadByte(uint8_t ReadAddr)
{				  
	uint8_t receivedata=0;		  	    																 
  
	SoftI2C_StartSignal();  
	SoftI2C_SendByte(0XA0);           //发送器件地址0XA0
	SoftI2C_WaitAck();
  SoftI2C_SendByte(ReadAddr);       //发送低地址
	SoftI2C_WaitAck();	    
	SoftI2C_StartSignal();  	 	   
	SoftI2C_SendByte(0XA1);           //进入接收模式			   
	SoftI2C_WaitAck();	 
  receivedata=SoftI2C_ReadByte(0);		   
  SoftI2C_StopSignal();                    //产生一个停止条件	    
	
	return receivedata;
}
/****************************************************************************
* 名    称: uint8_t AT24C02_WriteByte(uint8_t WriteAddr,uint8_t WriteData)
* 功    能：在AT24C02指定地址写入一个数据
* 入口参数：WriteAddr：要写入数据所在的地址
            WriteData: 要写入的数据
* 返回参数： 
* 说    明：  
****************************************************************************/
void AT24C02_WriteByte(uint8_t WriteAddr,uint8_t WriteData)
{				   	  	    																 
  SoftI2C_StartSignal();  
	SoftI2C_SendByte(0XA0);       //发送0XA0,写数据 	 
	SoftI2C_WaitAck();	   
  SoftI2C_SendByte(WriteAddr);  //发送低地址
	SoftI2C_WaitAck(); 	 										  		   
	SoftI2C_SendByte(WriteData);  //发送字节							   
	SoftI2C_WaitAck();  		    	   
  SoftI2C_StopSignal();                    //产生一个停止条件 
	APP_SoftTimer_DelayMS(10);	 
}
/****************************************************************************
* 名    称: uint8_t AT24C02_Test(void)
* 功    能：测试AT24C02是否正常
* 入口参数：无
* 返回参数：返回1:检测失败
            返回0:检测成功 
* 说    明：  
****************************************************************************/
uint8_t AT24C02_Test(void)
{
	uint8_t Testdata;
	Testdata=AT24C02_ReadByte(255); //如果开机测试，已有值无需再次写入	   
	if(Testdata==0XAB)return 0;		   
	else                             
	{
		AT24C02_WriteByte(255,0XAB);
	  Testdata=AT24C02_ReadByte(255);	  
		if(Testdata==0XAB)return 0;
	}
	return 1;											  
}
/****************************************************************************
* 名    称: uint32_t Buf_4Byte(uint8_t *pBuffer,uint32_t Date_4Byte,uint8_t Byte_num,uint8_t mode)
* 功    能：多位数与字节互转
* 入口参数：mode：1:多位数转分成字节   0:字节合并成一个多位数
            Byte_num：需要转化的字节数
            *pBuffer：字节接收数组或字节所在数组
            Date_4Byte：多位数数
* 返回参数：mode为0时，返回多位数
* 说    明：Byte_num最大为4个字节，该函数在后面的触摸屏校正时存取校正值所用到
****************************************************************************/
uint32_t Buf_4Byte(uint8_t *pBuffer,uint32_t Date_4Byte,uint8_t Byte_num,uint8_t mode)
{
  uint8_t i; uint32_t middata=0;
	if(mode)    //多位数转分成字节
	 {
	   for(i=0;i<Byte_num;i++)
	     {
	       *pBuffer++ =(Date_4Byte>>(8*i))&0xff;
	     }
			return 0; 
	 } 
	 else       //字节合并成一个多位数
	 {
	    Date_4Byte=0;
		  pBuffer+=(Byte_num-1);
		  for(i=0;i<Byte_num;i++)
	      { 		
		      middata<<=8;
		      middata+= *pBuffer--;			   
	      }
			return middata;	
	 }
}
/****************************************************************************
* 名    称: void AT24C02_Read(uint8_t ReadAddr,uint8_t *pBuffer,uint8_t ReadNum)
* 功    能：从AT24C02里面的指定地址开始读出指定个数的数据
* 入口参数：ReadAddr :开始读出的地址  0~255
            pBuffer  :数据数组首地址
            ReadNum:要读出数据的个数
* 返回参数：
* 说    明：  
****************************************************************************/
void AT24C02_Read(uint8_t ReadAddr,uint8_t *pBuffer,uint8_t ReadNum)
{
	while(ReadNum--)
	{
		*pBuffer++=AT24C02_ReadByte(ReadAddr++);	
	}
} 
/****************************************************************************
* 名    称: void AT24C02_Write(uint8_t WriteAddr,uint8_t *pBuffer,uint8_t WriteNum)
* 功    能：从AT24C02里面的指定地址开始写入指定个数的数据
* 入口参数：WriteAddr :开始写入的地址  0~255
            pBuffer  :数据数组首地址
            WriteNum:要写入数据的个数
* 返回参数：
* 说    明：  
****************************************************************************/
void AT24C02_Write(uint8_t WriteAddr,uint8_t *pBuffer,uint8_t WriteNum)
{
	while(WriteNum--)
	{
		AT24C02_WriteByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}
