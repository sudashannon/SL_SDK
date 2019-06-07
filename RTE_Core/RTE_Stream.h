#ifndef __RTE_STREAM_H
#define __RTE_STREAM_H
#include "RTE_Config.h"
#if RTE_USE_STREAM == 1
#include <stdint.h>
typedef enum
{
	STREAM_0 = 0,
	STREAM_N,
}stream_name_e;
typedef enum
{
    STREAM_INVALID = -1,
    STREAM_SYNCHRO = 0,
    STREAM_ASYNCHRO,
    STREAM_COUNT,
}stream_type_e;
typedef enum
{
	STREAM_NO_ERR = 0x00,
	STREAM_EN_FULL = 0x01,
	STREAM_EN_NOMEM = 0x02,
	STREAM_DE_EMPTY = 0x03,
}stream_error_e;
//--------------------------------------------------------------
// 流数据结构体（采用动态内存分配）
//--------------------------------------------------------------
typedef struct
{
    uint8_t *buffer;
    uint16_t length;
}stream_data_t;
//--------------------------------------------------------------
// 流结构体
//--------------------------------------------------------------
typedef struct
{
	uint8_t front;
	uint8_t rear;
	uint8_t size;
#if RTE_USE_MEMORY == 0
	stream_name_e name;
#endif
	stream_data_t *data;
}stream_t;
extern void Stream_Init(
	stream_t *stream,
#if RTE_USE_MEMORY == 0
	stream_name_e name,
#endif
	uint8_t size);
extern void Stream_Clear(stream_t *stream);
extern stream_error_e Stream_Enquene(stream_t *stream,uint8_t *string,uint16_t length);
extern stream_error_e Stream_Dequene(stream_t *stream,uint8_t *string,uint16_t *length);
#endif
#endif
