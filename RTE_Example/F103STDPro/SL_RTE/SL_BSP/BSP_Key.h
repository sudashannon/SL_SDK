/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_KEY_H
#define __APP_KEY_H
/* Includes ------------------------------------------------------------------*/
#include "APP_Include.h"
#include "BSP_Include.h"
/* Exported types ------------------------------------------------------------*/
//每个按键的ID
typedef enum 
{
  KEY_0 = 0,  
	KEY_1 = 1,  
	KEY_2 = 2, 
	KEY_3 = 3, 
	KEY_NUM,
}BSP_KEY_NAME_t;
typedef struct
{
	volatile uint8_t Buf[KEY_POOL_SIZE];													/* 键值缓冲区 */
	volatile uint8_t Read;																				/* 缓冲区读指针1 */
	volatile uint8_t Write;																			/* 缓冲区写指针 */
}BSP_KEY_FIFO_t;	
typedef struct																				//每个按键对应1个全局的结构体变量
{																									
	volatile uint8_t  Count;			                                /* 滤波器计数器 */
	volatile uint16_t LongCount;		                              /* 长按计数器 */
	volatile uint16_t LongTime;		                              	/* 按键按下持续时间, 0表示不检测长按 */
	volatile uint8_t  State;			                                /* 按键当前状态（按下还是弹起） */
	volatile uint8_t  RepeatSpeed;	                              /* 连续按键周期 */
	volatile uint8_t  RepeatCount;	                              /* 连续按键计数器 */
}BSP_KEY_SoftHandle_t;
typedef struct {
	BSP_KEY_NAME_t KEY_NAME;    // Name
  GPIO_TypeDef* KEY_PORT; // Port
  const uint16_t KEY_PIN; // Pin
	const uint32_t KEY_CLK; // Clock
	const uint8_t  KEY_PRESS;
	BSP_KEY_SoftHandle_t *KEY_ARG;
}BSP_KEY_HardHandle_t;
//每个按键的键值
typedef enum                                          //定义键值代码, 必须按如下次序定时每个键的按下、弹起和长按事件
{
	KEY_NONE = 0,			                                  //0 表示无按键事件
	
	KEY_1_DOWN,				                                  /* 1键按下 */
	KEY_1_UP,				                                    /* 1键弹起 */
	KEY_1_LONG,				                                  /* 1键长按 */
	
	KEY_2_DOWN,				                                  /* 2键按下 */
	KEY_2_UP,					                                  /* 2键弹起 */
	KEY_2_LONG,				                                  /* 2键长按 */

	KEY_3_DOWN,				                                  /* 3键按下 */
	KEY_3_UP,					                                  /* 3键弹起 */
	KEY_3_LONG,				                                  /* 3键长按 */

	KEY_4_DOWN,				                                  /* 4键按下 */
	KEY_4_UP,					                                  /* 4键弹起 */
	KEY_4_LONG,				                                  /* 4键长按 */

	KEY_5_DOWN,				                                  /* 5键按下 */
	KEY_5_UP,					                                  /* 5键弹起 */
	KEY_5_LONG,				                                  /* 5键长按 */
	
}BSP_KEY_State_t;
extern void BSP_KEY_Init(void);
extern BSP_KEY_State_t BSP_KEY_Get(void);
extern void BSP_KEY_Poll(void);
#endif
/****************** (C) COPYRIGHT SuChow University Shannon*****END OF FILE****/
