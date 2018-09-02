#ifndef __WEBSERVICE_PARSER_H
#define __WEBSERVICE_PARSER_H
#include "RTE_Include.h"
#include "WebService_SoapFuncionName.h"
typedef struct
{
	uint8_t MsgTypeCode[50];
	uint8_t LiftId[50];
	uint8_t TaskDetailKey[50];
	uint8_t Destination[10];
	uint8_t InstantLocation[10];
}ApplyTempInfor_t;
typedef struct
{
	uint8_t AGVServerIP[4];
	uint8_t MsgTypeCode[50];
	uint8_t OldLiftId[50];
	uint8_t TaskDetailKey[50];
	uint8_t Destination[10];
	uint8_t InstantLocation[10];
	uint8_t ServiceUrl[50];
	uint16_t AGVServerPortNum;
}ExcuteTempInfor_t;
typedef struct
{
	uint8_t MsgTypeCode[50];
	uint8_t LiftId[50];
	uint8_t TaskDetailKey[50];
	uint8_t AGVServerIP[4];
	uint16_t AGVServerPortNum;
}ReleaseTempInfor_t;
extern uint8_t ParserName;
extern ApplyTempInfor_t ApplyTempInfor;
extern ExcuteTempInfor_t ExcuteTempInfor;
extern ReleaseTempInfor_t ReleaseTempInfor;
void WebServiceDataParser(uint8_t *Data,uint16_t DataLenth);
#endif
