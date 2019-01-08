#include "RTE_RingQuene.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 3.0 2019.1.9
*** History: 1.0 创建，修改自tivaware
             2.0 为RTE的升级做适配
						 3.0 修改了出入逻辑，增加了互斥锁
*****************************************************************************/
#if RTE_USE_RINGQUENE == 1
#include "RTE_MEM.h"
#include "RTE_MATH.h"
#include "RTE_LOG.h"
#define RINGQUENE_STR "[RINGQUE]"
#define RB_INDH(rb)                ((rb)->head & ((rb)->count - 1))
#define RB_INDT(rb)                ((rb)->tail & ((rb)->count - 1))
/*************************************************
*** Args:   
			*RingBuff 待处理环形队列
			*buffer 实际占用的内存空间
			itemSize 单个存储单元大小
			count 队列容量
*** Function: 初始化一个环形队列
*************************************************/
int RTE_RingQuene_Init(RTE_RingQuene_t *RingBuff, void *buffer, int itemSize, int count)
{
	RingBuff->data = buffer;
	RingBuff->count = count;
	RingBuff->itemSz = itemSize;
	RingBuff->head = RingBuff->tail = 0;
	return 1;
}
/*************************************************
*** Args:   
			*RingBuff 待处理环形队列
      *data 待插入数据
*** Function: 向环形队列尾部添加一个数据
*************************************************/
int RTE_RingQuene_Insert(RTE_RingQuene_t *RingBuff, const void *data)
{
	uint8_t *ptr = RingBuff->data;
	/* We cannot insert when queue is full */
	if (RTE_RingQuene_IsFull(RingBuff))
		return 0;
	ptr += RB_INDH(RingBuff) * RingBuff->itemSz;
	memcpy(ptr, data, RingBuff->itemSz);
	RingBuff->head++;
	return 1;
}
/*************************************************
*** Args:   
			*RingBuff 待处理环形队列
      *data 待插入数据首地址
		   num 待插入数据个数
*** Function: 向环形队列尾部添加若干个数据
*************************************************/
int RTE_RingQuene_InsertMult(RTE_RingQuene_t *RingBuff, const void *data, int num)
{
	uint8_t *ptr = RingBuff->data;
	int cnt1, cnt2;
	/* We cannot insert when queue is full */
	if (RTE_RingQuene_IsFull(RingBuff))
		return 0;
	/* Calculate the segment lengths */
	cnt1 = cnt2 = RTE_RingQuene_GetFree(RingBuff);
	if (RB_INDH(RingBuff) + cnt1 >= RingBuff->count)
		cnt1 = RingBuff->count - RB_INDH(RingBuff);
	cnt2 -= cnt1;
	cnt1 = MATH_MIN(cnt1, num);
	num -= cnt1;
	cnt2 = MATH_MIN(cnt2, num);
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
/*************************************************
*** Args:   
			*RingBuff 待处理环形队列
      *data 存放数据的地址
*** Function: 从环形队列头部取出一个数据
*************************************************/
int RTE_RingQuene_Pop(RTE_RingQuene_t *RingBuff, void *data)
{
	uint8_t *ptr = RingBuff->data;
	/* We cannot pop when queue is empty */
	if (RTE_RingQuene_IsEmpty(RingBuff))
		return 0;
	ptr += RB_INDT(RingBuff) * RingBuff->itemSz;
	memcpy(data, ptr, RingBuff->itemSz);
	RingBuff->tail++;
	return 1;
}
/*************************************************
*** Args:   
			*RingBuff 待处理环形队列
      *data 存放数据的地址
			num 去除数据个数
*** Function: 从环形队列头部取出一个数据
*************************************************/
int RTE_RingQuene_PopMult(RTE_RingQuene_t *RingBuff, void *data, int num)
{
	uint8_t *ptr = RingBuff->data;
	int cnt1, cnt2;
	/* We cannot insert when queue is empty */
	if (RTE_RingQuene_IsEmpty(RingBuff))
		return 0;
	/* Calculate the segment lengths */
	cnt1 = cnt2 = RTE_RingQuene_GetCount(RingBuff);
	if (RB_INDT(RingBuff) + cnt1 >= RingBuff->count)
		cnt1 = RingBuff->count - RB_INDT(RingBuff);
	cnt2 -= cnt1;
	cnt1 = MATH_MIN(cnt1, num);
	num -= cnt1;
	cnt2 = MATH_MIN(cnt2, num);
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
/*************************************************
*** Args:   
			*MessageQuene 待处理消息队列
      Size 消息队列大小
*** Function: 初始化一个消息队列
*************************************************/
void RTE_MessageQuene_Init(RTE_MessageQuene_t *MessageQuene, uint16_t Size)
{
	MessageQuene->QueneBuffer = (uint8_t *)RTE_MEM_Alloc0(MEM_RTE,Size);
	RTE_AssertParam(MessageQuene->QueneBuffer);
	RTE_AssertParam(RTE_RingQuene_Init(&MessageQuene->RingBuff,MessageQuene->QueneBuffer,sizeof(uint8_t),Size));
}
/*************************************************
*** Args:   
			*MessageQuene 待处理消息队列
      *Data 待加入数据首地址
			DataSize 待加入数据大小
*** Function: 往消息队列中加入一个数据
*************************************************/
RTE_MessageQuene_Err_e RTE_MessageQuene_In(RTE_MessageQuene_t *MessageQuene, uint8_t *Data,uint16_t DataSize)
{
	RTE_MessageQuene_Err_e RetVal = MSG_NO_ERR;
	if(RTE_RingQuene_GetSize(&MessageQuene->RingBuff)>=3)
	{
		uint8_t SizeHigh = (DataSize>>8)&0xFF;
		RTE_RingQuene_Insert(&MessageQuene->RingBuff,&SizeHigh);
		uint8_t SizeLow = DataSize&0xFF;
		RTE_RingQuene_Insert(&MessageQuene->RingBuff,&SizeLow);
		if(RTE_RingQuene_InsertMult(&MessageQuene->RingBuff,Data,DataSize) != DataSize)
		{
			RTE_LOG_ERROR(RINGQUENE_STR,"In data not enough");
			RetVal = MSG_NOTSAME;
		}
	}
	else
	{
		RTE_LOG_ERROR(RINGQUENE_STR,"In error");
		RetVal = MSG_EN_FULL;
	}
	return RetVal;
}
/*************************************************
*** Args:   
			*MessageQuene 待处理消息队列
      *Data 存放数据空间首地址
			*DataSize 存放数据大小变量地址
*** Function: 从消息队列中取出一个数据
*************************************************/
RTE_MessageQuene_Err_e RTE_MessageQuene_Out(RTE_MessageQuene_t *MessageQuene, uint8_t *Data,uint16_t *DataSize)
{
	RTE_MessageQuene_Err_e RetVal = MSG_NO_ERR;
	uint8_t SizeHigh = 0x00;
	uint8_t SizeLow = 0x00;	
	if(RTE_RingQuene_Pop(&MessageQuene->RingBuff,&SizeHigh)&&RTE_RingQuene_Pop(&MessageQuene->RingBuff,&SizeLow))
	{
		*DataSize = (uint16_t)(SizeHigh<<8)|SizeLow;
		if(RTE_RingQuene_PopMult(&MessageQuene->RingBuff,Data,*DataSize) != *DataSize)
		{
			RTE_LOG_ERROR(RINGQUENE_STR,"Out data not enough");
			RetVal = MSG_NOTSAME;
		}
	}
	else
	{
		RetVal = MSG_DE_EMPTY;
	}
	return RetVal;
}
#endif
