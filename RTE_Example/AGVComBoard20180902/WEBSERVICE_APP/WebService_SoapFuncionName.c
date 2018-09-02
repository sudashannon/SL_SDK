#include "WebService_SoapFuncionName.h"
const char left[] = {"<"};
const char right[] = {">"};
const char slash[] = {"/"};
const char* SoapRequestFucList[SOAPFUCCOUNT] = {
	"cs2:ApplyResource", //资源申请
	"cs2:ExcuteTask", //任务执行
	"cs2:ReleaseResource", //资源释放
};
const char* SoapResponseFucList[SOAPFUCCOUNT] = {
	"ns:ApplyResourceRes", //资源申请
	"ns:ApplyForExecutionRes", //任务执行	
	"ns:ReleaseResourceRes", //资源释放
};
const char* StringResponseFucList[SOAPFUCCOUNT] = {
	"ApplyResourceRes", //资源申请
	"ApplyForExecutionRes", //任务执行	
	"ReleaseResourceRes", //资源释放
};
const char* ApplyResourceArgList[ARFUCARGCOUNT] = {
	"MsgTypeCode",//消息类型
	"LiftId",//电梯编号
	"TaskDetailKey",//任务编号
	"Destination",//AGV目标楼层
	"InstantLocation",//AGC所在楼层
};
const char* ApplyResourceResPonseArgList[ARFUCRESARGCOUNT] = {
	"MsgTypeCode",//消息类型
	"Error",//错误码
	"ErrMsg",//错误码对应的错误信息
	"LiftId",//请求参数中的电梯编号
	"TaskDetailKey",//请求中的任务唯一标识
};
const char* ApplyForExecutionArgList[AFFUCARGCOUNT] = {
	"MsgTypeCode",//消息类型
	"LiftId",//电梯编号
	"TaskDetailKey",//任务编号
	"Destination",//AGV目标楼层
	"InstantLocation",//AGC所在楼层
	"ServiceUrl",//WebService服务地址
};
const char* ApplyForExecutionResPonseArgList[AFFUCRESARGCOUNT] = {
	"MsgTypeCode",//消息类型
	"Error",//错误码
	"ErrMsg",//错误码对应的错误信息
	"OldLiftId",//请求参数中的电梯编号
	"OldTaskDetailKey",//请求中的任务唯一标识（该字段只有当出现当前执行的任务编号与请求执行的任务不一致时，才会有该字段，告知客户端）
};
const char* ReleaseResourceArgList[RRFUCARGCOUNT] = {
	"MsgTypeCode",//消息类型
	"LiftId",//电梯编号
	"TaskDetailKey",//任务编号
};
const char* ReleaseResourceResArgList[RRFUCRESARGCOUNT] = {
	"MsgTypeCode",//消息类型
	"Error",//错误码
	"ErrMsg",//错误码对应的错误信息
};
const char* ClientSoapRequestFucList[CLIENTSOAPFUCCOUNT] = {
	"urn:LiftService-FiniTask", //任务结束
	"urn:LiftService-ErrMsg", //错误消息
};
const char* StringRequestFucList[CLIENTSOAPFUCCOUNT] = {
	"FinishTask", //资源申请
	"ErrorMessage", //任务执行	
};
const char* FinishTaskArgList[FTFUCARGCOUNT] = {
	"MsgTypeCode",//消息类型
	"LiftId",//电梯编号
	"TaskDetailKey",//任务编号
	"InstantLocation",//AGC所在楼层
};
const char* FinishTaskResponseArgList[FTFUCRESARGCOUNT] = {
	"MsgTypeCode",//消息类型
	"LiftId",//电梯编号
	"TaskDetailKey",//任务编号
};
const char* ErrorMessageArgList[EMFUCARGCOUNT] = {
	"MsgTypeCode",//消息类型
	"LiftId",//电梯编号
	"TaskDetailKey",//任务编号
	"Error",//错误码
	"ErrMsg",//错误码对应的错误信息
};
const char* ErrorMessageResponseArgList[EMFUCRESARGCOUNT] = {
	"MsgTypeCode",//消息类型
	"Error",//错误码
	"ErrMsg",//错误码对应的错误信息
};
const AGVTaskError_t ErrorCode[]=
{
	{"0","NoError"},
	{"1","TaskDetailKeyDifferent"},
	{"2","FloorInforDifferent"},
	{"3","ApplyWithoutRelease"},
	{"4","NoTaskExcute"},
	{"5","NoTaskRelease"},
	{"6","NotAGVMode"},
	{"7","CanApplyFail"},
	{"8","CanExcuteFail"},
	{"9","CanReleaseFail"},
};
