#include "Board_RTC.h"
//RTC时间设置  
//hour,min,sec:小时,分钟,秒钟  
//ampm:@RTC_AM_PM_Definitions  :RTC_H12_AM/RTC_H12_PM  
//返回值:SUCEE(1),成功  
//       ERROR(0),进入初始化模式失败   
ErrorStatus Board_RTC_SetTime(uint8_t hour,uint8_t min,uint8_t sec,uint8_t ampm)  
{      
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;            
	RTC_TimeTypeInitStructure.RTC_Hours=hour;      
	RTC_TimeTypeInitStructure.RTC_Minutes=min;      
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;      
	RTC_TimeTypeInitStructure.RTC_H12=ampm;           
	return RTC_SetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);        
}  
//RTC日期设置  
//year,month,date:年(0~99),月(1~12),日(0~31)  
//week:星期(1~7,0,非法!)  
//返回值:SUCEE(1),成功  
//       ERROR(0),进入初始化模式失败   
ErrorStatus Board_RTC_SetDate(uint8_t year,uint8_t month,uint8_t date,uint8_t week)  
{            
	RTC_DateTypeDef 
	RTC_DateTypeInitStructure;     
	RTC_DateTypeInitStructure.RTC_Date=date;      
	RTC_DateTypeInitStructure.RTC_Month=month;      
	RTC_DateTypeInitStructure.RTC_WeekDay=week;      
	RTC_DateTypeInitStructure.RTC_Year=year;      
	return RTC_SetDate(RTC_Format_BIN,&RTC_DateTypeInitStructure); 
}  
ErrorStatus Board_RTC_Init(void)
{
	RTC_InitTypeDef RTC_InitStructure;      
	uint16_t retry=0X1FFF;       
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//使能PWR时钟      
	PWR_BackupAccessCmd(ENABLE);    //使能后备寄存器访问             
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x5050)     //是否第一次配置?      
	{          
		RCC_LSEConfig(RCC_LSE_ON);//LSE 开启              
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
		{
			//检查指定的RCC标志位,等待低速晶振就绪               
			retry++;              
			RTE_RoundRobin_DelayMS(10);          
		}          
		if(retry==0)
		return ERROR;       //LSE 开启失败.                 
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);     //设置RTC时钟(RTCCLK),选择LSE作为RTC时钟              
		RCC_RTCCLKCmd(ENABLE);  //使能RTC时钟           
		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;//RTC异步分频系数(1~0X7F)          
		RTC_InitStructure.RTC_SynchPrediv  = 0xFF;//RTC同步分频系数(0~7FFF)          
		RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;//RTC设置为,24小时格式          
		RTC_Init(&RTC_InitStructure);            
		Board_RTC_SetTime(23,59,56,RTC_H12_AM);  //设置时间          
		Board_RTC_SetDate(19,1,11,2);     //设置日期                 
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);    //标记已经初始化过了      
	} 
	return SUCCESS;  
}
