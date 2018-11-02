#ifndef __RTE_RINGQUENE_H
#define __RTE_RINGQUENE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Include.h"
#if RTE_USE_RINGQUENE
typedef struct {
	void *data;
	int count;
	int itemSz;
	uint32_t head;
	uint32_t tail;
} RTE_RingQuene_t;
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
int RTE_RingQuene_Init(RTE_RingQuene_t *RingBuff, void *buffer, int itemSize, int count);
/**
 * @brief	Resets the ring buffer to empty
 * @param	RingBuff	: Pointer to ring buffer
 * @return	Nothing
 */
static inline void RTE_RingQuene_Flush(RTE_RingQuene_t *RingBuff)
{
	RingBuff->head = RingBuff->tail = 0;
}
/**
 * @brief	Return size the ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @return	Size of the ring buffer in bytes
 */
static inline int RTE_RingQuene_GetSize(RTE_RingQuene_t *RingBuff)
{
	return RingBuff->count;
}
/**
 * @brief	Return number of items in the ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @return	Number of items in the ring buffer
 */
static inline int RTE_RingQuene_GetCount(RTE_RingQuene_t *RingBuff)
{
	return RB_VHEAD(RingBuff) - RB_VTAIL(RingBuff);
}
/**
 * @brief	Return number of free items in the ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @return	Number of free items in the ring buffer
 */
static inline int RTE_RingQuene_GetFree(RTE_RingQuene_t *RingBuff)
{
	return RingBuff->count - RTE_RingQuene_GetCount(RingBuff);
}
/**
 * @brief	Return number of items in the ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @return	1 if the ring buffer is full, otherwise 0
 */
static inline int RTE_RingQuene_IsFull(RTE_RingQuene_t *RingBuff)
{
	return (RTE_RingQuene_GetCount(RingBuff) >= RingBuff->count);
}
/**
 * @brief	Return empty status of ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @return	1 if the ring buffer is empty, otherwise 0
 */
static inline int RTE_RingQuene_IsEmpty(RTE_RingQuene_t *RingBuff)
{
	return RB_VHEAD(RingBuff) == RB_VTAIL(RingBuff);
}
/**
 * @brief	Insert a single item into ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @param	data		: pointer to item
 * @return	1 when successfully inserted,
 *			0 on error (Buffer not initialized using
 *			RTE_RingQuene_Init() or attempted to insert
 *			when buffer is full)
 */
int RTE_RingQuene_Insert(RTE_RingQuene_t *RingBuff, const void *data);
/**
 * @brief	Insert an array of items into ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @param	data		: Pointer to first element of the item array
 * @param	num			: Number of items in the array
 * @return	number of items successfully inserted,
 *			0 on error (Buffer not initialized using
 *			RTE_RingQuene_Init() or attempted to insert
 *			when buffer is full)
 */
int RTE_RingQuene_InsertMult(RTE_RingQuene_t *RingBuff, const void *data, int num);
/**
 * @brief	Pop an item from the ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @param	data		: Pointer to memory where popped item be stored
 * @return	1 when item popped successfuly onto @a data,
 * 			0 When error (Buffer not initialized using
 * 			RTE_RingQuene_Init() or attempted to pop item when
 * 			the buffer is empty)
 */
int RTE_RingQuene_Pop(RTE_RingQuene_t *RingBuff, void *data);
/**
 * @brief	Pop an array of items from the ring buffer
 * @param	RingBuff	: Pointer to ring buffer
 * @param	data		: Pointer to memory where popped items be stored
 * @param	num			: Max number of items array @a data can hold
 * @return	Number of items popped onto @a data,
 * 			0 on error (Buffer not initialized using RTE_RingQuene_Init()
 * 			or attempted to pop when the buffer is empty)
 */
int RTE_RingQuene_PopMult(RTE_RingQuene_t *RingBuff, void *data, int num);
typedef struct
{
	RTE_RingQuene_t RingBuff;
	uint8_t *QueneBuffer;
}RTE_MessageQuene_t;
typedef enum
{
	MSG_NO_ERR = 0x00,
	MSG_EN_FULL = 0x01,
	MSG_DE_EMPTY = 0x02,
}RTE_MessageQuene_Err_e;
extern void RTE_MessageQuene_Init(RTE_MessageQuene_t *MessageQuene, uint16_t Size);
extern RTE_MessageQuene_Err_e RTE_MessageQuene_In(RTE_MessageQuene_t *MessageQuene, uint8_t *Data,uint16_t DataSize);
extern RTE_MessageQuene_Err_e RTE_MessageQuene_Out(RTE_MessageQuene_t *MessageQuene, uint8_t *Data,uint16_t *DataSize);		
#endif
#ifdef __cplusplus
}
#endif
#endif

