#ifndef __RTE_BGET_H
#define __RTE_BGET_H
#ifdef __cplusplus
extern "C" {
#endif
	#include "RTE_Include.h"
	#if RTE_USE_BGET == 1 
  #define DM_MEM_ATTR                  /*Complier prefix for big array declaration*/
	#define MEM_MIN(a,b) (a<b?a:b)
	#define MEM_MAX(a,b) (a>b?a:b)
	#define MEM_ABS(x) ((x)>0?(x):(-(x)))
	/*The size of this union must be 4 bytes (uint32_t)*/
	typedef union
	{
		struct
		{
			uint32_t used:1;        //1: if the entry is used
			uint32_t d_size:31;     //Size off the data (1 means 4 bytes)
		};
		uint32_t header;            //The header (used + d_size)
	}RTE_BGet_Header_t;
	typedef struct
	{
		RTE_BGet_Header_t header;
		uint8_t first_data;        //First data in entry data
	}RTE_BGet_Ent_t;
	typedef struct
	{
		uint32_t cnt_free;
		uint32_t cnt_used;
		uint32_t size_free;
		uint32_t size_free_big;
		uint32_t size_total;
		uint8_t pct_frag;
		uint8_t pct_used;
	}RTE_BGet_Monitor_t;
	typedef enum
	{
		MEM_RTE = 0,
		MEM_N,
	}RTE_BGet_Name_e;
	typedef struct
	{
		RTE_BGet_Name_e MemName;
		DM_MEM_ATTR uint8_t *work_mem;    //Work memory for allocation
		uint32_t totalsize;
	}RTE_BGet_t;

	extern void RTE_BPool(RTE_BGet_Name_e mem_name,void *buf,uint32_t len);
	extern void *RTE_BGet(RTE_BGet_Name_e mem_name,uint32_t size);
	extern void *RTE_BGetz(RTE_BGet_Name_e mem_name,uint32_t size);
	extern void RTE_BRel(RTE_BGet_Name_e mem_name,void *data);
	extern void *RTE_BGetr(RTE_BGet_Name_e mem_name,void *data_p,uint32_t new_size);
	extern void RTE_BGet_Defrag(RTE_BGet_Name_e mem_name);
	extern void RET_BGet_Monitor(RTE_BGet_Name_e mem_name,RTE_BGet_Monitor_t *mon_infor);
	extern uint32_t RTE_BGet_BufSize(void *data);
	#endif
#ifdef __cplusplus
}
#endif
#endif
/****************** (C) COPYRIGHT SuChow University Shannon********************/
