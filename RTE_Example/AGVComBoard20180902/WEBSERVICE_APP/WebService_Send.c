#include "WebService_Send.h"
#include "WebService_SoapFuncionName.h"
#include "http_send.h"
#include "mxml.h"
#define TCP_SOAPBUF_LENTH   1536
#define TCP_HTTPBUF_LENTH   512
static char WebService_SendTempData[512] = {0};
static uint8_t WebService_SoapBuf[TCP_SOAPBUF_LENTH] = {0};
void WebService_ServerDataGenerate(uint8_t Sendtype,uint8_t *SendBuffer,AGV_SM_Arg_t *AGV_RunningTask)
{
	mxml_node_t *xml;    /* <?xml ... ?> */
	mxml_node_t *envelope;   /* <data> */
	mxml_node_t *header;   /* <data> */
	mxml_node_t *body;   /* <data> */
	mxml_node_t *response;   /* <data> */
	mxml_node_t *resultstr;   /* <data> */
	//xml类型
	xml = mxmlNewXML("1.0");
	//envelope部分
	envelope = mxmlNewElement(xml, "SOAP-ENV:Envelope");
	mxmlElementSetAttr(envelope, "xmlns:SOAP-ENV" , "http://schemas.xmlsoap.org/soap/envelope/");
	mxmlElementSetAttr(envelope, "xmlns:SOAP-ENC" , "http://schemas.xmlsoap.org/soap/encoding/");
	mxmlElementSetAttr(envelope, "xmlns:xsi" , "http://www.w3.org/2001/XMLSchema-instance");
	mxmlElementSetAttr(envelope, "xmlns:xsd" , "http://www.w3.org/2001/XMLSchema");
	mxmlElementSetAttr(envelope, "xmlns:ns" , "http://tempuri.org/ns.xsd");
	mxmlElementSetAttr(envelope, "xmlns:tpsms" , "urn:tpsms");
	//header和body部分
	header = mxmlNewElement(envelope, "SOAP-ENV:Header");
	mxmlNewText(header, 0, "");
	body = mxmlNewElement(envelope, "SOAP-ENV:Body");
	//response和str部分
	response = mxmlNewElement(body,SoapResponseFucList[Sendtype-1]);
	resultstr = mxmlNewElement(response,"result-str");
	switch(Sendtype)
	{
		case ApplyResource:
		{
			uint8_t i=0;
			//这里需要保证resultstr足够大
			//首先增加返回头
			memset(WebService_SendTempData,0,512);
			strcat(WebService_SendTempData,left);
			strcat(WebService_SendTempData,StringResponseFucList[Sendtype-1]);
			strcat(WebService_SendTempData,right);
			//接下来增加参数
			for(i=0;i<ARFUCRESARGCOUNT;i++)
			{
				//<开始>
				strcat(WebService_SendTempData,left);
				strcat(WebService_SendTempData,ApplyResourceResPonseArgList[i]);
				strcat(WebService_SendTempData,right);
				/******************************************************************
				*******************************参数处理****************************
				******************************************************************/
				switch(i)
				{
					case 0:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->MsgTypeCode);
					}break;
					case 1:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->Error);
					}break;
					case 2:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->ErrMsg);
					}break;
					case 3:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->LiftId);
					}break;
					case 4:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->TaskDetailKey);
					}break;
				}
				//<结束>
				strcat(WebService_SendTempData,left);
				strcat(WebService_SendTempData,slash);
				strcat(WebService_SendTempData,ApplyResourceResPonseArgList[i]);
				strcat(WebService_SendTempData,right);
			}
			//最后增加结束头
			strcat(WebService_SendTempData,left);
			strcat(WebService_SendTempData,slash);
			strcat(WebService_SendTempData,StringResponseFucList[Sendtype-1]);
			strcat(WebService_SendTempData,right);
			mxmlNewText(resultstr, 0, WebService_SendTempData);
		}break;
		case ApplyForExecution:
		{
			memset(WebService_SendTempData,0,512);
			uint8_t i=0;
			//这里需要保证resultstr足够大
			//首先增加返回头
			strcat(WebService_SendTempData,left);
			strcat(WebService_SendTempData,StringResponseFucList[Sendtype-1]);
			strcat(WebService_SendTempData,right);
			//接下来增加参数
			for(i=0;i<AFFUCRESARGCOUNT;i++)
			{
				//<开始>
				strcat(WebService_SendTempData,left);
				if(i==4)
				{
					if(strlen((char*)AGV_RunningTask->OldsTaskDetailKey))
						strcat(WebService_SendTempData,ApplyForExecutionResPonseArgList[i]);
				}
				else
					strcat(WebService_SendTempData,ApplyForExecutionResPonseArgList[i]);
				strcat(WebService_SendTempData,right);
				/******************************************************************
				*******************************参数处理****************************
				******************************************************************/
				switch(i)
				{
					case 0:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->MsgTypeCode);
					}break;
					case 1:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->Error);
						//strcat(WebService_SendTempData,(char *)"0");
					}break;
					case 2:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->ErrMsg);
						//strcat(WebService_SendTempData,(char *)"NoErr");
					}break;
					case 3:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->OldLiftId);
					}break;
					case 4:
					{
						if(strlen((char*)AGV_RunningTask->OldsTaskDetailKey))
							strcat(WebService_SendTempData,(char *)AGV_RunningTask->OldsTaskDetailKey);
					}break;
				}
				//<结束>
				strcat(WebService_SendTempData,left);
				strcat(WebService_SendTempData,slash);
				if(i==4)
				{
					if(strlen((char*)AGV_RunningTask->OldsTaskDetailKey))
					{
						strcat(WebService_SendTempData,ApplyForExecutionResPonseArgList[i]);
					}
				}
				else
					strcat(WebService_SendTempData,ApplyForExecutionResPonseArgList[i]);
				strcat(WebService_SendTempData,right);
			}
			//最后增加结束头
			strcat(WebService_SendTempData,left);
			strcat(WebService_SendTempData,slash);
			strcat(WebService_SendTempData,StringResponseFucList[Sendtype-1]);
			strcat(WebService_SendTempData,right);
			mxmlNewText(resultstr, 0, WebService_SendTempData);
		}break;
		case ReleaseResource:
		{
			memset(WebService_SendTempData,0,512);
			uint8_t i=0;
			//这里需要保证resultstr足够大
			//首先增加返回头
			strcat(WebService_SendTempData,left);
			strcat(WebService_SendTempData,StringResponseFucList[Sendtype-1]);
			strcat(WebService_SendTempData,right);
			//接下来增加参数
			for(i=0;i<RRFUCRESARGCOUNT;i++)
			{
				//<开始>
				strcat(WebService_SendTempData,left);
				strcat(WebService_SendTempData,ReleaseResourceResArgList[i]);
				strcat(WebService_SendTempData,right);
				/******************************************************************
				*******************************参数处理****************************
				******************************************************************/
				switch(i)
				{
					case 0:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->MsgTypeCode);
					}break;
					case 1:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->Error);
					}break;
					case 2:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->ErrMsg);
					}break;
				}
				//<结束>
				strcat(WebService_SendTempData,left);
				strcat(WebService_SendTempData,slash);
				strcat(WebService_SendTempData,ReleaseResourceResArgList[i]);
				strcat(WebService_SendTempData,right);
			}
			//最后增加结束头
			strcat(WebService_SendTempData,left);
			strcat(WebService_SendTempData,slash);
			strcat(WebService_SendTempData,StringResponseFucList[Sendtype-1]);
			strcat(WebService_SendTempData,right);
			mxmlNewText(resultstr, 0, WebService_SendTempData);
		}break;
	}
	//清空数据
	memset(WebService_SoapBuf,0,(uint8_t)sizeof(WebService_SoapBuf));
	//拷贝数据
  mxmlSaveString(xml, (char *)WebService_SoapBuf, TCP_SOAPBUF_LENTH, MXML_NO_CALLBACK);
	mxmlRemove(resultstr);
	mxmlDelete(resultstr);
	mxmlRemove(response);
	mxmlDelete(response);
	mxmlRemove(body);
	mxmlDelete(body);
	mxmlRemove(header);
	mxmlDelete(header);
	mxmlRemove(envelope);
	mxmlDelete(envelope);
	mxmlRemove(xml);
	mxmlDelete(xml);
	
	struct ___Dlove_HTTPSend http;
	char soapbuflen[20]={0};
	___Dlove_httpSendInit(&http);
	___Dlove_httpSendSetHTTPLine(&http,"1.1","200");
	___Dlove_httpSendAddHead(&http,"Accept-Ranges","bytes");
	___Dlove_httpSendAddHead(&http,"Content-Type","application/soap+xml; charset=utf-8");
	sprintf(soapbuflen,"%d",strlen((char *)WebService_SoapBuf));
	___Dlove_httpSendAddHead(&http,"Content-Length",soapbuflen);
	___Dlove_httpSendSetOver(&http);
	//清空所需要的数据
	memset(SendBuffer,0,(uint8_t)sizeof(SendBuffer));
	//拷贝数据
	sprintf((char *)SendBuffer,"%s%s",
			(char *)http.httpText.buf,
			(char *)WebService_SoapBuf);
	___Dlove_httpSendClr(&http);
}
void WebService_ClientDataGenerate(uint8_t Sendtype,uint8_t *SendBuffer,AGV_SM_Arg_t *AGV_RunningTask)
{
	mxml_node_t *xml;    /* <?xml ... ?> */
	mxml_node_t *envelope;   /* <data> */
	mxml_node_t *header;   /* <data> */
	mxml_node_t *body;   /* <data> */
	mxml_node_t *request;   /* <data> */
	mxml_node_t *req;   /* <data> */
	mxml_node_t *resultstr;   /* <data> */
	xml = mxmlNewXML("1.0");

	envelope = mxmlNewElement(xml, "soapenv:Envelope");
	mxmlElementSetAttr(envelope, "xmlns:soapenv" , "http://schemas.xmlsoap.org/soap/envelope/");
	mxmlElementSetAttr(envelope, "xmlns:urn" , "urn:tpsms");
	header = mxmlNewElement(envelope, "soapenv:Header");
	body = mxmlNewElement(envelope, "soapenv:Body");
	
	request = mxmlNewElement(body,ClientSoapRequestFucList[Sendtype]);
	req = mxmlNewElement(request,"req");
	resultstr = mxmlNewElement(req,"strReqInfo");
	switch(Sendtype)
	{
		case FinishTask:
		{
			memset(WebService_SendTempData,0,512);
			uint8_t i=0;
			//这里需要保证resultstr足够大
			//首先增加返回头
			strcat(WebService_SendTempData,left);
			strcat(WebService_SendTempData,StringRequestFucList[Sendtype]);
			strcat(WebService_SendTempData,right);
			//接下来增加参数
			for(i=0;i<FTFUCARGCOUNT;i++)
			{
				//<开始>
				strcat(WebService_SendTempData,left);
				strcat(WebService_SendTempData,FinishTaskArgList[i]);
				strcat(WebService_SendTempData,right);
				/******************************************************************
				*******************************参数处理****************************
				******************************************************************/
				switch(i)
				{
					case 0:
					{
						strcat(WebService_SendTempData,(char *)"cmd_FinishTask");
					}break;
					case 1:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->LiftId);
					}break;
					case 2:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->TaskDetailKey);
					}break;
					case 3:
					{
						char TempInstantLocation[20] = {};
						sprintf(TempInstantLocation,"%d",AGV_RunningTask->TaskLocation);
						strcat(WebService_SendTempData,TempInstantLocation);
					}break;
				}
				//<结束>
				strcat(WebService_SendTempData,left);
				strcat(WebService_SendTempData,slash);
				strcat(WebService_SendTempData,FinishTaskArgList[i]);
				strcat(WebService_SendTempData,right);
			}
			//最后增加结束头
			strcat(WebService_SendTempData,left);
			strcat(WebService_SendTempData,slash);
			strcat(WebService_SendTempData,StringRequestFucList[Sendtype]);
			strcat(WebService_SendTempData,right);
			mxmlNewText(resultstr, 0, WebService_SendTempData);
		}break;
		case ErrorMessage:
		{
			memset(WebService_SendTempData,0,512);
			uint8_t i=0;
			//这里需要保证resultstr足够大
			//首先增加返回头
			strcat(WebService_SendTempData,left);
			strcat(WebService_SendTempData,StringRequestFucList[Sendtype]);
			strcat(WebService_SendTempData,right);
			//接下来增加参数
			for(i=0;i<EMFUCARGCOUNT;i++)
			{
				//<开始>
				strcat(WebService_SendTempData,left);
				strcat(WebService_SendTempData,ErrorMessageArgList[i]);
				strcat(WebService_SendTempData,right);
				/******************************************************************
				*******************************参数处理****************************
				******************************************************************/
				switch(i)
				{
					case 0:
					{
						strcat(WebService_SendTempData,(char *)"cmd_ExceptionTask");
					}break;
					case 1:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->LiftId);
					}break;
					case 2:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->TaskDetailKey);
					}break;
					case 3:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->Error);
					}break;
					case 4:
					{
						strcat(WebService_SendTempData,(char *)AGV_RunningTask->ErrMsg);
					}break;
				}
				//<结束>
				strcat(WebService_SendTempData,left);
				strcat(WebService_SendTempData,slash);
				strcat(WebService_SendTempData,ErrorMessageArgList[i]);
				strcat(WebService_SendTempData,right);
			}
			//最后增加结束头
			strcat(WebService_SendTempData,left);
			strcat(WebService_SendTempData,slash);
			strcat(WebService_SendTempData,StringRequestFucList[Sendtype]);
			strcat(WebService_SendTempData,right);
			mxmlNewText(resultstr, 0, WebService_SendTempData);
		}break;
	}
	
	//清空数据
	memset(WebService_SoapBuf,0,(uint8_t)sizeof(WebService_SoapBuf));
	//拷贝数据
  mxmlSaveString(xml, (char *)WebService_SoapBuf, TCP_SOAPBUF_LENTH, MXML_NO_CALLBACK);
	mxmlRemove(resultstr);
	mxmlDelete(resultstr);
	mxmlRemove(req);
	mxmlDelete(req);
	mxmlRemove(request);
	mxmlDelete(request);
	mxmlRemove(body);
	mxmlDelete(body);
	mxmlRemove(header);
	mxmlDelete(header);
	mxmlRemove(envelope);
	mxmlDelete(envelope);
	mxmlRemove(xml);
	mxmlDelete(xml);
	
	struct ___Dlove_HTTPSend post;
	char soapbuflen[20]={0};
	char serverinfor[30] = {0};
	sprintf(serverinfor,"%d.%d.%d.%d:%d",AGV_RunningTask->ServerIP[0],
	AGV_RunningTask->ServerIP[1],AGV_RunningTask->ServerIP[2],AGV_RunningTask->ServerIP[3],AGV_RunningTask->ServerPortNum);
	___Dlove_httpSendInit(&post);
	___Dlove_httpSendSetReqLine(&post,"POST","/ws/tpsms","1.1");
	___Dlove_httpSendAddHead(&post,"Accept-Encoding","gzip,deflate");
	___Dlove_httpSendAddHead(&post,"Content-Type","text/xml; charset=utf-8");
	___Dlove_httpSendAddHead(&post,"SOAPAction","\"\"");
	___Dlove_httpSendAddHead(&post,"Host",serverinfor);
	___Dlove_httpSendAddHead(&post,"User-Agent","TM4C1294-HTTPClient/1.0");
	sprintf(soapbuflen,"%d",strlen((char *)WebService_SoapBuf));
	___Dlove_httpSendAddHead(&post,"Content-Length",soapbuflen);
	___Dlove_httpSendSetOver(&post);

	//清空所需要的数据
	memset(SendBuffer,0,(uint8_t)sizeof(SendBuffer));
	//拷贝数据
	sprintf((char *)SendBuffer,"%s%s",
			(char *)post.httpText.buf,
			(char *)WebService_SoapBuf);
	___Dlove_httpSendClr(&post);
}
void WebService_ErrorGenerate(uint8_t ErrorType,AGV_SM_Arg_t *AGV_RunningTask)
{
	if(ErrorType!=0)
		printf("[WEBSERVICE]    准备回复错误%s\n",ErrorCode[ErrorType].Value);
	memset(AGV_RunningTask->Error,0,10);
	memcpy(AGV_RunningTask->Error,ErrorCode[ErrorType].Key,10);
	memset(AGV_RunningTask->ErrMsg,0,10);
	memcpy(AGV_RunningTask->ErrMsg,ErrorCode[ErrorType].Value,50);
}
