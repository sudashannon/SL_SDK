/**
 * @file rte_memory.c
 * @author Leon Shan (813475603@qq.com)
 * @brief NOTE: The mutex provided to memory pool may not
 *             be recursive.
 * @version 1.0.0
 * @date 2021-07-27
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "../../inc/middle_layer/rte_memory.h"
#include "../../inc/middle_layer/rte_log.h"

#define THIS_MODULE LOG_STR(MEMORY)
#define MEM_LOGF(...) LOG_FATAL(THIS_MODULE, __VA_ARGS__)
#define MEM_LOGE(...) LOG_ERROR(THIS_MODULE, __VA_ARGS__)
#define MEM_LOGI(...) LOG_INFO(THIS_MODULE, __VA_ARGS__)
#define MEM_LOGW(...) LOG_WARN(THIS_MODULE, __VA_ARGS__)
#define MEM_LOGD(...) LOG_DEBUG(THIS_MODULE, __VA_ARGS__)
#define MEM_LOGV(...) LOG_VERBOSE(THIS_MODULE, __VA_ARGS__)
#define MEM_ASSERT(v) LOG_ASSERT(THIS_MODULE, v)
#define MEM_LOCK(bank)   RTE_LOCK(mem_handle[bank].mutex)
#define MEM_UNLOCK(bank) RTE_UNLOCK(mem_handle[bank].mutex)

#if RTE_USE_SIMPLY_MEMPOOL == 0

/**
 * @brief This struct defines the memory handle.
 *
 */
typedef void* mem_t;
typedef void* pool_t;

typedef struct __mem_handle_t_ {
    mem_t mem;
    pool_t pool;
    rte_mutex_t *mutex;
} mem_handle_t;

/* The handle for memory */
static mem_handle_t mem_handle[BANK_CNT] = {
    {NULL, NULL, NULL},
};

/* RealView Compilation Tools for ARM */
#if defined (__ARMCC_VERSION)
#include <cmsis_compiler.h>
static inline int mem_ffs(unsigned int word)
{
    const unsigned int reverse = word & (~word + 1);
    const int bit = 32 - __CLZ(reverse);
    return bit - 1;
}
static inline int mem_fls(unsigned int word)
{
    const int bit = word ? 32 - __CLZ(word) : 0;
    return bit - 1;
}
/*
** gcc 3.4 and above have builtin support, specialized for architecture.
** Some compilers masquerade as gcc; patchlevel test filters them out.
*/
#elif defined (__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)) \
    && defined (__GNUC_PATCHLEVEL__)
#if defined (__SNC__)
/* SNC for Playstation 3. */
static inline int mem_ffs(unsigned int word)
{
    const unsigned int reverse = word & (~word + 1);
    const int bit = 32 - __builtin_clz(reverse);
    return bit - 1;
}
#else
static inline int mem_ffs(unsigned int word)
{
    return __builtin_ffs(word) - 1;
}
#endif
static inline int mem_fls(unsigned int word)
{
    const int bit = word ? 32 - __builtin_clz(word) : 0;
    return bit - 1;
}
#endif
#if RTE_MEMPOOL_USE_64BIT
static inline int mem_fls_sizet(size_t size)
{
    int high = (int)(size >> 32);
    int bits = 0;
    if (high) {
        bits = 32 + mem_fls(high);
    } else {
        bits = mem_fls((int)size & 0xffffffff);

    }
    return bits;
}
#else
#define mem_fls_sizet mem_fls
#endif
/*
** Constants.
*/
/* Public constants: may be modified. */
enum mem_public
{
    /* log2 of number of linear subdivisions of block sizes. Larger
    ** values require more memory in the control structure. Values of
    ** 4 or 5 are typical.
    */
    SL_INDEX_COUNT_LOG2 = 5,
};
/* Private constants: do not modify. */
enum mem_private
{
#if RTE_MEMPOOL_USE_64BIT
    /* All allocation sizes and addresses are aligned to 8 bytes. */
    ALIGN_SIZE_LOG2 = 3,
#else
    /* All allocation sizes and addresses are aligned to 4 bytes. */
    ALIGN_SIZE_LOG2 = 2,
#endif
    ALIGN_SIZE = (1 << ALIGN_SIZE_LOG2),
    /*
    ** We support allocations of sizes up to (1 << FL_INDEX_MAX) bits.
    ** However, because we linearly subdivide the second-level lists, and
    ** our minimum size granularity is 4 bytes, it doesn't make sense to
    ** create first-level lists for sizes smaller than SL_INDEX_COUNT * 4,
    ** or (1 << (SL_INDEX_COUNT_LOG2 + 2)) bytes, as there we will be
    ** trying to split size ranges into more slots than we have available.
    ** Instead, we calculate the minimum threshold size, and place all
    ** blocks below that size into the 0th first-level list.
    */

#if RTE_MEMPOOL_USE_64BIT
    /*
    ** TODO: We can increase this to support larger sizes, at the expense
    ** of more overhead in the mem structure.
    */
    FL_INDEX_MAX = 32,
#else
    FL_INDEX_MAX = 30,
#endif
    SL_INDEX_COUNT = (1 << SL_INDEX_COUNT_LOG2),
    FL_INDEX_SHIFT = (SL_INDEX_COUNT_LOG2 + ALIGN_SIZE_LOG2),
    FL_INDEX_COUNT = (FL_INDEX_MAX - FL_INDEX_SHIFT + 1),

    SMALL_BLOCK_SIZE = (1 << FL_INDEX_SHIFT),
};
/*
** Cast macros.
*/
#define mem_cast(t, exp)    ((t) (exp))
/*
** Static assertion mechanism.
*/
#define _mem_glue2(x, y) x ## y
#define _mem_glue(x, y) _mem_glue2(x, y)
#define mem_static_assert(exp) \
    typedef char _mem_glue(static_assert, __LINE__) [(exp) ? 1 : -1]
/* This code has been tested on 32- and 64-bit (LP/LLP) architectures. */
mem_static_assert(sizeof(int) * CHAR_BIT == 32);
mem_static_assert(sizeof(size_t) * CHAR_BIT >= 32);
mem_static_assert(sizeof(size_t) * CHAR_BIT <= 64);
/* SL_INDEX_COUNT must be <= number of bits in sl_bitmap's storage type. */
mem_static_assert(sizeof(unsigned int) * CHAR_BIT >= SL_INDEX_COUNT);
/* Ensure we've properly tuned our sizes. */
mem_static_assert(ALIGN_SIZE == SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
/*
** Data structures and associated constants.
*/
/*
** Block header structure.
**
** There are several implementation subtleties involved:
** - The prev_phys_block field is only valid if the previous block is free.
** - The prev_phys_block field is actually stored at the end of the
**   previous block. It appears at the beginning of this structure only to
**   simplify the implementation.
** - The next_free / prev_free fields are only valid if the block is free.
*/
typedef struct block_header_t
{
    /* Points to the previous physical block. */
    struct block_header_t* prev_phys_block;
    /* The size of this block, excluding the block header. */
    size_t size;
    /* Next and previous free blocks. */
    struct block_header_t* next_free;
    struct block_header_t* prev_free;
} block_header_t;
/*
** Since block sizes are always at least a multiple of 4, the two least
** significant bits of the size field are used to store the block status:
** - bit 0: whether block is busy or free
** - bit 1: whether previous block is busy or free
*/
static const size_t block_header_free_bit = 1 << 0;
static const size_t block_header_prev_free_bit = 1 << 1;
/*
** The size of the block header exposed to used blocks is the size field.
** The prev_phys_block field is stored *inside* the previous free block.
*/
static const size_t block_header_overhead = sizeof(size_t);
/* User data starts directly after the size field in a used block. */
static const size_t block_start_offset =
    offsetof(block_header_t, size) + sizeof(size_t);
/*
** A free block must be large enough to store its header minus the size of
** the prev_phys_block field, and no larger than the number of addressable
** bits for FL_INDEX.
*/
static const size_t block_size_min =
    sizeof(block_header_t) - sizeof(block_header_t*);
static const size_t block_size_max = mem_cast(size_t, 1) << FL_INDEX_MAX;
/* The mem control structure. */
typedef struct control_t
{
    /* Empty lists point at this block to indicate they are free. */
    block_header_t block_null;
    /* Bitmaps for free lists. */
    unsigned int fl_bitmap;
    unsigned int sl_bitmap[FL_INDEX_COUNT];
    /* Head of free lists. */
    block_header_t* blocks[FL_INDEX_COUNT][SL_INDEX_COUNT];
} control_t;
/* A type used for casting when doing pointer arithmetic. */
typedef ptrdiff_t memptr_t;
/*
** block_header_t member functions.
*/
static size_t block_size(const block_header_t* block)
{
    return block->size & ~(block_header_free_bit | block_header_prev_free_bit);
}
static void block_set_size(block_header_t* block, size_t size)
{
    const size_t oldsize = block->size;
    block->size = size | (oldsize & (block_header_free_bit | block_header_prev_free_bit));
}
static int block_is_last(const block_header_t* block)
{
    return block_size(block) == 0;
}
static int block_is_free(const block_header_t* block)
{
    return mem_cast(int, block->size & block_header_free_bit);
}
static void block_set_free(block_header_t* block)
{
    block->size |= block_header_free_bit;
}
static void block_set_used(block_header_t* block)
{
    block->size &= ~block_header_free_bit;
}
static int block_is_prev_free(const block_header_t* block)
{
    return mem_cast(int, block->size & block_header_prev_free_bit);
}
static void block_set_prev_free(block_header_t* block)
{
    block->size |= block_header_prev_free_bit;
}
static void block_set_prev_used(block_header_t* block)
{
    block->size &= ~block_header_prev_free_bit;
}
static block_header_t* block_from_ptr(const void* ptr)
{
    return mem_cast(block_header_t*,
        mem_cast(unsigned char*, ptr) - block_start_offset);
}
static void* block_to_ptr(const block_header_t* block)
{
    return mem_cast(void*,
        mem_cast(unsigned char*, block) + block_start_offset);
}
/* Return location of next block after block of given size. */
static block_header_t* offset_to_block(const void* ptr, memptr_t size)
{
    return mem_cast(block_header_t*, mem_cast(memptr_t, ptr) + size);
}
/* Return location of previous block. */
static block_header_t* block_prev(const block_header_t* block)
{
    MEM_ASSERT(block_is_prev_free(block));
    return block->prev_phys_block;
}
/* Return location of next existing block. */
static block_header_t* block_next(const block_header_t* block)
{
    block_header_t* next = offset_to_block(block_to_ptr(block),
        block_size(block) - block_header_overhead);
    MEM_ASSERT(!block_is_last(block));
    return next;
}
/* Link a new block with its physical neighbor, return the neighbor. */
static block_header_t* block_link_next(block_header_t* block)
{
    block_header_t* next = block_next(block);
    next->prev_phys_block = block;
    return next;
}
static void block_mark_as_free(block_header_t* block)
{
    /* Link the block to the next block, first. */
    block_header_t* next = block_link_next(block);
    block_set_prev_free(next);
    block_set_free(block);
}
static void block_mark_as_used(block_header_t* block)
{
    block_header_t* next = block_next(block);
    block_set_prev_used(next);
    block_set_used(block);
}
static size_t align_up(size_t x, size_t align)
{
    MEM_ASSERT(0 == (align & (align - 1)));
    return (x + (align - 1)) & ~(align - 1);
}
static size_t align_down(size_t x, size_t align)
{
    MEM_ASSERT(0 == (align & (align - 1)));
    return x - (x & (align - 1));
}
static void* align_ptr(const void* ptr, size_t align)
{
    const memptr_t aligned =
        (mem_cast(memptr_t, ptr) + (align - 1)) & ~(align - 1);
    MEM_ASSERT(0 == (align & (align - 1)));
    return mem_cast(void*, aligned);
}
/*
** Adjust an allocation size to be aligned to word size, and no smaller
** than internal minimum.
*/
size_t adjust_request_size(size_t size, size_t align)
{
    size_t adjust = 0;
    if (size)
    {
        const size_t aligned = align_up(size, align);

        /* aligned sized must not exceed block_size_max or we'll go out of bounds on sl_bitmap */
        if (aligned < block_size_max)
        {
            adjust = RTE_MAX(aligned, block_size_min);
        }
    }
    return adjust;
}
/*
** mem utility functions. In most cases, these are direct translations of
** the documentation found in the white paper.
*/

static void mapping_insert(size_t size, int* fli, int* sli)
{
    int fl, sl;
    if (size < SMALL_BLOCK_SIZE)
    {
        /* Store small blocks in first list. */
        fl = 0;
        sl = mem_cast(int, size) / (SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
    }
    else
    {
        fl = mem_fls_sizet(size);
        sl = mem_cast(int, size >> (fl - SL_INDEX_COUNT_LOG2)) ^ (1 << SL_INDEX_COUNT_LOG2);
        fl -= (FL_INDEX_SHIFT - 1);
    }
    *fli = fl;
    *sli = sl;
}
/* This version rounds up to the next block size (for allocations) */
static void mapping_search(size_t size, int* fli, int* sli)
{
    if (size >= SMALL_BLOCK_SIZE)
    {
        const size_t round = (1 << (mem_fls_sizet(size) - SL_INDEX_COUNT_LOG2)) - 1;
        size += round;
    }
    mapping_insert(size, fli, sli);
}
static block_header_t* search_suitable_block(control_t* control, int* fli, int* sli)
{
    int fl = *fli;
    int sl = *sli;

    /*
    ** First, search for a block in the list associated with the given
    ** fl/sl index.
    */
    unsigned int sl_map = control->sl_bitmap[fl] & (~0U << sl);
    if (!sl_map)
    {
        /* No block exists. Search in the next largest first-level list. */
        const unsigned int fl_map = control->fl_bitmap & (~0U << (fl + 1));
        if (!fl_map)
        {
            /* No free blocks available, memory has been exhausted. */
            return 0;
        }

        fl = mem_ffs(fl_map);
        *fli = fl;
        sl_map = control->sl_bitmap[fl];
    }
    MEM_ASSERT(sl_map);
    sl = mem_ffs(sl_map);
    *sli = sl;
    /* Return the first block in the free list. */
    return control->blocks[fl][sl];
}
/* Remove a free block from the free list.*/
static void remove_free_block(control_t* control, block_header_t* block, int fl, int sl)
{
    block_header_t* prev = block->prev_free;
    block_header_t* next = block->next_free;
    MEM_ASSERT(prev);
    MEM_ASSERT(next);
    next->prev_free = prev;
    prev->next_free = next;

    /* If this block is the head of the free list, set new head. */
    if (control->blocks[fl][sl] == block)
    {
        control->blocks[fl][sl] = next;

        /* If the new head is null, clear the bitmap. */
        if (next == &control->block_null)
        {
            control->sl_bitmap[fl] &= ~(1U << sl);

            /* If the second bitmap is now empty, clear the fl bitmap. */
            if (!control->sl_bitmap[fl])
            {
                control->fl_bitmap &= ~(1U << fl);
            }
        }
    }
}
/* Insert a free block into the free block list. */
static void insert_free_block(control_t* control, block_header_t* block, int fl, int sl)
{
    block_header_t* current = control->blocks[fl][sl];
    MEM_ASSERT(current);
    MEM_ASSERT(block);
    block->next_free = current;
    block->prev_free = &control->block_null;
    current->prev_free = block;
    MEM_ASSERT(block_to_ptr(block) == align_ptr(block_to_ptr(block), ALIGN_SIZE));
    /*
    ** Insert the new block at the head of the list, and mark the first-
    ** and second-level bitmaps appropriately.
    */
    control->blocks[fl][sl] = block;
    control->fl_bitmap |= (1U << fl);
    control->sl_bitmap[fl] |= (1U << sl);
}
/* Remove a given block from the free list. */
static void block_remove(control_t* control, block_header_t* block)
{
    int fl, sl;
    mapping_insert(block_size(block), &fl, &sl);
    remove_free_block(control, block, fl, sl);
}
/* Insert a given block into the free list. */
static void block_insert(control_t* control, block_header_t* block)
{
    int fl, sl;
    mapping_insert(block_size(block), &fl, &sl);
    insert_free_block(control, block, fl, sl);
}
static int block_can_split(block_header_t* block, size_t size)
{
    return block_size(block) >= sizeof(block_header_t) + size;
}
/* Split a block into two, the second of which is free. */
static block_header_t* block_split(block_header_t* block, size_t size)
{
    /* Calculate the amount of space left in the remaining block. */
    block_header_t* remaining =
        offset_to_block(block_to_ptr(block), size - block_header_overhead);
    const size_t remain_size = block_size(block) - (size + block_header_overhead);
    MEM_ASSERT(block_to_ptr(remaining) == align_ptr(block_to_ptr(remaining), ALIGN_SIZE));

    MEM_ASSERT(block_size(block) == remain_size + size + block_header_overhead);
    block_set_size(remaining, remain_size);
    MEM_ASSERT(block_size(remaining) >= block_size_min);

    block_set_size(block, size);
    block_mark_as_free(remaining);

    return remaining;
}
/* Absorb a free block's storage into an adjacent previous free block. */
static block_header_t* block_absorb(block_header_t* prev, block_header_t* block)
{
    MEM_ASSERT(!block_is_last(prev));
    /* Note: Leaves flags untouched. */
    prev->size += block_size(block) + block_header_overhead;
    block_link_next(prev);
    return prev;
}
/* Merge a just-freed block with an adjacent previous free block. */
static block_header_t* block_merge_prev(control_t* control, block_header_t* block)
{
    if (block_is_prev_free(block))
    {
        block_header_t* prev = block_prev(block);
        MEM_ASSERT(prev);
        MEM_ASSERT(block_is_free(prev));
        block_remove(control, prev);
        block = block_absorb(prev, block);
    }
    return block;
}
/* Merge a just-freed block with an adjacent free block. */
static block_header_t* block_merge_next(control_t* control, block_header_t* block)
{
    block_header_t* next = block_next(block);
    MEM_ASSERT(next);
    if (block_is_free(next))
    {
        MEM_ASSERT(!block_is_last(block));
        block_remove(control, next);
        block = block_absorb(block, next);
    }
    return block;
}
/* Trim any trailing block space off the end of a block, return to pool. */
static void block_trim_free(control_t* control, block_header_t* block, size_t size)
{
    MEM_ASSERT(block_is_free(block));
    if (block_can_split(block, size))
    {
        block_header_t* remaining_block = block_split(block, size);
        block_link_next(block);
        block_set_prev_free(remaining_block);
        block_insert(control, remaining_block);
    }
}
/* Trim any trailing block space off the end of a used block, return to pool. */
static void block_trim_used(control_t* control, block_header_t* block, size_t size)
{
    MEM_ASSERT(!block_is_free(block));
    if (block_can_split(block, size))
    {
        /* If the next block is free, we must coalesce. */
        block_header_t* remaining_block = block_split(block, size);
        block_set_prev_used(remaining_block);
        remaining_block = block_merge_next(control, remaining_block);
        block_insert(control, remaining_block);
    }
}
static block_header_t* block_trim_free_leading(control_t* control, block_header_t* block, size_t size)
{
    block_header_t* remaining_block = block;
    if (block_can_split(block, size))
    {
        /* We want the 2nd block. */
        remaining_block = block_split(block, size - block_header_overhead);
        block_set_prev_free(remaining_block);
        block_link_next(block);
        block_insert(control, block);
    }
    return remaining_block;
}
static block_header_t* block_locate_free(control_t* control, size_t size)
{
    int fl = 0, sl = 0;
    block_header_t* block = 0;
    if (size)
    {
        mapping_search(size, &fl, &sl);
        /*
        ** mapping_search can futz with the size, so for excessively large sizes it can sometimes wind up
        ** with indices that are off the end of the block array.
        ** So, we protect against that here, since this is the only callsite of mapping_search.
        ** Note that we don't need to check sl, since it comes from a modulo operation that guarantees it's always in range.
        */
        if (fl < FL_INDEX_COUNT)
        {
            block = search_suitable_block(control, &fl, &sl);
        }
    }
    if (block)
    {
        MEM_ASSERT(block_size(block) >= size);
        remove_free_block(control, block, fl, sl);
    }
    return block;
}
static void* block_prepare_used(control_t* control, block_header_t* block, size_t size)
{
    void* p = 0;
    if (block)
    {
        MEM_ASSERT(size);
        block_trim_free(control, block, size);
        block_mark_as_used(block);
        p = block_to_ptr(block);
    }
    return p;
}
/* Clear structure and point all empty lists at the null block. */
static void control_construct(control_t* control)
{
    int i, j;
    control->block_null.next_free = &control->block_null;
    control->block_null.prev_free = &control->block_null;
    control->fl_bitmap = 0;
    for (i = 0; i < FL_INDEX_COUNT; ++i)
    {
        control->sl_bitmap[i] = 0;
        for (j = 0; j < SL_INDEX_COUNT; ++j)
        {
            control->blocks[i][j] = &control->block_null;
        }
    }
}
static mem_t mem_create(void* mem_pool)
{
    if (((memptr_t)mem_pool % ALIGN_SIZE) != 0)
    {
        MEM_LOGE("Memory must be aligned to %u bytes.", (unsigned int)ALIGN_SIZE);
        return 0;
    }
    control_construct(mem_cast(control_t*, mem_pool));
    return mem_cast(mem_t, mem_pool);
}
static pool_t mem_add_pool(mem_t mem, void* mem_pool, size_t mem_pool_size)
{
    block_header_t* block;
    block_header_t* next;
    const size_t pool_overhead = 2 * block_header_overhead;
    const size_t pool_bytes = align_down(mem_pool_size - pool_overhead, ALIGN_SIZE);
    if (((memptr_t)mem_pool % ALIGN_SIZE) != 0)
    {
        MEM_LOGE("Memory must be aligned by %u bytes.", (unsigned int)ALIGN_SIZE);
        return 0;
    }

    if (pool_bytes < block_size_min || pool_bytes > block_size_max)
    {
#if RTE_MEMPOOL_USE_64BIT
        MEM_LOGE("Memory size must be between 0x%x and 0x%x00 bytes.",
            (unsigned int)(pool_overhead + block_size_min),
            (unsigned int)((pool_overhead + block_size_max) / 256));
#else
        MEM_LOGE("Memory size must be between %u and %u bytes now: %u.",
            (unsigned int)(pool_overhead + block_size_min),
            (unsigned int)(pool_overhead + block_size_max),
            (unsigned int)(pool_overhead + pool_bytes));
#endif
        return 0;
    }

    /*
    ** Create the main free block. Offset the start of the block slightly
    ** so that the prev_phys_block field falls outside of the pool -
    ** it will never be used.
    */
    block = offset_to_block(mem_pool, (-(memptr_t)block_header_overhead));
    block_set_size(block, pool_bytes);
    block_set_free(block);
    block_set_prev_used(block);
    block_insert(mem_cast(control_t*, mem), block);

    /* Split the block to create a zero-size sentinel block. */
    next = block_link_next(block);
    block_set_size(next, 0);
    block_set_used(next);
    block_set_prev_free(next);
    return mem_pool;
}
/**
 * @brief Initialize a memory bank as a memory pool.
 *
 * @param bank
 * @param mutex
 * @param mem_pool
 * @param mem_pool_size
 */
void memory_pool(mem_bank_t bank, rte_mutex_t *mutex, void *mem_pool, size_t mem_pool_size)
{
    mem_handle[bank].mutex = mutex;
    mem_t mem = mem_create(mem_pool);
    mem_handle[bank].pool = mem_add_pool(mem, (char*)mem_pool + sizeof(control_t), mem_pool_size - sizeof(control_t));
    mem_handle[bank].mem = mem;
}
/**
 * @brief Alloc a size of memory stack.
 *
 * @param bank
 * @param size
 * @return void*
 */
void* memory_alloc(mem_bank_t bank, size_t size)
{
    void *p = NULL;
    MEM_LOCK(bank);
    if(size) {
        control_t* control = mem_cast(control_t*, mem_handle[bank].mem);
        const size_t adjust = adjust_request_size(size, ALIGN_SIZE);
        block_header_t* block = block_locate_free(control, adjust);
        p = block_prepare_used(control, block, adjust);
    }
    MEM_UNLOCK(bank);
    return p;
}
/**
 * @brief Alloc a size of memory stack and set it to zero.
 *
 * @param bank
 * @param size
 * @return void*
 */
void *memory_calloc(mem_bank_t bank, size_t size)
{
    void *ret = 0;
    ret = memory_alloc(bank,size);
    if(ret && size)
        memset(ret,0,size);
    return ret;
}
/**
 * @brief Alloc a size of aligned memory stack.
 *
 * @param bank
 * @param align
 * @param size
 * @return void*
 */
void* memory_alloc_align(mem_bank_t bank, size_t align, size_t size)
{
    MEM_LOCK(bank);
    void *retval = NULL;
    if(size) {
        control_t* control = mem_cast(control_t*, mem_handle[bank].mem);
        const size_t adjust = adjust_request_size(size, ALIGN_SIZE);
        /*
        ** We must allocate an additional minimum block size bytes so that if
        ** our free block will leave an alignment gap which is smaller, we can
        ** trim a leading free block and release it back to the pool. We must
        ** do this because the previous physical block is in use, therefore
        ** the prev_phys_block field is not valid, and we can't simply adjust
        ** the size of that block.
        */
        const size_t gap_minimum = sizeof(block_header_t);
        const size_t size_with_gap = adjust_request_size(adjust + align + gap_minimum, align);
        /*
        ** If alignment is less than or equals base alignment, we're done.
        ** If we requested 0 bytes, return null, as mem_malloc(0) does.
        */
        const size_t aligned_size = (adjust && align > ALIGN_SIZE) ? size_with_gap : adjust;
        block_header_t* block = block_locate_free(control, aligned_size);
        /* This can't be a static assert. */
        MEM_ASSERT(sizeof(block_header_t) == block_size_min + block_header_overhead);
        if (block) {
            void* ptr = block_to_ptr(block);
            void* aligned = align_ptr(ptr, align);
            size_t gap = mem_cast(size_t, mem_cast(memptr_t, aligned) - mem_cast(memptr_t, ptr));
            /* If gap size is too small, offset to next aligned boundary. */
            if (gap && gap < gap_minimum) {
                const size_t gap_remain = gap_minimum - gap;
                const size_t offset = RTE_MAX(gap_remain, align);
                const void* next_aligned = mem_cast(void*,
                    mem_cast(memptr_t, aligned) + offset);
                aligned = align_ptr(next_aligned, align);
                gap = mem_cast(size_t,
                    mem_cast(memptr_t, aligned) - mem_cast(memptr_t, ptr));
            }
            if (gap) {
                MEM_ASSERT(gap >= gap_minimum);
                block = block_trim_free_leading(control, block, gap);
            }
        }
        retval = block_prepare_used(control, block, adjust);
    }
    MEM_UNLOCK(bank);
    return retval;
}
/**
 * @brief Free a allocated memory stack.
 *
 * @param bank
 * @param ptr
 */
void memory_free(mem_bank_t bank,void* ptr)
{
    /* Don't attempt to free a NULL pointer. */
    if (ptr) {
        MEM_LOCK(bank);
        control_t* control = mem_cast(control_t*, mem_handle[bank].mem);
        block_header_t* block = block_from_ptr(ptr);
        if(!block_is_free(block)) {
            block_mark_as_free(block);
            block = block_merge_prev(control, block);
            block = block_merge_next(control, block);
            block_insert(control, block);
        }
        MEM_UNLOCK(bank);
    }
}
/*
** The mem block information provides us with enough information to
** provide a reasonably intelligent implementation of realloc, growing or
** shrinking the currently allocated block as required.
**
** This routine handles the somewhat esoteric edge cases of realloc:
** - a non-zero size with a null pointer will behave like malloc
** - a zero size with a non-null pointer will behave like free
** - a request that cannot be satisfied will leave the original buffer
**   untouched
** - an extended buffer size will leave the newly-allocated area with
**   contents undefined
*/
void* memory_realloc(mem_bank_t bank, void* ptr, size_t size)
{
    MEM_LOCK(bank);
    /* Protect the critical section... */
    void* p = NULL;
    /* Zero-size requests are treated as free. */
    if (ptr && size == 0) {
        memory_free(bank, ptr);
    } else if (!ptr) { /* Requests with NULL pointers are treated as malloc. */
        if(size) {
            control_t* control = mem_cast(control_t*, mem_handle[bank].mem);
            const size_t adjust = adjust_request_size(size, ALIGN_SIZE);
            block_header_t* block = block_locate_free(control, adjust);
            p = block_prepare_used(control, block, adjust);
        }
    } else {
        block_header_t* block = block_from_ptr(ptr);
        block_header_t* next = block_next(block);
        const size_t cursize = block_size(block);
        const size_t combined = cursize + block_size(next) + block_header_overhead;
        const size_t adjust = adjust_request_size(size, ALIGN_SIZE);
        MEM_ASSERT(!block_is_free(block));
        /*
        ** If the next block is used, or when combined with the current
        ** block, does not offer enough space, we must reallocate and copy.
        */
        if (adjust > cursize && (!block_is_free(next) || adjust > combined)) {
            if(size) {
                control_t* control = mem_cast(control_t*, mem_handle[bank].mem);
                block = block_locate_free(control, adjust);
                p = block_prepare_used(control, block, adjust);
            }
            if (p) {
                const size_t minsize = RTE_MIN(cursize, size);
                memcpy(p, ptr, minsize);
                memory_free(bank, ptr);
            }
        } else {
            control_t* control = mem_cast(control_t*, mem_handle[bank].mem);
            /* Do we need to expand to the next block? */
            if (adjust > cursize) {
                block_merge_next(control, block);
                block_mark_as_used(block);
            }
            /* Trim the resulting block and return the original pointer. */
            block_trim_used(control, block, adjust);
            p = ptr;
        }
    }
    MEM_UNLOCK(bank);
    return p;
}
static void print_block(void* ptr, size_t size, int used)
{
    MEM_LOGI("%p %s size: %d (%p)", ptr, used ? "used" : "free", (unsigned int)size, block_from_ptr(ptr));
}
/**
 * @brief Demon a bank of memory stack.
 *
 * @param ptr
 * @return size_t
 */
void memory_demon(mem_bank_t bank)
{
    if(bank >= BANK_CNT)
        return;
    block_header_t* block =
        offset_to_block(mem_handle[bank].pool, -(int)block_header_overhead);
    MEM_LOGI("--------------------------------------------------");
    MEM_LOGI("BANK%d start at %p",bank, mem_handle[bank].pool);
    MEM_LOGI("--------------------------------------------------");
    while (block && !block_is_last(block)) {
        print_block(block_to_ptr(block), block_size(block), !block_is_free(block));
        block = block_next(block);
    }
}
/**
 * @brief Get a malloced buffer's size.
 *
 * @param ptr
 * @return size_t
 */
size_t memory_sizeof_p(void *ptr)
{
    size_t size = 0;
    if (ptr) {
        const block_header_t* block = block_from_ptr(ptr);
        size = block_size(block);
    }
    return size;
}
/**
 * @brief Get a free size of a memory bank.
 *
 * @param bank
 * @return size_t
 */
size_t memory_sizeof_free(mem_bank_t bank)
{
    block_header_t* block =
        offset_to_block(mem_handle[bank].pool, -(int)block_header_overhead);
    size_t freesize = 0;
    while (block && !block_is_last(block)) {
        if(block_is_free(block))
            freesize += block_size(block);
        block = block_next(block);
    }
    return freesize;
}
/**
 * @brief Get a max size of a memory bank.
 *
 * @param bank
 * @return size_t
 */
size_t memory_sizeof_max(mem_bank_t bank)
{
    size_t nowsize = 0;
    size_t maxsize = 0;
    block_header_t* block = offset_to_block(mem_handle[bank].pool, -(int)block_header_overhead);
    while (block && !block_is_last(block)) {
        nowsize = block_size(block);
        if((nowsize > maxsize)&&block_is_free(block))
            maxsize = nowsize;
        block = block_next(block);
    }
    return maxsize;
}
/**
 * @brief Malloc a max free size of a memory bank.
 *
 * @param bank
 * @param size
 * @return void*
 */
void *memory_alloc_max(mem_bank_t bank,size_t *size)
{
    MEM_LOCK(bank);
    void *p_retval = NULL;
    /* Protect the critical section... */
    block_header_t* retval = NULL;
    block_header_t* block = offset_to_block(mem_handle[bank].pool, -(int)block_header_overhead);
    size_t nowsize = 0;
    size_t maxsize = 0;
    while (block && !block_is_last(block)) {
        nowsize = block_size(block);
        if((nowsize > maxsize)&&block_is_free(block)) {
            maxsize = nowsize;
            retval = block;
        }
        block = block_next(block);
    }
    *size = maxsize;
    if(maxsize > 0) {
        block_mark_as_used(retval);
        p_retval = (void *)block_to_ptr(retval);
    }
    /* Release the critical section... */
    MEM_UNLOCK(bank);
    return p_retval;
}
#else

typedef union {
#if RTE_MEMPOOL_USE_64BIT
    struct {
        uint64_t used: 1;       //1: if the entry is used
        uint64_t d_size: 63;    //Size off the data (1 means 4 bytes)
    };
    uint64_t header;            //The header (used + d_size)
#else
    struct {
        uint32_t used: 1;       //1: if the entry is used
        uint32_t d_size: 31;    //Size off the data (1 means 4 bytes)
    };
    uint32_t header;            //The header (used + d_size)
#endif
} mem_header_t;

typedef struct {
    mem_header_t header;
    uint8_t first_data;         /*First data byte in the allocated data (Just for easily create a pointer)*/
} mem_entry_t;

typedef struct
{
    uint8_t *work_mem;          //Work memory for allocation
    uint32_t totalsize;
    rte_mutex_t *mutex;
} mem_handle_t;

static mem_entry_t  *ent_get_next(mem_bank_t mem_name, mem_entry_t *act_e);
static void *ent_alloc(mem_entry_t * e, uint32_t size);
static void ent_trunc(mem_entry_t * e, uint32_t size);

/* The handle for memory */
static mem_handle_t mem_handle[BANK_CNT] = {
    {NULL, 0, NULL},
};

/**
 * @brief Initialize a memory bank as a memory pool.
 *
 * @param bank
 * @param mutex
 * @param mem_pool
 * @param mem_pool_size
 */
void memory_pool(mem_bank_t bank, rte_mutex_t *mutex, void *mem_pool, size_t mem_pool_size)
{
    mem_handle[bank].mutex = mutex;
	mem_handle[bank].work_mem = (uint8_t *)mem_pool;
	mem_handle[bank].totalsize = mem_pool_size;
	mem_entry_t *full = (mem_entry_t *)mem_handle[bank].work_mem;
	full->header.used = 0;
	/*The total mem size id reduced by the first header and the close patterns */
	full->header.d_size = mem_pool_size - sizeof(mem_header_t);
}
/**
 * @brief Alloc a size of memory stack.
 *
 * @param bank
 * @param size
 * @return void*
 */
void* memory_alloc(mem_bank_t bank, size_t size)
{
    void *p = NULL;
    if(size) {
        MEM_LOCK(bank);
#if RTE_MEMPOOL_USE_64BIT
        /*Round the size up to 8*/
        if(size & 0x7) {
            size = size & (~0x7);
            size += 8;
        }
#else
        /*Round the size up to 4*/
        if(size & 0x3) {
            size = size & (~0x3);
            size += 4;
        }
#endif
        mem_entry_t * e = NULL;
        //Search for a appropriate entry
        do {
            //Get the next entry
            e = ent_get_next(bank,e);
            /*If there is next entry then try to allocate there*/
            if(e != NULL) {
                p = ent_alloc(e, size);
            }
            //End if there is not next entry OR the alloc. is successful
        } while(e != NULL && p == NULL);
        MEM_UNLOCK(bank);
    }
    return p;
}
/**
 * @brief Alloc a size of memory stack and set it to zero.
 *
 * @param bank
 * @param size
 * @return void*
 */
void *memory_calloc(mem_bank_t bank, size_t size)
{
    void *ret = 0;
    ret = memory_alloc(bank,size);
    if(ret&&size)
        memset(ret,0,size);
    return ret;
}
/**
 * @brief Free a allocated memory stack.
 *
 * @param bank
 * @param ptr
 */
void memory_free(mem_bank_t bank, void* ptr)
{
    /* Don't attempt to free a NULL pointer. */
    if (ptr) {
        MEM_LOCK(bank);
        /*e points to the header*/
        mem_entry_t * e = (mem_entry_t *)((uint8_t *)ptr - sizeof(mem_header_t));
        e->header.used = 0;
        /* Make a simple defrag.
         * Join the following free entries after this*/
        mem_entry_t * e_next;
        e_next = ent_get_next(bank, e);
        while(e_next != NULL) {
            if(e_next->header.used == 0) {
                e->header.d_size += e_next->header.d_size + sizeof(e->header);
            } else {
                break;
            }
            e_next = ent_get_next(bank, e_next);
        }
        MEM_UNLOCK(bank);
    }
}
/**
 * @brief Realloc a malloced buffer.
 *
 * @param bank
 * @param ptr
 * @param size
 * @return void*
 */
void* memory_realloc(mem_bank_t bank, void* ptr, size_t size)
{
    void *p = NULL;
    MEM_LOCK(bank);
	/*ptr could be previously freed pointer (in this case it is invalid)*/
	if(ptr != NULL) {
		mem_entry_t * e = (mem_entry_t *)((uint8_t *) ptr - sizeof(mem_header_t));
		if(e->header.used == 0) {
			ptr = NULL;
		}
	}
    if (ptr && size == 0) {
        memory_free(bank, ptr);
    } else if (!ptr) {  /* Requests with NULL pointers are treated as malloc. */
        if(size) {
#if RTE_MEMPOOL_USE_64BIT
            /*Round the size up to 8*/
            if(size & 0x7) {
                size = size & (~0x7);
                size += 8;
            }
#else
            /*Round the size up to 4*/
            if(size & 0x3) {
                size = size & (~0x3);
                size += 4;
            }
#endif
            mem_entry_t * e = NULL;
            //Search for a appropriate entry
            do {
                //Get the next entry
                e = ent_get_next(bank,e);
                /*If there is next entry then try to allocate there*/
                if(e != NULL) {
                    p = ent_alloc(e, size);
                }
                //End if there is not next entry OR the alloc. is successful
            } while(e != NULL && p == NULL);
        }
    } else {
#if RTE_MEMPOOL_USE_64BIT
        /*Round the size up to 8*/
        if(size & 0x7) {
            size = size & (~0x7);
            size += 8;
        }
#else
        /*Round the size up to 4*/
        if(size & 0x3) {
            size = size & (~0x3);
            size += 4;
        }
#endif
        uint32_t old_size = memory_sizeof_p(ptr);
        if(old_size == size) {
            p = ptr;     /*Also avoid reallocating the same memory*/
        } else if(size < old_size) {
            /* Only truncate the memory is possible
               If the 'old_size' was extended by a header size in 'ent_trunc' it avoids reallocating this same memory */
            mem_entry_t * e = (mem_entry_t *)((uint8_t *) ptr - sizeof(mem_header_t));
            ent_trunc(e, size);
            p = &e->first_data;
        } else {
            mem_entry_t * e = NULL;
            //Search for a appropriate entry
            do {
                //Get the next entry
                e = ent_get_next(bank,e);
                /*If there is next entry then try to allocate there*/
                if(e != NULL) {
                    p = ent_alloc(e, size);
                }
                //End if there is not next entry OR the alloc. is successful
            } while(e != NULL && p == NULL);
            /*Copy the old data to the new. Use the smaller size*/
            if(p && old_size != 0) {
                memcpy(p, ptr, old_size);
                memory_free(bank, ptr);
            }
        }
    }
    MEM_UNLOCK(bank);
    return p;
}
/**
 * @brief Get a malloced buffer's size.
 *
 * @param ptr
 * @return size_t
 */
size_t memory_sizeof_p(void *ptr)
{
	if(ptr == NULL) return 0;
	mem_entry_t *e = (mem_entry_t *)((uint8_t *)ptr - sizeof(mem_header_t));
	return e->header.d_size;
}
/**
 * Give the next entry after 'act_e'
 * @param act_e pointer to an entry
 * @return pointer to an entry after 'act_e'
 */
static mem_entry_t * ent_get_next(mem_bank_t mem_name,mem_entry_t * act_e)
{
	mem_entry_t * next_e = NULL;
	if(act_e == NULL) { /*NULL means: get the first entry*/
		next_e = (mem_entry_t *) mem_handle[mem_name].work_mem;
	} else { /*Get the next entry */
		uint8_t * data = &act_e->first_data;
		next_e = (mem_entry_t *)&data[act_e->header.d_size];
		if(&next_e->first_data >= &mem_handle[mem_name].work_mem[mem_handle[mem_name].totalsize]) next_e = NULL;
	}
	return next_e;
}
/**
 * Try to do the real allocation with a given size
 * @param e try to allocate to this entry
 * @param size size of the new memory in bytes
 * @return pointer to the allocated memory or NULL if not enough memory in the entry
 */
static void *ent_alloc(mem_entry_t * e, uint32_t size)
{
	void * alloc = NULL;
	/*If the memory is free and big enough then use it */
	if(e->header.used == 0 && e->header.d_size >= size) {
		/*Truncate the entry to the desired size */
		ent_trunc(e, size),
		e->header.used = 1;
		/*Save the allocated data*/
		alloc = &e->first_data;
	}
	return alloc;
}

/**
 * Truncate the data of entry to the given size
 * @param e Pointer to an entry
 * @param size new size in bytes
 */
static void ent_trunc(mem_entry_t * e, uint32_t size)
{
	/*Don't let empty space only for a header without data*/
	if(e->header.d_size == size + sizeof(mem_header_t)) {
		size = e->header.d_size;
	}
	/* Create the new entry after the current if there is space for it */
	if(e->header.d_size != size) {
		uint8_t * e_data = &e->first_data;
		mem_entry_t * after_new_e = (mem_entry_t *)&e_data[size];
		after_new_e->header.used = 0;
		after_new_e->header.d_size = e->header.d_size - size - sizeof(mem_header_t);
	}
	/* Set the new size for the original entry */
	e->header.d_size = size;
}

#endif
