#ifndef __RING_BUFFER_H_
#define __RING_BUFFER_H_
/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif
#include "APP_Include.h"
/*
 * min()/max() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x, y) ({                            \
			 typeof(x) _min1 = (x);                  \
			 typeof(y) _min2 = (y);                  \
			 (void) (&_min1 == &_min2);              \
			 _min1 < _min2 ? _min1 : _min2; })
#define max(x, y) ({                            \
			 typeof(x) _max1 = (x);                  \
			 typeof(y) _max2 = (y);                  \
			 (void) (&_max1 == &_max2);              \
			 _max1 > _max2 ? _max1 : _max2; })
#define min3(x, y, z) ({                        \
			 typeof(x) _min1 = (x);                  \
			 typeof(y) _min2 = (y);                  \
			 typeof(z) _min3 = (z);                  \
			 (void) (&_min1 == &_min2);              \
			 (void) (&_min1 == &_min3);              \
			 _min1 < _min2 ? (_min1 < _min3 ? _min1 : _min3) : \
							 (_min2 < _min3 ? _min2 : _min3); })
#define max3(x, y, z) ({                        \
			 typeof(x) _max1 = (x);                  \
			 typeof(y) _max2 = (y);                  \
			 typeof(z) _max3 = (z);                  \
			 (void) (&_max1 == &_max2);              \
			 (void) (&_max1 == &_max3);              \
			 _max1 > _max2 ? (_max1 > _max3 ? _max1 : _max3) : \
							 (_max2 > _max3 ? _max2 : _max3); })
/** @defgroup Ring_Buffer CHIP: Simple ring buffer implementation
 * @ingroup CHIP_Common
 * @{
 */

/**
 * @brief Ring buffer structure
 */
typedef struct {
	void *data;
	int count;
	int itemSz;
	uint32_t head;
	uint32_t tail;
} APP_RingBuff_t;

/**
 * @def		RB_VHEAD(rb)
 * volatile typecasted head index
 */
#define RB_VHEAD(rb)              (*(volatile uint32_t *) &(rb)->head)

/**
 * @def		RB_VTAIL(rb)
 * volatile typecasted tail index
 */
#define RB_VTAIL(rb)              (*(volatile uint32_t *) &(rb)->tail)

/**
 * @brief	Initialize ring buffer
 * @param	RingBuff	: Pointer to ring buffer to initialize
 * @param	buffer		: Pointer to buffer to associate with RingBuff
 * @param	itemSize	: Size of each buffer item size
 * @param	count		: Size of ring buffer
 * @note	Memory pointed by @a buffer must have correct alignment of
 * 			@a itemSize, and @a count must be a power of 2 and must at
 * 			least be 2 or greater.
 * @return	Nothing
 */
int APP_RingBuffer_Init(APP_RingBuff_t *RingBuff, void *buffer, int itemSize, int count);

/**
 * @brief	Resets the ring buffer to empty
 * @param	RingBuff	: Pointer to ring buffer
 * @return	Nothing
 */
static inline void APP_RingBuffer_Flush(APP_RingBuff_t *RingBuff)
{
	RingBuff->head = RingBuff->tail = 0;
}

/**
 * @brief	Return size the ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @return	Size of the ring buffer in bytes
 */
static inline int APP_RingBuffer_GetSize(APP_RingBuff_t *RingBuff)
{
	return RingBuff->count;
}

/**
 * @brief	Return number of items in the ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @return	Number of items in the ring buffer
 */
static inline int APP_RingBuffer_GetCount(APP_RingBuff_t *RingBuff)
{
	return RB_VHEAD(RingBuff) - RB_VTAIL(RingBuff);
}

/**
 * @brief	Return number of free items in the ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @return	Number of free items in the ring buffer
 */
static inline int APP_RingBuffer_GetFree(APP_RingBuff_t *RingBuff)
{
	return RingBuff->count - APP_RingBuffer_GetCount(RingBuff);
}

/**
 * @brief	Return number of items in the ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @return	1 if the ring buffer is full, otherwise 0
 */
static inline int APP_RingBuffer_IsFull(APP_RingBuff_t *RingBuff)
{
	return (APP_RingBuffer_GetCount(RingBuff) >= RingBuff->count);
}

/**
 * @brief	Return empty status of ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @return	1 if the ring buffer is empty, otherwise 0
 */
static inline int APP_RingBuffer_IsEmpty(APP_RingBuff_t *RingBuff)
{
	return RB_VHEAD(RingBuff) == RB_VTAIL(RingBuff);
}

/**
 * @brief	Insert a single item into ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @param	data		: pointer to item
 * @return	1 when successfully inserted,
 *			0 on error (Buffer not initialized using
 *			APP_RingBuffer_Init() or attempted to insert
 *			when buffer is full)
 */
int APP_RingBuffer_Insert(APP_RingBuff_t *RingBuff, const void *data);

/**
 * @brief	Insert an array of items into ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @param	data		: Pointer to first element of the item array
 * @param	num			: Number of items in the array
 * @return	number of items successfully inserted,
 *			0 on error (Buffer not initialized using
 *			APP_RingBuffer_Init() or attempted to insert
 *			when buffer is full)
 */
int APP_RingBuffer_InsertMult(APP_RingBuff_t *RingBuff, const void *data, int num);

/**
 * @brief	Pop an item from the ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @param	data		: Pointer to memory where popped item be stored
 * @return	1 when item popped successfuly onto @a data,
 * 			0 When error (Buffer not initialized using
 * 			APP_RingBuffer_Init() or attempted to pop item when
 * 			the buffer is empty)
 */
int APP_RingBuffer_Pop(APP_RingBuff_t *RingBuff, void *data);

/**
 * @brief	Pop an array of items from the ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @param	data		: Pointer to memory where popped items be stored
 * @param	num			: Max number of items array @a data can hold
 * @return	Number of items popped onto @a data,
 * 			0 on error (Buffer not initialized using APP_RingBuffer_Init()
 * 			or attempted to pop when the buffer is empty)
 */
int APP_RingBuffer_PopMult(APP_RingBuff_t *RingBuff, void *data, int num);


typedef struct
{
	APP_RingBuff_t RingBuff;
	uint8_t *QueneBuffer;
}APP_MessageQuene_t;
typedef enum
{
	MSG_NO_ERR = 0x00,
	MSG_EN_FULL = 0x01,
	MSG_DE_EMPTY = 0x02,
}APP_MessgaeQuene_Err_e;
bool APP_MessageQuene_Init(APP_MessageQuene_t *MessageQuene, uint16_t Size);
APP_MessgaeQuene_Err_e APP_MessageQuene_In(APP_MessageQuene_t *MessageQuene, uint8_t *Data,uint16_t DataSize);
APP_MessgaeQuene_Err_e APP_MessageQuene_Out(APP_MessageQuene_t *MessageQuene, uint8_t *Data,uint16_t *DataSize);
/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* __RING_BUFFER_H_ */
