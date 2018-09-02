#ifndef __BOARD_SOFT_COM_H
#define __BOARD_SOFT_COM_H
#include "APP_INCLUDE.h"
#include "BSP_INCLUDE.h"
#include "APP_RingBuffer.h"
#define SOFTCOM_PORT        GPIOB
#if USE_SOFTCOMRX == 1
#define SOFTCOM_RXPIN       GPIO_Pin_8
#define SOFTCOM_RXINTERLINE EXTI_Line8
#define SOFTCOM_PORTSOURCR  GPIO_PortSourceGPIOB 
#define SOFTCOM_PINSOURCR   GPIO_PinSource8 
#define SOFTCOM_IRQ         EXTI9_5_IRQn 
typedef struct
{
	//结构体变量
	APP_RingBuffer_Data_t    ComData;    //串口数据缓存
	APP_RingBuffer_Quene_t   ComQuene;  //串口数据环形队列
}SoftCom_Data_t;
#endif
#if USE_SOFTCOMTX == 1
#define SOFTCOM_TXPIN 
#endif
#define SOFTCOM_GPIOCLK     RCC_APB2Periph_GPIOB
typedef enum
{
	BuadRate_9600_TIM = 107,
	BuadRate_4800_TIM = 209,
	BuadRate_2400_TIM =	419,
	BuadRate_1200_TIM =	839,
	BuadRate_600_TIM = 1679,
}SoftComBaud_e;
extern SoftCom_Data_t SoftComData;
void Board_SoftComInit(SoftComBaud_e BuadRateCalForTimer);
#endif

