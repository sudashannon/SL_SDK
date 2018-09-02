#include "APP_RingBuffer.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define RB_INDH(rb)                ((rb)->head & ((rb)->count - 1))
#define RB_INDT(rb)                ((rb)->tail & ((rb)->count - 1))

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Initialize ring buffer */
int APP_RingBuffer_Init(APP_RingBuff_t *RingBuff, void *buffer, int itemSize, int count)
{
	RingBuff->data = buffer;
	RingBuff->count = count;
	RingBuff->itemSz = itemSize;
	RingBuff->head = RingBuff->tail = 0;

	return 1;
}

/* Insert a single item into Ring Buffer */
int APP_RingBuffer_Insert(APP_RingBuff_t *RingBuff, const void *data)
{
	uint8_t *ptr = RingBuff->data;

	/* We cannot insert when queue is full */
	if (APP_RingBuffer_IsFull(RingBuff))
		return 0;

	ptr += RB_INDH(RingBuff) * RingBuff->itemSz;
	memcpy(ptr, data, RingBuff->itemSz);
	RingBuff->head++;

	return 1;
}

/* Insert multiple items into Ring Buffer */
int APP_RingBuffer_InsertMult(APP_RingBuff_t *RingBuff, const void *data, int num)
{
	uint8_t *ptr = RingBuff->data;
	int cnt1, cnt2;

	/* We cannot insert when queue is full */
	if (APP_RingBuffer_IsFull(RingBuff))
		return 0;

	/* Calculate the segment lengths */
	cnt1 = cnt2 = APP_RingBuffer_GetFree(RingBuff);
	if (RB_INDH(RingBuff) + cnt1 >= RingBuff->count)
		cnt1 = RingBuff->count - RB_INDH(RingBuff);
	cnt2 -= cnt1;

	cnt1 = min(cnt1, num);
	num -= cnt1;

	cnt2 = min(cnt2, num);
	num -= cnt2;

	/* Write segment 1 */
	ptr += RB_INDH(RingBuff) * RingBuff->itemSz;
	memcpy(ptr, data, cnt1 * RingBuff->itemSz);
	RingBuff->head += cnt1;

	/* Write segment 2 */
	ptr = (uint8_t *) RingBuff->data + RB_INDH(RingBuff) * RingBuff->itemSz;
	data = (const uint8_t *) data + cnt1 * RingBuff->itemSz;
	memcpy(ptr, data, cnt2 * RingBuff->itemSz);
	RingBuff->head += cnt2;

	return cnt1 + cnt2;
}

/* Pop single item from Ring Buffer */
int APP_RingBuffer_Pop(APP_RingBuff_t *RingBuff, void *data)
{
	uint8_t *ptr = RingBuff->data;

	/* We cannot pop when queue is empty */
	if (APP_RingBuffer_IsEmpty(RingBuff))
		return 0;

	ptr += RB_INDT(RingBuff) * RingBuff->itemSz;
	memcpy(data, ptr, RingBuff->itemSz);
	RingBuff->tail++;

	return 1;
}

/* Pop multiple items from Ring buffer */
int APP_RingBuffer_PopMult(APP_RingBuff_t *RingBuff, void *data, int num)
{
	uint8_t *ptr = RingBuff->data;
	int cnt1, cnt2;

	/* We cannot insert when queue is empty */
	if (APP_RingBuffer_IsEmpty(RingBuff))
		return 0;

	/* Calculate the segment lengths */
	cnt1 = cnt2 = APP_RingBuffer_GetCount(RingBuff);
	if (RB_INDT(RingBuff) + cnt1 >= RingBuff->count)
		cnt1 = RingBuff->count - RB_INDT(RingBuff);
	cnt2 -= cnt1;

	cnt1 = min(cnt1, num);
	num -= cnt1;

	cnt2 = min(cnt2, num);
	num -= cnt2;

	/* Write segment 1 */
	ptr += RB_INDT(RingBuff) * RingBuff->itemSz;
	memcpy(data, ptr, cnt1 * RingBuff->itemSz);
	RingBuff->tail += cnt1;

	/* Write segment 2 */
	ptr = (uint8_t *) RingBuff->data + RB_INDT(RingBuff) * RingBuff->itemSz;
	data = (uint8_t *) data + cnt1 * RingBuff->itemSz;
	memcpy(data, ptr, cnt2 * RingBuff->itemSz);
	RingBuff->tail += cnt2;

	return cnt1 + cnt2;
}

bool APP_MessageQuene_Init(APP_MessageQuene_t *MessageQuene, uint16_t Size)
{
	MessageQuene->QueneBuffer = APP_BGetz(MEM_0,Size);
	if(!MessageQuene->QueneBuffer)
		return false;
	if(APP_RingBuffer_Init(&MessageQuene->RingBuff,MessageQuene->QueneBuffer,sizeof(uint8_t),Size))
		return true;
	else
		return false;
}
APP_MessgaeQuene_Err_e APP_MessageQuene_In(APP_MessageQuene_t *MessageQuene, uint8_t *Data,uint16_t DataSize)
{
	uint8_t SizeHigh = (DataSize>>8)&0xFF;
	if(APP_RingBuffer_Insert(&MessageQuene->RingBuff,&SizeHigh))
	{
		uint8_t SizeLow = DataSize&0xFF;
		if(APP_RingBuffer_Insert(&MessageQuene->RingBuff,&SizeLow))
		{
			if(APP_RingBuffer_InsertMult(&MessageQuene->RingBuff,Data,DataSize) == DataSize)
				return MSG_NO_ERR;
		}
	}
	return MSG_EN_FULL;
}
APP_MessgaeQuene_Err_e APP_MessageQuene_Out(APP_MessageQuene_t *MessageQuene, uint8_t *Data,uint16_t *DataSize)
{
	uint8_t SizeHigh = 0x00;
	uint8_t SizeLow = 0x00;
	if(APP_RingBuffer_Pop(&MessageQuene->RingBuff,&SizeHigh))
	{
		if(APP_RingBuffer_Pop(&MessageQuene->RingBuff,&SizeLow))
		{
			*DataSize = (uint16_t)(SizeHigh<<8)|SizeLow;
			if(APP_RingBuffer_PopMult(&MessageQuene->RingBuff,Data,*DataSize) == *DataSize)
			{
				uint16_t AnotherFrameSize=0;
				if(Data[*DataSize-1] == 0xAA&& Data[*DataSize-2] == 0x55)
				{
					APP_MessageQuene_Out(MessageQuene,Data+*DataSize-2,&AnotherFrameSize);
					*DataSize = AnotherFrameSize + *DataSize-2;
				}
				return MSG_NO_ERR;
			}
		}
	}
	return MSG_DE_EMPTY;
}
