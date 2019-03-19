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
#define RTE_VERSION "3.7.0319"
//=======================
//<e> 动态内存管理模块
//=======================
//<i> 使用RTE自带的动态内存管理，依赖模块：无。
#define RTE_USE_MEMMANAGE             1
#if RTE_USE_MEMMANAGE == 1
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
	//<q>使用片上flash
	#define USE_ONCHIP_FLASH            0
	//<q>环境变量功能
	#define EF_USING_ENV                1
	//<q>环境变量自动更新功能
	//<i>Auto update ENV to latest default when current ENV version number is changed.
	#define EF_ENV_AUTO_UPDATE          0
	#if EF_ENV_AUTO_UPDATE
	//<o>环境变量版本
	#define EF_ENV_VER_NUM              0
	#endif
	//<q>IAT功能
	#define EF_USING_IAP                0
	//<o>最小擦除大小
	//<i>单位：K
	#if USE_ONCHIP_FLASH == 0
	#define EF_ERASE_MIN_SIZE          4096
	#else
	#define EF_ERASE_MIN_SIZE          (32 * 1024)
	#endif
	//<o>FLASH写入单元
	//<i>support 1(nor flash)/ 8(stm32f4)/ 32(stm32f1)/ 64(stm32l4)
	#if USE_ONCHIP_FLASH == 0
	#define EF_WRITE_GRAN              1
	#else
	#define EF_WRITE_GRAN              8
	#endif
	//<o>可用FLASH首地址
	#if USE_ONCHIP_FLASH == 0
	#define EF_START_ADDR              (0)
	#else
	#define EF_START_ADDR              0x08000000 + 480 * 1024
	#endif
	//<o>环境变量大小
	//<i>至少两个扇区大小用于GC
	#define ENV_AREA_SIZE             (16 * EF_ERASE_MIN_SIZE)      

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
	//<o>最大支持模组数目
	//<i>默认大小: 16
	#define SHELL_MAX_MODULE    		    16
	//<o>模组最大函数数目
	//<i>默认大小: 16
	#define SHELL_MAX_MODULE_FUC    		16
	//<o>函数最大支持参数量
	//<i>默认大小: 8
	#define SHELL_MAX_ARGS                  8
	//<o>数据缓存大小
	//<i>默认大小: 128 [bytes]
	#define SHELL_BUFSIZE    		        128
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
