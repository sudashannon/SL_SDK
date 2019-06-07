/**
  ******************************************************************************
  * @file    RTE_Stream.c
  * @author  Shan Lei ->>lurenjia.tech ->>https://github.com/sudashannon
  * @brief   基于环形队列的数据流
  * @version V1.1 2019/06/07
  * @History V1.0 创建
             V1.1 为动态内存管理环境做适配
  ******************************************************************************
  */
#include "RTE_Stream.h"
#if RTE_USE_STREAM == 1
#if RTE_USE_MEMORY == 1
#include "RTE_Memory.h"
#else
static stream_data_t streamarray[STREAM_N][STATIC_QUENE_SIZE] = {0};
static uint8_t streambuffer[STREAM_N][STATIC_BUFFER_SIZE] = {0};
#endif
#if STREAM_DEBUG == 1
#include "RTE_Printf.h"
#endif
#define STREAM_STR "[STREAM]"
#include <string.h>
#include <stdbool.h>
void Stream_Init(
	stream_t *stream,
#if RTE_USE_MEMORY == 0
	stream_name_e name,
#endif
	uint8_t size)
{
	stream->front = 0;
	stream->rear = 0;
	stream->size = size;
#if RTE_USE_MEMORY == 0
	RTE_AssertParam(name<STREAM_N);
	stream->name = name;
	stream->data = streamarray[name];
#else
	stream->data = (stream_data_t *)Memory_Alloc(MEM_RTE,size*sizeof(stream_data_t));
	RTE_AssertParam(stream->data);
#endif
	for(uint8_t i=0;i<stream->size;i++)
	{
		stream->data[i].buffer = (void *)0;
		stream->data[i].length = 0;
	}
}
void Stream_Clear(stream_t *stream)
{
	for(uint8_t i=0;i<stream->size;i++)
	{
		if(stream->data[i].buffer)
		{
#if RTE_USE_MEMORY == 0
			stream->data[i].buffer = (void *)0;
#else
			Memory_Free(MEM_RTE,stream->data[i].buffer);
			stream->data[i].buffer = (void *)0;
#endif
		}
		stream->data[i].length = 0;
	}
	stream->front = 0;
	stream->rear = 0;
}
static inline bool Stream_IsFull(stream_t *stream)
{
	return(((stream->rear+1)%stream->size == stream->front)?true:false);
}
static inline bool Stream_IsEmpty(stream_t *stream)
{
	return((stream->front == stream->rear)?true:false);
}
stream_error_e Stream_Enquene(stream_t *stream,uint8_t *string,uint16_t length)
{
	if(Stream_IsFull(stream))
	{
#if STREAM_DEBUG == 1
		uprintf("%10s    No extra quene for this stream\r\n",STREAM_STR);
#endif
		return STREAM_EN_FULL;
	}
	else
	{
#if RTE_USE_MEMORY == 0
		stream->data[stream->rear].buffer = streambuffer[stream->name];
#else
		stream->data[stream->rear].buffer = (uint8_t *)Memory_Alloc(MEM_RTE,length);
#endif
		if(stream->data[stream->front].buffer != (void *)0)
		{
			memcpy((uint8_t *)stream->data[stream->rear].buffer,string,length);		
			stream->data[stream->rear].length = length;
			stream->rear = (stream->rear+1)%stream->size;
		}
		else
		{
#if STREAM_DEBUG == 1
			uprintf("%10s    No extra mem for this stream with data lenth:%d\r\n",STREAM_STR,length);
#endif
			return STREAM_EN_NOMEM;
		}
	}
	return STREAM_NO_ERR;
}
stream_error_e Stream_Dequene(stream_t *stream,uint8_t *string,uint16_t *length)
{
	if(Stream_IsEmpty(stream))
		return STREAM_DE_EMPTY;
	else
	{
		if(length)
			*length = stream->data[stream->front].length;
		memcpy(string,(uint8_t *)stream->data[stream->front].buffer,stream->data[stream->front].length);
		if(stream->data[stream->front].buffer!=(void *)0)
		{
#if RTE_USE_MEMORY == 0
			memset(stream->data[stream->rear].buffer,0,stream->data[stream->front].length);
			stream->data[stream->front].buffer = (void *)0;
#else
			Memory_Free(MEM_RTE,stream->data[stream->front].buffer);
			stream->data[stream->front].buffer = (void *)0;
#endif
		}
		stream->data[stream->front].length = 0;
		stream->front = (stream->front+1)%stream->size;
	}
	return STREAM_NO_ERR;
}
#endif
