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
	uprintf("[ASSERT]    Wrong parameters value: file %s on line %d\r\n", file, line);
	while (1)
	{
		
	}
}
/*************************************************
*** RTE_KVDB的相关函数
*************************************************/
#if RTE_USE_KVDB == 1
/* default ENV set for user */
static const ef_env rte_kvdb_lib[] = {
	{"boot_times","0"},
};
#define KVDB_TXT  "[KVDB]"
void KVDB_Init(void) 
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
			uprintf("%10s    The system boot times:%d\r\n",KVDB_TXT,i_boot_times);
			/* interger to string */
			usprintf(c_new_boot_times,"%d", i_boot_times);
			ef_set_and_save_env("boot_times", c_new_boot_times);
		}
	}
}
#endif
#if RTE_USE_SHELL == 1
#include "RTE_Shell.h"
static shell_error_e Shell_Memory_Demon(int argc, char *argv[])
{
    if(argc!=3)
		return SHELL_ARGSERROR;
	uprintf("--------------------------------------------------\r\n");
	uprintf("%10s    BANK COUNTS:%d\r\n","[MEM]",BANK_N);
	uprintf("--------------------------------------------------\r\n");
	if(strstr(argv[2],"all"))
    {
        for(mem_bank_e i=(mem_bank_e)(BANK_NULL+1);i<BANK_N;i++)
            Memory_Demon(i);
    }
    else
        Memory_Demon((mem_bank_e)atoi(argv[2]));
	return(SHELL_NOERR);
}
#endif
/*************************************************
*** RTE所管理的内存，静态分配，32位对齐
*************************************************/
#if RTE_USE_MEMORY == 1
MEM_ALIGN_BYTES (uint8_t RAM_RTE[RTE_MEM_SIZE * 1024]) = {0};
#endif
/*************************************************
*** Args:   NULL
*** Function: RTE初始化
*************************************************/
void RTE_Init(void)
{
#if RTE_USE_MEMORY == 1
	Memory_Pool(MEM_RTE,RAM_RTE,RTE_MEM_SIZE*1024);
#endif
#if RTE_USE_ROUNDROBIN == 1
	#if RTE_USE_SHELL == 1
        Shell_Init();
    #endif // RTE_USE_SHELL
	RoundRobin_Init();
	RoundRobin_CreateGroup(0);
    #if RTE_USE_SHELL == 1
        RoundRobin_CreateTimer(0,0,10,1,1,Shell_Poll,(void *)0);
	#endif
#endif
#if RTE_USE_MEMORY == 1
	#if RTE_USE_SHELL == 1
    Shell_CreateModule("mem");
	Shell_AddCommand("mem","demon",Shell_Memory_Demon,"Demon a specific membank Example:mem.demon(0)");
	#endif
#endif
#if RTE_USE_PRINTF == 1
	Printf_Init();
#endif
}
