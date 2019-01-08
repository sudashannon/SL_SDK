#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.2 2018.10.02
*** History: 1.0 创建
             2.0 为RTE的升级做适配
						 2.1 增加了几个互斥锁
						 2.2 为增加得GUI内核做适配
*****************************************************************************/
/*************************************************
*** Args:   *file  断言失败的文件;
            line 断言失败的行;
*** Function: 断言功能
*************************************************/
void RTE_Assert(char *file, uint32_t line)
{ 
	RTE_Printf("[ASSERT]    Wrong parameters value: file %s on line %d\r\n", file, line);
	while (1)
	{
		
	}
}
/*************************************************
*** RTE所管理的内存，静态分配，32位对齐
*************************************************/
#if RTE_USE_MEMMANAGE == 1
RTE_ALIGN_32BYTES (__attribute__((section (".RAM_RTE"))) uint8_t RTE_RAM[RTE_MEM_SIZE * 1024]) = {0};
RTE_ALIGN_32BYTES (uint8_t DMA_RAM[4 * 1024]) = {0};
#endif
/*************************************************
*** RTE_KVDB的相关函数
*************************************************/
#if RTE_USE_KVDB
/* default ENV set for user */
static const ef_env rte_kvdb_lib[] = {
	{"boot_times","0"},
};
#define KVDB_TXT  "[KVDB]"
void RTE_KVDB_Init(void) 
{
	#if RTE_USE_OS == 1
	static const osMutexAttr_t MutexIDKVDBAttr = {
		"KVDBMutex",     // human readable mutex name
		0U,    					// attr_bits
		NULL,                // memory for control block   
		0U                   // size for control block
	};
	MutexIDKVDB = osMutexNew(&MutexIDKVDBAttr);
	#endif
	size_t default_env_set_size = sizeof(rte_kvdb_lib) / sizeof(rte_kvdb_lib[0]);
	EfErrCode result = EF_NO_ERR;
	if (result == EF_NO_ERR) 
	{
		result = ef_env_init(rte_kvdb_lib, default_env_set_size);
		if(result == EF_NO_ERR)
		{
			uint32_t i_boot_times = NULL;
			char *c_old_boot_times, c_new_boot_times[11] = {0};
			/* get the boot count number from Env */
			c_old_boot_times = ef_get_env("boot_times");
			RTE_AssertParam(c_old_boot_times);
			i_boot_times = atol(c_old_boot_times);
			/* boot count +1 */
			i_boot_times ++;
			RTE_Printf("%10s    The system boot times:%d\r\n",KVDB_TXT,i_boot_times);
			/* interger to string */
			usprintf(c_new_boot_times,"%d", i_boot_times);
			ef_set_env("boot_times", c_new_boot_times);
			ef_save_env();
		}
	}
}
#endif
/*************************************************
*** Args:   NULL
*** Function: RTE初始化
*************************************************/
void RTE_Init(void)
{
#if RTE_USE_MEMMANAGE == 1
	RTE_MEM_Pool(MEM_RTE,RTE_RAM,RTE_MEM_SIZE*1024);
	RTE_MEM_Pool(MEM_DMA,DMA_RAM,4*1024);
#endif
#if RTE_USE_ROUNDROBIN == 1
	RTE_RoundRobin_Init();
	RTE_RoundRobin_CreateGroup("RTEGroup");
	#if RTE_USE_SHELL == 1
		RTE_Shell_Init();
		RTE_RoundRobin_CreateTimer(0,"ShellTimer",100,1,1,RTE_Shell_Poll,(void *)0);
	#endif
#endif
#if RTE_USE_STDOUT != 1
	#if RTE_USE_OS == 1
	static const osMutexAttr_t MutexIDStdioAttr = {
		"StdioMutex",     // human readable mutex name
		0U,    					// attr_bits
		NULL,                // memory for control block   
		0U                   // size for control block
	};
	MutexIDStdio = osMutexNew(&MutexIDStdioAttr);
	#endif
#endif
}
