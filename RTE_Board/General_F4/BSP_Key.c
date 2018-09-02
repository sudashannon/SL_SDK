#include "BSP_Key.h"
static uint8_t BSP_KEY_Press(BSP_KEY_NAME_t KEY_NAME);
static BSP_KEY_SoftHandle_t KeyArg[KEY_NUM] = {
	{KEY_FILTER_TIME / 2, 0 , KEY_LONG_TIME , 0 , 0 , 0},
	{KEY_FILTER_TIME / 2, 0 , KEY_LONG_TIME , 0 , 0 , 0},
	{KEY_FILTER_TIME / 2, 0 , KEY_LONG_TIME , 0 , 0 , 0},
	{KEY_FILTER_TIME / 2, 0 , KEY_LONG_TIME , 0 , 0 , 0},
};
static BSP_KEY_HardHandle_t Key[KEY_NUM] = {
  // Name    ,PORT , PIN  , PressState ,Arg 
  {KEY_0 , GPIOF  , GPIO_Pin_9 ,RCC_AHB1Periph_GPIOF,  0  , 0},
	{KEY_1 , GPIOF  , GPIO_Pin_8 ,RCC_AHB1Periph_GPIOF,  0  , 0},
	{KEY_2 , GPIOF  , GPIO_Pin_7 ,RCC_AHB1Periph_GPIOF,  0  , 0},
	{KEY_3 , GPIOF  , GPIO_Pin_6 ,RCC_AHB1Periph_GPIOF,  0  , 0},
};
volatile BSP_KEY_FIFO_t KEY_StatePool;		/* 按键FIFO变量,结构体 */
/*
*********************************************************************************************************
*	函 数 名: BSP_BSP_PressKey
*	功能说明: 判断按键是否按下。
*	形    参:  KEY_NAME : 按键代码
*	返 回 值: 1或0
*********************************************************************************************************
*/
static uint8_t BSP_KEY_Press(BSP_KEY_NAME_t KEY_NAME)
{
	if (GPIO_ReadInputDataBit(Key[KEY_NAME].KEY_PORT, Key[KEY_NAME].KEY_PIN) == Key[KEY_NAME].KEY_PRESS)
	{
		return 1;
	}
	else
		return 0;
}
/*
*********************************************************************************************************
*	函 数 名: BSP_BSP_InitKey
*	功能说明: 初始化按键硬件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void BSP_KEY_InitvaluePool(void);
void BSP_KEY_Init(void)
{
	BSP_KEY_InitvaluePool();
  BSP_KEY_NAME_t key_name;
	for(key_name=KEY_0;key_name<KEY_NUM;key_name++)
	{
	  GPIO_InitTypeDef  GPIO_InitStructure;
#ifdef STM32F1
    // Clock Enable
    RCC_APB2PeriphClockCmd(Key[key_name].KEY_CLK, ENABLE);
    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = Key[key_name].KEY_PIN;
		if(key_name == KEY_3)
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
		else
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Key[key_name].KEY_PORT, &GPIO_InitStructure);		
#endif
#ifdef STM32F4
    // Clock Enable
    RCC_AHB1PeriphClockCmd(Key[key_name].KEY_CLK, ENABLE);
    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = Key[key_name].KEY_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;            
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;       
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       
    GPIO_Init(Key[key_name].KEY_PORT, &GPIO_InitStructure);		
#endif
		if(&KeyArg[key_name]==(void *)0)
			APP_Assert((char *)__FILE__, __LINE__);
		Key[key_name].KEY_ARG = (BSP_KEY_SoftHandle_t *)&KeyArg[key_name];
	}
}
/*
*********************************************************************************************************
*	函 数 名: BSP_BSP_PutKey
*	功能说明: 将1个键值压入按键FIFO缓冲区。可用于模拟一个按键。
*	形    参:  _KeyCode : 按键代码
*	返 回 值: 无
*********************************************************************************************************
*/
static void BSP_KEY_Put(uint8_t _KeyCode)
{
	KEY_StatePool.Buf[KEY_StatePool.Write] = _KeyCode;
	if (++KEY_StatePool.Write  >= KEY_POOL_SIZE)
	{
		KEY_StatePool.Write = 0;
	}
}
/*
*********************************************************************************************************
*	函 数 名: BSP_BSP_GetKey
*	功能说明: 从按键FIFO缓冲区读取一个键值。
*	形    参:  无
*	返 回 值: 按键代码
*********************************************************************************************************
*/
BSP_KEY_State_t BSP_KEY_Get(void)
{
	BSP_KEY_State_t ret;
	
	if (KEY_StatePool.Read == KEY_StatePool.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = (BSP_KEY_State_t)KEY_StatePool.Buf[KEY_StatePool.Read];

		if (++KEY_StatePool.Read >= KEY_POOL_SIZE)
		{
			KEY_StatePool.Read = 0;
		}
		return ret;
	}
}

/**************************************************************************************
* FunctionName   : BSP_BSP_InitKeyvaluePoll()
* Description    : 键值缓冲池初始化
* EntryParameter : None
* ReturnValue    : None
**************************************************************************************/
void BSP_KEY_InitvaluePool(void)
{
	/* 对按键FIFO读写指针清零 */
	memset((void *)KEY_StatePool.Buf,0,KEY_POOL_SIZE);
	KEY_StatePool.Read = 0;
	KEY_StatePool.Write = 0;
}
/*
*********************************************************************************************************
*	函 数 名: BSP_BSP_KeyPoll
*	功能说明: 检测按键序列。非阻塞状态，必须被周期性的调用。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void BSP_KEY_Poll(void)
{
	BSP_KEY_NAME_t i;
	for (i = KEY_0; i < KEY_NUM ; i++)
	{
		//如果没有初始化按键函数，跳过此按键检测
		if (BSP_KEY_Press(i))
		{
			if (Key[i].KEY_ARG->Count < KEY_FILTER_TIME)
			{
				Key[i].KEY_ARG->Count = KEY_FILTER_TIME;
			}
			else if(Key[i].KEY_ARG->Count < 2 * KEY_FILTER_TIME)
			{
				Key[i].KEY_ARG->Count++;
			}
			else
			{
				if (Key[i].KEY_ARG->State == 0)
				{
					Key[i].KEY_ARG->State = 1;

					/* 发送按钮按下的消息 */
					BSP_KEY_Put((uint8_t)(3 * i + 1));
				}

				if (Key[i].KEY_ARG->LongTime > 0)
				{
					if (Key[i].KEY_ARG->LongCount < Key[i].KEY_ARG->LongTime)
					{
						/* 发送按钮持续按下的消息 */
						if (++Key[i].KEY_ARG->LongCount == Key[i].KEY_ARG->LongTime)
						{
							/* 键值放入按键FIFO */
							BSP_KEY_Put((uint8_t)(3 * i + 3));
						}
					}
					else
					{
						if (Key[i].KEY_ARG->RepeatSpeed > 0)
						{
							if (++Key[i].KEY_ARG->RepeatCount >= Key[i].KEY_ARG->RepeatSpeed)
							{
								Key[i].KEY_ARG->RepeatCount = 0;
								/* 常按键后，每隔10ms发送1个按键 */
								BSP_KEY_Put((uint8_t)(3 * i + 3));
							}
						}
					}
				}
			}
		}
		else
		{
			if(Key[i].KEY_ARG->Count > KEY_FILTER_TIME)
			{
				Key[i].KEY_ARG->Count = KEY_FILTER_TIME;
			}
			else if(Key[i].KEY_ARG->Count != 0)
			{
				Key[i].KEY_ARG->Count--;
			}
			else
			{
				if (Key[i].KEY_ARG->State == 1)
				{
					Key[i].KEY_ARG->State = 0;

					/* 发送按钮弹起的消息 */
					BSP_KEY_Put((uint8_t)(3 * i + 2));
				}
			}

			Key[i].KEY_ARG->LongCount = 0;
			Key[i].KEY_ARG->RepeatCount = 0;
		}
	}
}

