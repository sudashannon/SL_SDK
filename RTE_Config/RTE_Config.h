/**
  ******************************************************************************
  * @file    RTE_Config.h
  * @author  Shan Lei ->>lurenjia.tech ->>https://github.com/sudashannon
  * @brief   RTE环境配置头文件，按照KEIL格式编写，为RTE环境进行裁剪配置。
  * @version V2.0 2019/06/06 第二版
						 V1.0 2018/11/02 第一版
  ******************************************************************************
  */
#ifndef __RTE_CONFIG_H
#define __RTE_CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif
//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//=======================
//  <h> RTE基本配置
//=======================
//      <s> RTE版本号
//      <i> 具体定义见相关文档。
#define RTE_VERSION          "4.0.0606"
//      <q> 实时操作系统环境
//      <i> 默认是RTX5
#define RTE_USE_OS                 1
//=======================
//  </h> RTE基本配置
//=======================
//=======================
//  <e> RTE标准输出模块
//=======================
//  <i> 提供printf输出函数实现，依赖模块：无。
#define RTE_USE_PRINTF             1
#if RTE_USE_PRINTF == 1
// 		<o> ntoa缓存大小
// 		<i> 'ntoa'功能使用的缓存，创建在栈上。默认大小: 32 byte。
#define PRINTF_NTOA_BUFFER_SIZE    32U
// 		<o> ftoa缓存大小
// 		<i> 'ftoa'功能使用的缓存，创建在栈上。默认大小: 32 byte。
#define PRINTF_FTOA_BUFFER_SIZE    32U
// 		<e> %f支持
// 		<i> 使能%f格式输出
#define PRINTF_SUPPORT_FLOAT       1
#if PRINTF_SUPPORT_FLOAT == 1
// 		<o> %f输出数据精度
// 		<i> define the default floating point precision default: 6 digits
#define PRINTF_DEFAULT_FLOAT_PRECISION  6U
// 		<o> %f输出最大数据
// 		<i> define the largest float suitable to print with %f default: 1e9
#define PRINTF_MAX_FLOAT           1e9
#endif
//      </e> %f支持
// 		<q> %e/%g支持
// 		<i> 使能%e/%g格式输出
#define PRINTF_SUPPORT_EXPONENTIAL 1
// 		<q> %llu/%p支持
// 		<i> support for the long long types (%llu or %p) default: activated
#define PRINTF_SUPPORT_LONG_LONG   1
// 		<q> %t支持
// 		<i> support for the ptrdiff_t type (%t) ptrdiff_t is normally defined in <stddef.h> as long or long long type
// 		<i> default: activated
#define PRINTF_SUPPORT_PTRDIFF_T   1
#endif
//=======================
//  </e> RTE标准输出模块
//=======================
//=======================
//  <e> RTE动态内存模块
//=======================
//  <i> 使用RTE自带的动态内存管理，依赖模块：无。
#define RTE_USE_MEMORY             1
#if RTE_USE_MEMORY == 1
// 		<o> 动态内存管理类型 <0=>LVGL <1=>TLSF
// 		<i> 设置动态内存管理的类型
// 		<i>                  LVGL（边界标定法，资源消耗少，碎片多）
// 		<i>                  TLSF（TLSF法，资源消耗大，碎片少）
#define MEMORY_TYPE                1
// 		<q> 动态内存模块调试信息
// 		<i> 使能动态内存模块调试信息输出
#define MEMORY_DEBUG               1
//      <o> RTE内存块大小
//      <i> RTE使用得动态内存大小
//      <i> 默认大小: 32（单位：K）
#define RTE_MEM_SIZE    		        (32U)
#if MEMORY_TYPE == 0
//      <h> LVGL类型下参数配置
// 			<q> 64位处理器使能
#define  MEMORY_SIMPLE_64BIT       0
// 			<q> 自动碎片整理
#define  MEMORY_SIMPLE_AUTODEFRAG  1
//      </h> 简单模式
#endif
#endif
//=======================
//  </e> RTE动态内存模块
//=======================
//=======================
//  <e> RTE动态数组模块
//=======================
//  <i> 使用RTE自带的动态数组管理，依赖模块：动态内存模块。
#define RTE_USE_VEC                1
#if RTE_USE_VEC == 1
#if RTE_USE_MEMORY == 0
#error "需要动态内存模块的支持"
#endif
#endif
//=======================
//  </e> RTE动态数组模块
//=======================
//=======================
//  <e> RTE时间片轮询模块
//=======================
//  <i> 裸机或操作系统环境下的时间片轮转调度，依赖模块：SIMPLE模式无依赖，其余模式依赖vec模块，动态内存模块。
#define RTE_USE_ROUNDROBIN            1
#if RTE_USE_ROUNDROBIN == 1
// 		<o> 时间片轮询类型 <0=>SIMPLE <1=>BASE <2=>FULL
// 		<i> 设置时间片轮询的类型
// 		<i>                SIMPLE（简单时间片轮询，静态管理，需要手动维护RR头文件中的定时器）
// 		<i>                BASE（基础时间片轮询，动态管理，不分组，定时器名称由用户维护）
// 		<i>                FULL（分组时间片轮询，动态管理，适合多线程环境，组名称和定时器名称由用户维护）
#define RR_TYPE                       2   
// 		<q> 时间片轮询调试信息
// 		<i> 使能时间片轮询调试信息输出
#define RR_DEBUG                      1
#if RR_TYPE == 0

#elif RR_TYPE == 1
#if RTE_USE_VEC == 0 || RTE_USE_MEMORY == 0
#error "需要VEC模块以及动态内存模块的支持"
#endif
			// <o> BASE模式下轮转调度最大支持的Timer数目
			// <i> 默认大小: 16
			#define RR_MAX_NUM    		  16
#elif RR_TYPE == 2
#if RTE_USE_VEC == 0 || RTE_USE_MEMORY == 0
#error "需要VEC模块以及动态内存模块的支持"
#endif
			// <o> FULL模式下轮转调度最大支持的Timer数目
			// <i> 默认大小: 16
			#define RR_MAX_NUM    		  16
			// <o> FULL模式下轮转调度最大支持的Group数目
			// <i> 默认大小: 4
			#define RR_MAX_GROUP_NUM      4
#endif
#endif
//=======================
//  </e> RTE时间片轮询模块
//=======================
//=======================
//  <e> 双向链表模块
//=======================
//  <i> 使用RTE自带的双向链表模块，依赖模块：动态内存模块。
#define RTE_USE_LL                    1
#if RTE_USE_LL == 1
#if RTE_USE_MEMORY == 0
#error "需要动态内存模块的支持"
#endif
#endif
//=======================
//  </e> 双向链表模块
//=======================
//=======================
//  <e> 数据流模块
//=======================
//  <i> 使用RTE自带的数据流，依赖模块：无（可选动态内存）。
//  <i> 无动态内存模块配合时，需要手动维护Stream头文件中的数据流。
#define RTE_USE_STREAM                1
#if RTE_USE_STREAM == 1
// 		<q> 数据流调试信息
// 		<i> 使能数据流调试信息输出
#define STREAM_DEBUG                      1
#if RTE_USE_MEMORY == 0
			// <o> 默认队列长度
			// <i> 静态管理时每个队列的默认长度
			#define STATIC_QUENE_SIZE         8
			// <o> 默认缓存大小
			// <i> 静态管理时每个队列的默认缓存的大小
			#define STATIC_BUFFER_SIZE        1024
#endif
#endif
//=======================
//  </e> 数据流模块
//=======================
//=======================
//  <e> 状态机模块
//=======================
//  <i> 使用RTE自带的状态机模型，依赖模块：动态数组与动态内存。
#define RTE_USE_SM                    1
#if RTE_USE_SM == 1
#if RTE_USE_VEC == 0 || RTE_USE_MEMORY == 0
#error "需要VEC模块以及动态内存模块的支持"
#endif
#endif
//=======================
//  </e> 状态机模块
//=======================
//=======================
// <e> SHELL模块
//=======================
// <i> 消息交互模块，依赖模块：环形队列模块，VEC模块，动态内存管理。
#define RTE_USE_SHELL                 1
#if RTE_USE_SHELL == 1
#if RTE_USE_VEC == 0 || RTE_USE_MEMORY == 0 || RTE_USE_STREAM == 0
#error "需要VEC模块、动态内存模块以及数据流模块的支持"
#endif
	// <o> 最大支持模组数目
	// <i> 默认大小: 16
	#define SHELL_MAX_MODULE    		    16
	// <o> 模组最大函数数目
	// <i> 默认大小: 16
	#define SHELL_MAX_MODULE_FUC    		16
	// <o> 函数最大支持参数量
	// <i> 默认大小: 8
	#define SHELL_MAX_ARGS                  8
	// <o> 数据缓存大小
	// <i> 默认大小: 32 [bytes]
	#define SHELL_BUF_SIZE    		       64
	// <o> 数据队列大小
	// <i> 默认大小: 4
	#define SHELL_QUENE_SIZE    		        4
#endif
//=======================
//  </e> SHELL模块
//=======================
//=======================
// <e> KV数据库模块
//=======================
// <i> 使用RTE自带的KV关系数据库，依赖模块：标准输出。
#define RTE_USE_KVDB                  1
	// <q> 环境变量自动更新功能
	// <i> Auto update ENV to latest default when current ENV version number is changed.
	#define EF_ENV_AUTO_UPDATE          0
	#if EF_ENV_AUTO_UPDATE
	// <o> 环境变量版本
	#define EF_ENV_VER_NUM              0
	#endif
	// <q> IAP功能
	#define EF_USING_IAP                0
	// <o> 最小擦除大小
	// <i> 单位：K
	#define EF_ERASE_MIN_SIZE          4096 // 片外2048
	// <o> FLASH写入单元
	// <i> support 1(nor flash)/ 8(stm32f4)/ 32(stm32f1)/ 64(stm32l4)
	#define EF_WRITE_GRAN              32
	// <o> 可用FLASH首地址
	#define EF_START_ADDR              (0x08000000 + 250 * 1024) //  片外0
	// <o> 环境变量大小
	// <i> 至少两个扇区大小用于GC
	#define ENV_AREA_SIZE             (4 * EF_ERASE_MIN_SIZE)
//=======================
// </e> KV数据库模块
//=======================
//=======================
//  <e> RTE断言模块
//=======================
#define RTE_USE_ASSERT             1
#if RTE_USE_ASSERT == 1
extern void RTE_Assert(char *file, unsigned int line);
#define RTE_AssertParam(expr)                               {                                       \
																 if(!(expr))                        \
																 {                                  \
																		 RTE_Assert(__FILE__, __LINE__);\
																 }                                  \
															}
#else
#define RTE_AssertParam(expr) ((void)0)
#endif
//=======================
//  </e> RTE断言模块
//=======================
#ifdef __cplusplus
}
#endif
#endif
