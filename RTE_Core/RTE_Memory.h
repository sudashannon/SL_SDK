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
/* Note: no need for RTE_Memory3.1 */
#define BLOCK_ALIGN (sizeof(void *) * 2)
#define MEM_ALIGN_BYTES(buf) buf __attribute__ ((aligned (BLOCK_ALIGN)))

typedef enum
{
    BANK_NULL = -1,
    BANK_RTE = 0,
	BANK_DMA = 1,
    BANK_N,
}mem_bank_e;
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
extern void Memory_Pool(mem_bank_e bank,void *mem_pool,size_t mem_pool_size);
extern void *Memory_Alloc(mem_bank_e bank, size_t size);
extern void *Memory_Alloc0(mem_bank_e bank, size_t size);
extern void *Memory_Realloc(mem_bank_e bank, void *ptr, size_t size );
extern void Memory_Free(mem_bank_e bank, void *ptr);
extern size_t Memory_GetDataSize(void *ptr);
extern void *Memory_AllocMaxFree(mem_bank_e bank,size_t *size);
extern void Memory_Demon(mem_bank_e bank);
#endif
#endif // __RTE_MEMORY_H
