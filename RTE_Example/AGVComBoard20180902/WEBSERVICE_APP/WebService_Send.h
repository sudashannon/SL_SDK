#ifndef __WEBSERVICEDATASEND_H
#define __WEBSERVICEDATASEND_H
#include "RTE_Include.h"
#include "AGV_SM.h"
void WebService_ServerDataGenerate(uint8_t Sendtype,uint8_t *SendBuffer,AGV_SM_Arg_t *AGV_RunningTask);
void WebService_ClientDataGenerate(uint8_t Sendtype,uint8_t *SendBuffer,AGV_SM_Arg_t *AGV_RunningTask);
void WebService_ErrorGenerate(uint8_t ErrorType,AGV_SM_Arg_t *AGV_RunningTask);
#endif


