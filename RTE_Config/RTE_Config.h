/**
  ******************************************************************************
  * @file    RTE_Config.h
  * @author  Shan Lei ->>lurenjia.tech ->>https://github.com/sudashannon
  * @brief   RTE环境配置头文件，按照KEIL格式编写，为RTE环境进行裁剪配置。
  * @version V1.0 2018/11/02 第一版
  ******************************************************************************
  */
#ifndef __RTE_CONFIG_H
#define __RTE_CONFIG_H
#ifdef __cplusplus  
extern "C" {  
#endif  
//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//<s> RTE版本号
//<i> 具体定义见相关文档。
#define RTE_VERSION "3.3.0108"
//=======================
//<e> 动态内存管理模块
//=======================
//<i> 使用RTE自带的动态内存管理，依赖模块：无。
#define RTE_USE_MEMMANAGE             1
#if RTE_USE_MEMMANAGE == 1
	//<q> 硬件环境是否支持64位
	//<i> 对于STM32等32位MCU，无需勾选此项。
	#define RTE_MEM_64BIT               0
	//<q> 是否自动回收碎片
	//<i> 使能MEM自带的碎片整理功能。
	#define RTE_MEM_AUTO_DEFRAG         1
	//<o> RTE内存块大小
	//<i> RTE使用得动态内存大小
	//<i> 默认大小: 32（单位：K）
	#define RTE_MEM_SIZE    		        (64U)
#endif
//</e>
//=======================
//<e> 动态数组模块
//=======================
//<i> 使用RTE自带的动态数组管理，依赖模块：动态内存模块。
#define RTE_USE_VEC                   1
#if RTE_USE_VEC == 1
#if RTE_USE_MEMMANAGE == 0
#error "This module needs dynatic mem manage module's support!"
#endif
#endif
//</e>
//=======================
//<e> 双向链表模块
//=======================
//<i> 使用RTE自带的双向链表模块，依赖模块：动态内存模块。
#define RTE_USE_LL                    1
#if RTE_USE_LL == 1
#if RTE_USE_MEMMANAGE == 0
#error "This module needs dynatic mem manage module's support!"
#endif
#endif
//</e>
//=======================
//<h> 标准C库模块
//=======================
//<i> 使用RTE自带的替换标准C库的函数，依赖模块：无。
//<q> 字符串处理模块
//<i> 提供一些字符串处理的函数实现。
#define RTE_USE_USTRING               1

//<q> 标准输出模块
//<i> 提供printf输出函数实现，不支持%f。
#define RTE_USE_USTDOUT               1
//</h>
//=======================
//<e> KV数据库模块
//=======================
//<i> 使用RTE自带的KV关系数据库，依赖模块：标准输出。
#define RTE_USE_KVDB                  1
	//<o>KVDB最小擦除单位大小
	//<i>单位：K
	#define KVDB_ERASE_MIN_SIZE         (16 * 1024)
	//<q>掉电保护
	#define KVDB_USE_PFS                0
	//<o>用户设置环境变量大小
	#define KVDB_USER_SETTING_SIZE      2048
	//<o>KVDB地址偏移
	//<i>加上FLASH首地址后为实际地址（单位：K）
	#define KVDB_ADDR_OFFSET            480 * 1024 
	//<o>FLASH首地址
	#define KVDB_FLASH_BASE             0x08000000
	//<e>自动更新（增量更新）
	#define KVDB_USE_AUTO_UPDATE        0
	//<o>固件版本
	//<i>如果检测到产品存储的版本号与设定版本号不一致，会自动追加默认环境变量集合中新增的环境变量。
	#define KVDB_FM_VER_NUM             0
	//</e>
//</e>
//=======================
//<e> 状态机模块
//=======================
//<i> 使用RTE自带的状态机模型，依赖模块：VEC模块，动态内存管理。
#define RTE_USE_SM                    1
#if RTE_USE_SM == 1
#if RTE_USE_VEC == 0 || RTE_USE_MEMMANAGE == 0
#error "This module needs vec and dynatic mem manage module's support!"
#endif
#endif
//</e>
//=======================
//<e> 环形队列模块
//=======================
//<i> 使用RTE自带的环形队列与消息队列，依赖模块：动态内存管理。
#define RTE_USE_RINGQUENE             1
#if RTE_USE_RINGQUENE == 1
#if RTE_USE_MEMMANAGE == 0
#error "This module needs dynatic mem manage module's support!"
#endif
#endif
//</e>
//=======================
//<e> SHELL模块
//=======================
//<i> 消息交互模块，依赖模块：环形队列模块，VEC模块，动态内存管理。
#define RTE_USE_SHELL                 1
#if RTE_USE_SHELL == 1
#if RTE_USE_VEC == 0 || RTE_USE_MEMMANAGE == 0 || RTE_USE_RINGQUENE == 0
#error "This module needs vec and dynatic mem manage module's support!"
#endif
	//<o>最大支持指令数目
	//<i>默认大小: 16
	#define SHELL_MAX_NUM    		        16
	//<o>单条指令最大支持参数量
	//<i>默认大小: 8
	#define SHELL_MAX_ARGS              8
	//<o>数据缓存大小
	//<i>默认大小: 32 [bytes] 
	#define SHELL_BUFSIZE    		        32
#endif
//</e>
//=======================
//<e> 时间片轮转调度模块
//=======================
//<i> 裸机或操作系统环境下的时间片轮转调度，依赖模块：vec模块，动态内存模块。
#define RTE_USE_ROUNDROBIN            1
#if RTE_USE_ROUNDROBIN == 1
#if RTE_USE_VEC == 0 || RTE_USE_MEMMANAGE == 0
#error "This module needs vec and dynatic mem manage module's support!"
#endif
	// <q> 实时操作系统
	// <i> 默认是RTX5
	#ifndef RTE_USE_OS
	#define RTE_USE_OS          1
	#endif
	// <o> 轮转调度最大支持的Timer数目 
	// <i> 默认大小: 16
	#define ROUNDROBIN_MAX_NUM    		  16
	// <o> 轮转调度最大支持的TimerGroup数目
	// <i> 默认大小: 4
	#define ROUNDROBIN_MAX_GROUP_NUM    4
#endif
//</e>
//=======================
//<e> 日志模块
//=======================
//<i> 使用RTE自带的日志模块纪录程序运行信息，可选纪录位置。
#define RTE_USE_LOG                   1
//</e>
//=======================
//<e> 图像处理模块
//=======================
//<i> 使用RTE自带的机器视觉
#define RTE_USE_OPENMV                0
//</e>
//=======================
//<e> GUI模块
//=======================
//<i> 使用RTE自带的GUI
#define RTE_USE_GUI                   1
#if RTE_USE_GUI == 1
#if RTE_USE_MEMMANAGE == 0
#error "This module needs dynatic mem manage module's support!"
#endif
#endif
//</e>
//=======================
//<e> ASSERT
//=======================
#define RTE_USE_ASSERT                   1
#if RTE_USE_ASSERT
#include <stdint.h>
extern void RTE_Assert(char *file, uint32_t line);
#define RTE_AssertParam(expr) {                                     \
																 if(!(expr))                        \
																 {                                  \
																		 RTE_Assert(__FILE__, __LINE__);\
																 }                                  \
															}
#else
#define RTE_AssertParam(expr) ((void)0)
#endif
//</e>
#ifdef __cplusplus  
}  
#endif  
#endif
