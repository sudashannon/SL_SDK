#ifndef __RTE_CONFIG_H
#define __RTE_CONFIG_H
//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//<o>  RTE_VERSION
//<i> RTE版本号，具体定义见相关文档。
#define RTE_VERSION                 "2.1.0826"
// <e> RTE_USE_BGET
// <i> RTE_BGet所管理的动态内存，可由多块组成，每块分配不同大小。
// <i> 在SL_RTE中，RTE_BGet的相关结构体为静态管理方式，其余模块依托于RTE_BGet实现动态管理
#ifndef RTE_USE_BGET
#define RTE_USE_BGET                1 
#endif
// <o> BGET_MEMSIZE
// <i> BGet内存管理的大小
// <i> 默认大小: 1024
#ifndef BGET_MEMSIZE
#define BGET_MEMSIZE              1024
#endif
// </e>
// <e> RTE_USE_STDIO
// <i> c语言标准stdio
// <i> 注意：资源受限的情况下可以考虑不使用
#ifndef RTE_USE_STDIO
#define RTE_USE_STDIO              1
#endif
// </e>
// <e> RTE_USE_STDLIB
// <i> c语言标准stdlib
// <i> 注意：资源受限的情况下可以考虑不使用
#ifndef RTE_USE_STDLIB
#define RTE_USE_STDLIB             1
#endif
// </e>
// <e> RTE_USE_HUMMANINTERFACE
// <i> 人机交互接口
#ifndef RTE_USE_HUMMANINTERFACE
#define RTE_USE_HUMMANINTERFACE   1 
#endif
	// <e> HI_USE_SHELL
	// <i> 类shell交互
	// <i> 注意: 一条shell指令所占用的ram空间为12字节（此大小不统计const char产生的消耗）
	#ifndef HI_USE_SHELL
	#define HI_USE_SHELL          1
	#endif
		// <o> HI_SHELL_MAX_ARGS
		// <i> 内置shell可以解析的最大的输入指令参数量
		// <i> 默认大小: 8
		#define HI_SHELL_MAX_ARGS      8
		// <o> HI_SHELL_MAX_BUFSIZE
		// <i> 内置shell使用的数据缓存大小
		// <i> 默认大小: 32 [bytes] 
		#define HI_SHELL_MAX_BUFSIZE    		32
	// </e>
	// <e> HI_USE_RINGBUF
	// <i> 环形队列
	#ifndef HI_USE_RINGBUF
	#define HI_USE_RINGBUF          1
	#endif
	// </e>
// </e>
// <e> RTE_USE_ROUNDROBIN
// <i> 动态时间片轮转调度
// <i> 注意: 一个SoftTimer所占用的ram空间为24字节（此大小不统计const char产生的消耗）
#ifndef RTE_USE_ROUNDROBIN
#define RTE_USE_ROUNDROBIN   1 
#endif
	// <e> RTE_USE_OS
	// <i> 实时操作系统
	#ifndef RTE_USE_OS
	#define RTE_USE_OS          1
	#endif
	// </e>
	#if RTE_USE_OS == 1
	
	#endif
// </e>
// <e> RTE_USE_STATEMACHINE
// <i> 状态机模型
// <i> 注意: 一个StateMachine所占用的ram空间为24字节（此大小不统计const char产生的消耗）
#ifndef RTE_USE_STATEMACHINE
#define RTE_USE_STATEMACHINE   1 
#endif
// </e>
// <e> RTE_USE_KVDB
// <i> KV数据库
#ifndef RTE_USE_KVDB
#define RTE_USE_KVDB   1 
	// <o> KVDB_ERASE_MIN_SIZE
	// <i> KVDB最小擦除单位大小（与FLASH结构有关 单位：K）
	#ifndef KVDB_ERASE_MIN_SIZE
	#define KVDB_ERASE_MIN_SIZE         (128 * 1024)              /* it is 128K for compatibility */
	#endif
	// <e> KVDB_USE_PFS
	// <i> 掉电保护模式
	#ifndef KVDB_USE_PFS
	#define KVDB_USE_PFS          0
	#endif
	// </e>
	// <e> KVDB_USE_AUTO_UPDATE
	// <i> 自动更新（增量更新）
	#ifndef KVDB_USE_AUTO_UPDATE
	#define KVDB_USE_AUTO_UPDATE  0
	#endif
	// </e>
	// <o> KVDB_USER_SETTING_SIZE
	// <i> 用户设置环境变量大小
	#ifndef KVDB_USER_SETTING_SIZE
	#define KVDB_USER_SETTING_SIZE             2048
	#endif
	// <o> KVDB_ADDR_OFFSET
	// <i> KVDB地址偏移，加上FLASH首地址后为实际地址（也是用户程序最大大小 单位：K）
	#ifndef KVDB_ADDR_OFFSET
	#define KVDB_ADDR_OFFSET             			 128 * 1024 
	#endif
	// <o> FLASH_BASE
	// <i> FLASH首地址
	#ifndef KVDB_FLASH_BASE
	#define KVDB_FLASH_BASE                    0x00000000
	#endif
	// <o> KVDB_FM_VER_NUM
	// <i> 固件版本，如果检测到产品存储的版本号与设定版本号不一致，会自动追加默认环境变量集合中新增的环境变量。
	#ifndef KVDB_FM_VER_NUM
	#define KVDB_FM_VER_NUM                 0
	#endif
#endif
// </e>
#endif
/****************** (C) COPYRIGHT SuChow University Shannon********************/
