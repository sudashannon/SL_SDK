#ifndef __RTE_MEMORY_H
#define __RTE_MEMORY_H
#include "RTE_Config.h"
#if RTE_USE_MEMMANAGE == 1
#if RTE_USE_OS == 1
#include "cmsis_os2.h"
#endif
#include <stdint.h>
/* Some IMPORTANT MEM parameters */
/* Unlike the preview MEM versions, now they are statics */
#define BLOCK_ALIGN (sizeof(void *) * 2)
#define MEM_ALIGN_BYTES(buf) buf __attribute__ ((aligned (BLOCK_ALIGN)))
typedef enum
{
    BANK_NULL = -1,
    BANK_RTE = 0,
	BANK_DMA = 1,
    BANK_N,
}mem_bank_e;
typedef struct
{
	char *mp;
    uint32_t used_size;
#if RTE_USE_OS == 1
	osMutexId_t mutex_mem;
#endif
}mem_control_t;
extern uint32_t Memory_Pool(mem_bank_e bank,void *mem_pool,uint32_t mem_pool_size);
extern void *Memory_Alloc(mem_bank_e bank, uint32_t size);
extern void *Memory_Alloc0(mem_bank_e bank, uint32_t size);
extern void *Memory_Realloc(mem_bank_e bank, void *ptr, uint32_t size );
extern void Memory_Free(mem_bank_e bank, const void *ptr);
extern uint32_t Memory_GetDataSize(const void *ptr);
extern uint32_t Memory_GetUsedSize(mem_bank_e bank);
extern void *Memory_AllocMaxFree(mem_bank_e bank,uint32_t *size);
extern void Memory_Demon(mem_bank_e bank);
#endif
#endif // __RTE_MEMORY_H
