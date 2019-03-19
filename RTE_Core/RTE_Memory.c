#include "RTE_Memory.h"
/**
  ******************************************************************************
  * @file    RTE_MEM.h
  * @author  Shan Lei ->>lurenjia.tech ->>https://github.com/sudashannon
  * @brief   动态内存管理，可以管理多块内存。
  * @version V3.0 2019/03/17
  * @History V1.0 创建，移植并修改自lvgl
             V1.1 增加了互斥锁
			 V1.2 修复了互斥锁逻辑
			 v2.0 使用umm_alloc作为管理内核 https://github.com/rhempel/umm_malloc
			 v3.0 使用tlsf作为管理内核 http://www.gii.upv.es/tlsf/mm/intro
  ******************************************************************************
  */
#if RTE_USE_MEMMANAGE == 1
#include "RTE_UStdout.h"
#include <string.h>
static mem_control_t MemoryHandle[BANK_N] = {0};
#define MEM_STR  "[MEM]"
#define MAX_FLI		(30)
#define MAX_LOG2_SLI	(5)
#define MAX_SLI		(1 << MAX_LOG2_SLI)     /* MAX_SLI = 2^MAX_LOG2_SLI */
#define FLI_OFFSET	(6)     /* mem structure just will manage blocks bigger */
/* than 128 bytes */
#define SMALL_BLOCK	(128)
#define REAL_FLI	(MAX_FLI - FLI_OFFSET)
#define MIN_BLOCK_SIZE	(sizeof (mem_free_ptr_t))
#define BHDR_OVERHEAD	(sizeof (mem_bhdr_t) - MIN_BLOCK_SIZE)
#define MEM_SIGNATURE	(0x2A59FA59)

#define	PTR_MASK	(sizeof(void *) - 1)
#define BLOCK_SIZE	(0xFFFFFFFF - PTR_MASK)

#define GET_NEXT_BLOCK(_addr, _r) ((mem_bhdr_t *) ((char *) (_addr) + (_r)))
#define	MEM_ALIGN		  ((BLOCK_ALIGN) - 1)
#define ROUNDUP_SIZE(_r)          (((_r) + MEM_ALIGN) & ~MEM_ALIGN)
#define ROUNDDOWN_SIZE(_r)        ((_r) & ~MEM_ALIGN)
#define ROUNDUP(_x, _v)           ((((~(_x)) + 1) & ((_v)-1)) + (_x))

#define BLOCK_STATE	(0x1)
#define PREV_STATE	(0x2)

/* bit 0 of the block size */
#define FREE_BLOCK	(0x1)
#define USED_BLOCK	(0x0)

/* bit 1 of the block size */
#define PREV_FREE	(0x2)
#define PREV_USED	(0x0)


#define	MEM_ADD_SIZE(mem, b) do {									\
		mem->used_size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;	\
		} while(0)

#define	MEM_REMOVE_SIZE(mem, b) do {								\
		mem->used_size -= (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;	\
	} while(0)

typedef struct mem_free_ptr_struct{
    struct mem_bhdr_struct *prev;
    struct mem_bhdr_struct *next;
}mem_free_ptr_t;
typedef struct mem_bhdr_struct {
    /* This pointer is just valid if the first bit of size is set */
    struct mem_bhdr_struct *prev_hdr;
    /* The size is stored in bytes */
    uint32_t size;                /* bit 0 indicates whether the block is used and */
    /* bit 1 allows to know whether the previous block is free */
    union {
        struct mem_free_ptr_struct free_ptr;
        uint8_t buffer[1];         /*sizeof(struct free_ptr_struct)]; */
    } ptr;
}mem_bhdr_t;
/* This structure is embedded at the beginning of each area, giving us
 * enough information to cope with a set of areas */
typedef struct mem_area_info_struct {
    mem_bhdr_t *end;
    struct mem_area_info_struct *next;
}mem_area_info_t;
typedef struct
{
	/* the MEM's structure signature */
	uint32_t signature;
    /* A linked list holding all the existing areas */
    mem_area_info_t *area_head;
    /* the first-level bitmap */
    /* This array should have a size of REAL_FLI bits */
    uint32_t fl_bitmap;
    /* the second-level bitmap */
    uint32_t sl_bitmap[REAL_FLI];
    mem_bhdr_t *matrix[REAL_FLI][MAX_SLI];
}mem_t;
static const int memtable[] = {
    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4,
    4, 4,
    4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5,
    5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6,
    6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6,
    6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7,
    7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7,
    7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7,
    7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7,
    7, 7, 7, 7, 7, 7, 7
};
static __inline__ int ls_bit(int i)
{
    unsigned int a;
    unsigned int x = i & -i;
    a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);
    return memtable[x >> a] + a;
}
static __inline__ int ms_bit(int i)
{
    unsigned int a;
    unsigned int x = (unsigned int) i;
    a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);
    return memtable[x >> a] + a;
}
static __inline__ void set_bit(int nr, uint32_t * addr)
{
    addr[nr >> 5] |= 1 << (nr & 0x1f);
}
static __inline__ void clear_bit(int nr, uint32_t * addr)
{
    addr[nr >> 5] &= ~(1 << (nr & 0x1f));
}
static __inline__ void MAPPING_SEARCH(uint32_t * _r, int *_fl, int *_sl)
{
    int _t;
    if (*_r < SMALL_BLOCK) {
        *_fl = 0;
        *_sl = *_r / (SMALL_BLOCK / MAX_SLI);
    } else {
        _t = (1 << (ms_bit(*_r) - MAX_LOG2_SLI)) - 1;
        *_r = *_r + _t;
        *_fl = ms_bit(*_r);
        *_sl = (*_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
        *_fl -= FLI_OFFSET;
        /*if ((*_fl -= FLI_OFFSET) < 0) // FL wil be always >0!
         *_fl = *_sl = 0;
         */
        *_r &= ~_t;
    }
}
static __inline__ void MAPPING_INSERT(uint32_t _r, int *_fl, int *_sl)
{
    if (_r < SMALL_BLOCK) {
        *_fl = 0;
        *_sl = _r / (SMALL_BLOCK / MAX_SLI);
    } else {
        *_fl = ms_bit(_r);
        *_sl = (_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
        *_fl -= FLI_OFFSET;
    }
}
static __inline__ mem_bhdr_t *FIND_SUITABLE_BLOCK(mem_t * _tlsf, int *_fl, int *_sl)
{
    uint32_t _tmp = _tlsf->sl_bitmap[*_fl] & (~0 << *_sl);
    mem_bhdr_t *_b = NULL;

    if (_tmp) {
        *_sl = ls_bit(_tmp);
        _b = _tlsf->matrix[*_fl][*_sl];
    } else {
        *_fl = ls_bit(_tlsf->fl_bitmap & (~0 << (*_fl + 1)));
        if (*_fl > 0) {         /* likely */
            *_sl = ls_bit(_tlsf->sl_bitmap[*_fl]);
            _b = _tlsf->matrix[*_fl][*_sl];
        }
    }
    return _b;
}
#define EXTRACT_BLOCK_HDR(_b, _tlsf, _fl, _sl) do {					\
		_tlsf -> matrix [_fl] [_sl] = _b -> ptr.free_ptr.next;		\
		if (_tlsf -> matrix[_fl][_sl])								\
			_tlsf -> matrix[_fl][_sl] -> ptr.free_ptr.prev = NULL;	\
		else {														\
			clear_bit (_sl, &_tlsf -> sl_bitmap [_fl]);				\
			if (!_tlsf -> sl_bitmap [_fl])							\
				clear_bit (_fl, &_tlsf -> fl_bitmap);				\
		}															\
		_b -> ptr.free_ptr.prev =  NULL;				\
		_b -> ptr.free_ptr.next =  NULL;				\
	}while(0)


#define EXTRACT_BLOCK(_b, _tlsf, _fl, _sl) do {							\
		if (_b -> ptr.free_ptr.next)									\
			_b -> ptr.free_ptr.next -> ptr.free_ptr.prev = _b -> ptr.free_ptr.prev; \
		if (_b -> ptr.free_ptr.prev)									\
			_b -> ptr.free_ptr.prev -> ptr.free_ptr.next = _b -> ptr.free_ptr.next; \
		if (_tlsf -> matrix [_fl][_sl] == _b) {							\
			_tlsf -> matrix [_fl][_sl] = _b -> ptr.free_ptr.next;		\
			if (!_tlsf -> matrix [_fl][_sl]) {							\
				clear_bit (_sl, &_tlsf -> sl_bitmap[_fl]);				\
				if (!_tlsf -> sl_bitmap [_fl])							\
					clear_bit (_fl, &_tlsf -> fl_bitmap);				\
			}															\
		}																\
		_b -> ptr.free_ptr.prev = NULL;					\
		_b -> ptr.free_ptr.next = NULL;					\
	} while(0)

#define INSERT_BLOCK(_b, _tlsf, _fl, _sl) do {							\
		_b -> ptr.free_ptr.prev = NULL; \
		_b -> ptr.free_ptr.next = _tlsf -> matrix [_fl][_sl]; \
		if (_tlsf -> matrix [_fl][_sl])									\
			_tlsf -> matrix [_fl][_sl] -> ptr.free_ptr.prev = _b;		\
		_tlsf -> matrix [_fl][_sl] = _b;								\
		set_bit (_sl, &_tlsf -> sl_bitmap [_fl]);						\
		set_bit (_fl, &_tlsf -> fl_bitmap);								\
	} while(0)
static __inline__ mem_bhdr_t *process_area(void *area, uint32_t size)
{
    mem_bhdr_t *b, *lb, *ib;
    mem_area_info_t *ai;

    ib = (mem_bhdr_t *) area;
    ib->size =
        (sizeof(mem_area_info_t) <
         MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : ROUNDUP_SIZE(sizeof(mem_area_info_t)) | USED_BLOCK | PREV_USED;
    b = (mem_bhdr_t *) GET_NEXT_BLOCK(ib->ptr.buffer, ib->size & BLOCK_SIZE);
    b->size = ROUNDDOWN_SIZE(size - 3 * BHDR_OVERHEAD - (ib->size & BLOCK_SIZE)) | USED_BLOCK | PREV_USED;
    b->ptr.free_ptr.prev = b->ptr.free_ptr.next = 0;
    lb = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    lb->prev_hdr = b;
    lb->size = 0 | USED_BLOCK | PREV_FREE;
    ai = (mem_area_info_t *) ib->ptr.buffer;
    ai->next = 0;
    ai->end = lb;
    return ib;
}
/******************************************************************/
void *mem_malloc_ex(mem_bank_e bank,uint32_t size)
{
/******************************************************************/
    mem_t *mem = (mem_t *) MemoryHandle[bank].mp;
    mem_bhdr_t *b, *b2, *next_b;
    int fl, sl;
    uint32_t tmp_size;
    size = (size < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : ROUNDUP_SIZE(size);
    /* Rounding up the requested size and calculating fl and sl */
    MAPPING_SEARCH(&size, &fl, &sl);
    /* Searching a free block, recall that this function changes the values of fl and sl,
       so they are not longer valid when the function fails */
    b = FIND_SUITABLE_BLOCK(mem, &fl, &sl);
    if (!b)
        return NULL;            /* Not found */
    EXTRACT_BLOCK_HDR(b, mem, fl, sl);
    /*-- found: */
    next_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    /* Should the block be split? */
    tmp_size = (b->size & BLOCK_SIZE) - size;
    if (tmp_size >= sizeof(mem_bhdr_t)) {
        tmp_size -= BHDR_OVERHEAD;
        b2 = GET_NEXT_BLOCK(b->ptr.buffer, size);
        b2->size = tmp_size | FREE_BLOCK | PREV_USED;
        next_b->prev_hdr = b2;
        MAPPING_INSERT(tmp_size, &fl, &sl);
        INSERT_BLOCK(b2, mem, fl, sl);
        b->size = size | (b->size & PREV_STATE);
    } else {
        next_b->size &= (~PREV_FREE);
        b->size &= (~FREE_BLOCK);       /* Now it's used */
    }
    MEM_ADD_SIZE((&MemoryHandle[bank]), b);
    return (void *) b->ptr.buffer;
}
/******************************************************************/
void mem_free_ex(mem_bank_e bank,const void *ptr)
{
/******************************************************************/
    mem_t *mem = (mem_t *) MemoryHandle[bank].mp;
    mem_bhdr_t *b, *tmp_b;
    int fl = 0, sl = 0;
    if (!ptr) {
        return;
    }
    b = (mem_bhdr_t *) ((char *) ptr - BHDR_OVERHEAD);
    b->size |= FREE_BLOCK;
    MEM_REMOVE_SIZE((&MemoryHandle[bank]), b);
    b->ptr.free_ptr.prev = NULL;
    b->ptr.free_ptr.next = NULL;
    tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    if (tmp_b->size & FREE_BLOCK) {
        MAPPING_INSERT(tmp_b->size & BLOCK_SIZE, &fl, &sl);
        EXTRACT_BLOCK(tmp_b, mem, fl, sl);
        b->size += (tmp_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
    }
    if (b->size & PREV_FREE) {
        tmp_b = b->prev_hdr;
        MAPPING_INSERT(tmp_b->size & BLOCK_SIZE, &fl, &sl);
        EXTRACT_BLOCK(tmp_b, mem, fl, sl);
        tmp_b->size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
        b = tmp_b;
    }
    MAPPING_INSERT(b->size & BLOCK_SIZE, &fl, &sl);
    INSERT_BLOCK(b, mem, fl, sl);
    tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    tmp_b->size |= PREV_FREE;
    tmp_b->prev_hdr = b;
}

/******************************************************************/
void *mem_realloc_ex(mem_bank_e bank,void *ptr, uint32_t new_size)
{
/******************************************************************/
    mem_t *mem = (mem_t *) MemoryHandle[bank].mp;
    void *ptr_aux;
    unsigned int cpsize;
    mem_bhdr_t *b, *tmp_b, *next_b;
    int fl, sl;
    uint32_t tmp_size;

    if (!ptr) {
        if (new_size)
            return (void *) mem_malloc_ex(bank,new_size);
        if (!new_size)
            return NULL;
    } else if (!new_size) {
        mem_free_ex(bank,ptr);
        return NULL;
    }
    b = (mem_bhdr_t *) ((char *) ptr - BHDR_OVERHEAD);
    next_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    new_size = (new_size < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : ROUNDUP_SIZE(new_size);
    tmp_size = (b->size & BLOCK_SIZE);
    if (new_size <= tmp_size) {
	MEM_REMOVE_SIZE((&MemoryHandle[bank]), b);
        if (next_b->size & FREE_BLOCK) {
            MAPPING_INSERT(next_b->size & BLOCK_SIZE, &fl, &sl);
            EXTRACT_BLOCK(next_b, mem, fl, sl);
            tmp_size += (next_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
            next_b = GET_NEXT_BLOCK(next_b->ptr.buffer, next_b->size & BLOCK_SIZE);
            /* We allways reenter this free block because tmp_size will
               be greater then sizeof (bhdr_t) */
        }
        tmp_size -= new_size;
        if (tmp_size >= sizeof(mem_bhdr_t)) {
            tmp_size -= BHDR_OVERHEAD;
            tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, new_size);
            tmp_b->size = tmp_size | FREE_BLOCK | PREV_USED;
            next_b->prev_hdr = tmp_b;
            next_b->size |= PREV_FREE;
            MAPPING_INSERT(tmp_size, &fl, &sl);
            INSERT_BLOCK(tmp_b, mem, fl, sl);
            b->size = new_size | (b->size & PREV_STATE);
        }
	MEM_ADD_SIZE((&MemoryHandle[bank]), b);
        return (void *) b->ptr.buffer;
    }
    if ((next_b->size & FREE_BLOCK)) {
        if (new_size <= (tmp_size + (next_b->size & BLOCK_SIZE))) {
			MEM_REMOVE_SIZE((&MemoryHandle[bank]), b);
            MAPPING_INSERT(next_b->size & BLOCK_SIZE, &fl, &sl);
            EXTRACT_BLOCK(next_b, mem, fl, sl);
            b->size += (next_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
            next_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
            next_b->prev_hdr = b;
            next_b->size &= ~PREV_FREE;
            tmp_size = (b->size & BLOCK_SIZE) - new_size;
            if (tmp_size >= sizeof(mem_bhdr_t)) {
                tmp_size -= BHDR_OVERHEAD;
                tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, new_size);
                tmp_b->size = tmp_size | FREE_BLOCK | PREV_USED;
                next_b->prev_hdr = tmp_b;
                next_b->size |= PREV_FREE;
                MAPPING_INSERT(tmp_size, &fl, &sl);
                INSERT_BLOCK(tmp_b, mem, fl, sl);
                b->size = new_size | (b->size & PREV_STATE);
            }
			MEM_ADD_SIZE((&MemoryHandle[bank]), b);
            return (void *) b->ptr.buffer;
        }
    }
	ptr_aux = mem_malloc_ex(bank, new_size);
    if (!ptr_aux){
        return NULL;
    }      
    cpsize = ((b->size & BLOCK_SIZE) > new_size) ? new_size : (b->size & BLOCK_SIZE);
    memcpy(ptr_aux, ptr, cpsize);
    mem_free_ex(bank,ptr);
    return ptr_aux;
}


/******************************************************************/
void *mem_calloc_ex(mem_bank_e bank,uint32_t nelem, uint32_t elem_size)
{
/******************************************************************/
    void *ptr;

    if (nelem <= 0 || elem_size <= 0)
        return NULL;
	ptr = mem_malloc_ex(bank, nelem * elem_size);
    if (!ptr)
        return NULL;
    memset(ptr, 0, nelem * elem_size);
    return ptr;
}
/******************************************************************/
/******************** Begin of the allocator code *****************/
/******************************************************************/
uint32_t Memory_Pool(mem_bank_e bank,void *mem_pool,uint32_t mem_pool_size)
{
    mem_t *mem;
    mem_bhdr_t *b, *ib;
    if (!mem_pool || !mem_pool_size || mem_pool_size < sizeof(mem_t) + BHDR_OVERHEAD * 8) {
        RTE_Printf("%10s    Memory_Pool (): memory_pool invalid\r\n",MEM_STR);
        return 0;
    }
    if (((unsigned long) mem_pool & PTR_MASK)) {
        RTE_Printf("%10s    Memory_Pool (): mem_pool must be aligned to a word\r\n",MEM_STR);
        return 0;
    }
	mem = (mem_t *)mem_pool;
#if RTE_USE_OS == 1
	/* ------------------------------------------------------------------------- */
	const static osMutexAttr_t MemoryMutexattr = {
	  .attr_bits = osMutexRecursive,    // attr_bits
	};
	MemoryHandle[bank].mutex_mem = osMutexNew(&MemoryMutexattr);
#endif
    /* Check if already initialised */
    if (mem->signature == MEM_SIGNATURE) {
        MemoryHandle[bank].mp = mem_pool;
        b = GET_NEXT_BLOCK(MemoryHandle[bank].mp, ROUNDUP_SIZE(sizeof(mem_t)));
        return b->size & BLOCK_SIZE;
    }
	MemoryHandle[bank].mp = mem_pool;
    /* Zeroing the memory pool */
    memset(mem_pool, 0, sizeof(mem_t));
    ib = process_area(GET_NEXT_BLOCK
                      (mem_pool, ROUNDUP_SIZE(sizeof(mem_t))), ROUNDDOWN_SIZE(mem_pool_size - sizeof(mem_t)));
    b = GET_NEXT_BLOCK(ib->ptr.buffer, ib->size & BLOCK_SIZE);
    mem_free_ex(bank,b->ptr.buffer);
    mem->area_head = (mem_area_info_t *) ib->ptr.buffer;
    MemoryHandle[bank].used_size = mem_pool_size - (b->size & BLOCK_SIZE);
    return (b->size & BLOCK_SIZE);
}
/******************************************************************/
void Memory_DestroyPool(mem_bank_e bank)
{
/******************************************************************/
    mem_t *mem = (mem_t *) MemoryHandle[bank].mp;
    mem->signature = 0;
#if RTE_USE_OS == 1
	osMutexAcquire(MemoryHandle[bank].mutex_mem,osWaitForever);
#endif
}
/******************************************************************/
void *Memory_Alloc(mem_bank_e bank,uint32_t size)
{
/******************************************************************/
    void *ret = NULL;
	/* Protect the critical section... */
#if RTE_USE_OS == 1
	osMutexAcquire(MemoryHandle[bank].mutex_mem,osWaitForever);
#endif
    ret = mem_malloc_ex(bank, size);
  /* Release the critical section... */
#if RTE_USE_OS == 1
	osMutexRelease(MemoryHandle[bank].mutex_mem);
#endif
    return ret;
}
/******************************************************************/
void *Memory_Alloc0(mem_bank_e bank,uint32_t size)
{
/******************************************************************/
    void *ret = NULL;
	ret = Memory_Alloc(bank,size);
	if(ret&&size)
		memset(ret,0,size);
    return ret;
}
/******************************************************************/
void Memory_Free(mem_bank_e bank,const void *ptr)
{
/******************************************************************/
	/* Protect the critical section... */
#if RTE_USE_OS == 1
	osMutexAcquire(MemoryHandle[bank].mutex_mem,osWaitForever);
#endif
    mem_free_ex(bank, ptr);
  /* Release the critical section... */
#if RTE_USE_OS == 1
	osMutexRelease(MemoryHandle[bank].mutex_mem);
#endif
}

/******************************************************************/
void *Memory_Realloc(mem_bank_e bank,void *ptr, uint32_t size)
{
/******************************************************************/
    void *ret = NULL;
	/* Protect the critical section... */
#if RTE_USE_OS == 1
	osMutexAcquire(MemoryHandle[bank].mutex_mem,osWaitForever);
#endif
    ret = mem_realloc_ex(bank, ptr, size);
  /* Release the critical section... */
#if RTE_USE_OS == 1
	osMutexRelease(MemoryHandle[bank].mutex_mem);
#endif
    return ret;
}

/******************************************************************/
void *Memory_Calloc(mem_bank_e bank,uint32_t nelem, uint32_t elem_size)
{
/******************************************************************/
    void *ret = NULL;
	/* Protect the critical section... */
#if RTE_USE_OS == 1
	osMutexAcquire(MemoryHandle[bank].mutex_mem,osWaitForever);
#endif
    ret = mem_calloc_ex(bank, nelem, elem_size);
  /* Release the critical section... */
#if RTE_USE_OS == 1
	osMutexRelease(MemoryHandle[bank].mutex_mem);
#endif
    return ret;
}
static void print_block(mem_bhdr_t * b)
{
    if (!b)
        return;
    RTE_Printf("%10s    >> [%p] (",MEM_STR, b);
    if ((b->size & BLOCK_SIZE))
        RTE_Printf("%d bytes, ", (unsigned long) (b->size & BLOCK_SIZE));
    else
        RTE_Printf("sentinel, ");
    if ((b->size & BLOCK_STATE) == FREE_BLOCK)
        RTE_Printf("free [%p, %p], ", b->ptr.free_ptr.prev, b->ptr.free_ptr.next);
    else
        RTE_Printf("used, ");
    if ((b->size & PREV_STATE) == PREV_FREE)
        RTE_Printf("prev. free [%p])\n", b->prev_hdr);
    else
        RTE_Printf("prev used)\r\n");
}
void Memory_Demon(mem_bank_e bank)
{
	/* Protect the critical section... */
#if RTE_USE_OS == 1
	osMutexAcquire(MemoryHandle[bank].mutex_mem,osWaitForever);
#endif
	mem_t *mem = (mem_t*)MemoryHandle[bank].mp;
    mem_area_info_t *ai;
    mem_bhdr_t *next;
    RTE_Printf("%10s    BANK%d start at %x\r\n",MEM_STR,bank, mem);
	RTE_Printf("--------------------------------------------------\r\n");
    ai = mem->area_head;
    while (ai) {
        next = (mem_bhdr_t *) ((char *) ai - BHDR_OVERHEAD);
        while (next) {
            print_block(next);
            if ((next->size & BLOCK_SIZE))
                next = GET_NEXT_BLOCK(next->ptr.buffer, next->size & BLOCK_SIZE);
            else
                next = NULL;
        }
        ai = ai->next;
    }
	RTE_Printf("--------------------------------------------------\r\n");
  /* Release the critical section... */
#if RTE_USE_OS == 1
	osMutexRelease(MemoryHandle[bank].mutex_mem);
#endif
}
/******************************************************************/
uint32_t Memory_GetUsedSize(mem_bank_e bank)
{
/******************************************************************/
    return (MemoryHandle[bank].used_size);
}
void *Memory_AllocMaxFree(mem_bank_e bank,uint32_t *size)
{
	/* Protect the critical section... */
#if RTE_USE_OS == 1
	osMutexAcquire(MemoryHandle[bank].mutex_mem,osWaitForever);
#endif
	mem_t *mem = (mem_t*)MemoryHandle[bank].mp;
	mem_area_info_t *ai = mem->area_head;
	mem_bhdr_t *next;
	uint32_t nowsize = 0;
	uint32_t maxsize = 0;
	mem_bhdr_t *retval;
    while (ai) {
        next = (mem_bhdr_t *) ((char *) ai - BHDR_OVERHEAD);
        while (next) {
			nowsize = (next->size & BLOCK_SIZE);
            if((nowsize > maxsize)&&(next->size & FREE_BLOCK))
			{
				maxsize = nowsize;
				retval = next;
			}
            if (nowsize)
                next = GET_NEXT_BLOCK(next->ptr.buffer, next->size & BLOCK_SIZE);
            else
                next = NULL;
        }
        ai = ai->next;
    }
	*size = maxsize;
	if(maxsize > 0)
	{
		mem_bhdr_t *next_b = GET_NEXT_BLOCK(retval->ptr.buffer, retval->size & BLOCK_SIZE);
		next_b->size &= (~PREV_FREE);
		retval->size &= (~FREE_BLOCK);       /* Now it's used */
		MEM_ADD_SIZE((&MemoryHandle[bank]), retval);
  /* Release the critical section... */
#if RTE_USE_OS == 1
		osMutexRelease(MemoryHandle[bank].mutex_mem);
#endif
		return (void *)retval->ptr.buffer;
	}
  /* Release the critical section... */
#if RTE_USE_OS == 1
	osMutexRelease(MemoryHandle[bank].mutex_mem);
#endif
	return NULL;
}
uint32_t Memory_GetDataSize(const void *ptr)
{
	mem_bhdr_t *b = (mem_bhdr_t *) ((char *) ptr - BHDR_OVERHEAD);
	return(b->size & BLOCK_SIZE);
}
#endif
