/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H
//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
// <e> MEM_USE
// <i> SL_Lib自带的内存管理。
#define USE_MEM                1 
//<o> Global Dynamic Memory size [bytes]
//<i> 用于SL_Lib及应用的动态内存，可由多块组成，每块分配不同大小。
//<i> 默认大小: 4096
#ifndef MEM_BASE_SIZE
#define MEM_BASE_SIZE          1024
#endif
// </e>
// <e> KEY_USE
// <i> SL_Lib自带的缓冲按键。
#define USE_KEY                0 
//<o> Key Pool size [bytes] 
//<i> 按键缓冲池大小。
//<i> 默认大小: 10
#define KEY_POOL_SIZE     		10  
//<o> Key Filter time [10ms] 
//<i> 按键滤波时间，只有连续检测到50ms状态不变才认为有效，包括弹起和按下两种事件。
//<i> 默认大小: 5*10ms
#define KEY_FILTER_TIME       5                  
//<o> Key Long Press time [10ms] 
//<i> 按键长按持续时间，持续1秒，认为长按事件。
//<i> 默认大小: 100*10ms
#define KEY_LONG_TIME         100			            
// </e>
// <e> DEBUG_USE
// <i> SL_Lib自带的串口调试工具。
/* Degug ----------------------------------------------------------*/
#define USE_DEBUG         		1
//<o> Debug Buffer Size [bytes] 
//<i> 用于串口调试的接收缓存的大小。
//<i> 默认大小: 128
#define DEBUG_BUF_SIZE    		32
// </e>
// <e> OS_USE
// <i> SL_Lib是否运行于操作系统环境下。
#define USE_OS            		1
// </e>
// <e> SOFTCOMRX_USE
// <i> 是否使用SL_Lib自带的模拟串口接收功能。
#define USE_SOFTCOMRX         0
// </e>
// <e> SOFTCOMRX_USE
// <i> 是否使用SL_Lib自带的模拟串口发送功能。
#define USE_SOFTCOMTX         0
// </e>
// <e> MPU_USE
// <i> 是否使用MPU功能，仅针对带有MPU的片子。
#define USE_MPU            0
// </e>
// <e> LOWPOWER_USE
// <i> 是否使用低功耗功能，仅针对使用RTX的情况下。
#define USE_LOWPOWER            1
// </e>
#endif
/****************** (C) COPYRIGHT SuChow University Shannon*****END OF FILE****/

