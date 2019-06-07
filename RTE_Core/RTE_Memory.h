#ifndef __RTE_MEMORY_H
#define __RTE_MEMORY_H
#include "RTE_Config.h"
#if RTE_USE_MEMORY == 1
#if RTE_USE_OS == 1
#include "cmsis_os2.h"
#endif
#include <stdint.h>
#include <stddef.h>
/* Some IMPORTANT MEM parameters */
/* Unlike the preview MEM versions, now they are statics */
/* Note: no need for RTE_Memory3.1 */
#define BLOCK_ALIGN (sizeof(void *) * 2)
#define MEM_ALIGN_BYTES(buf) buf __attribute__ ((aligned (BLOCK_ALIGN)))
typedef enum
{
    BANK_NULL = -1,
    MEM_RTE = 0,
    BANK_N,
}mem_bank_e;
#if MEMORY_TYPE == 1
/* mem_t: a mem structure. Can contain 1 to N pools. */
/* pool_t: a block of memory that mem can manage. */
typedef void* mem_t;
typedef void* pool_t;
typedef struct
{
	mem_t mem;
	pool_t pool;
#if RTE_USE_OS == 1
	osMutexId_t mutex_mem;
#endif
}mem_control_t;
#elif MEMORY_TYPE == 0
#if MEMORY_SIMPLE_64BIT == 1
#define mem_unit uint64_t
#else
#define mem_unit uint32_t
#endif
typedef union {
	struct {
		mem_unit used: 1;       //1: if the entry is used
		mem_unit d_size: 31;    //Size off the data (1 means 4 bytes)
	};
	mem_unit header;            //The header (used + d_size)
}mem_head_t;
typedef struct {
	mem_head_t head;
	uint8_t first_data;        /*First data byte in the allocated data (Just for easily create a pointer)*/
}mem_ent_t;
typedef struct
{
	uint8_t *work_mem;    //Work memory for allocation
	size_t totalsize;
#if RTE_USE_OS
	osMutexId_t mutex_mem;
#endif
}mem_control_t;
typedef struct
{
	uint8_t used_pct;
	uint8_t frag_pct;
	uint32_t total_size;
	uint32_t free_cnt;
	uint32_t free_size;
	uint32_t free_biggest_size;
	uint32_t used_cnt;
}mem_mon_t;
#endif
extern void Memory_Pool(mem_bank_e bank,void *mem_pool,size_t mem_pool_size);
extern void *Memory_Alloc(mem_bank_e bank, size_t size);
extern void *Memory_Alloc0(mem_bank_e bank, size_t size);
extern void *Memory_Realloc(mem_bank_e bank, void *ptr, size_t size );
extern void Memory_Free(mem_bank_e bank, void *ptr);
extern size_t Memory_GetDataSize(void *ptr);
#if MEMORY_TYPE == 0
extern void Memory_Defrag(mem_bank_e bank);
extern void Memory_Demon(mem_bank_e mem_name,mem_mon_t * mon_p);
extern size_t Memory_MaxFree(mem_bank_e bank);
#elif MEMORY_TYPE == 1
extern void Memory_Demon(mem_bank_e bank);
extern void *Memory_AllocMaxFree(mem_bank_e bank,size_t *size);
#endif
#endif
#endif // __RTE_MEMORY_H
