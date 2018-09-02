#ifndef __WEBSERVICESOAPFUNCIONNAME_H
#define __WEBSERVICESOAPFUNCIONNAME_H
#define SOAPFUCCOUNT  3
typedef enum {
  Nofuncion=0,
	ApplyResource=1,
	ApplyForExecution=2,
	ReleaseResource=3,
}SoapFucName;
#define ARFUCARGCOUNT  5
#define ARFUCRESARGCOUNT  5

#define AFFUCARGCOUNT  6
#define AFFUCRESARGCOUNT  5

#define RRFUCARGCOUNT  3
#define RRFUCRESARGCOUNT  3

typedef enum {
	FinishTask=0,
	ErrorMessage,
}ClientSoapFucName;
#define CLIENTSOAPFUCCOUNT  2

#define FTFUCARGCOUNT  4
#define FTFUCRESARGCOUNT  3

#define EMFUCARGCOUNT  5
#define EMFUCRESARGCOUNT  3

typedef struct{
    char *Key;
    char *Value;
}AGVTaskError_t;

extern const char left[];
extern const char right[];
extern const char slash[];
extern const char* SoapRequestFucList[SOAPFUCCOUNT];
extern const char* SoapResponseFucList[SOAPFUCCOUNT];
extern const char* StringResponseFucList[SOAPFUCCOUNT];
extern const char* ApplyResourceArgList[ARFUCARGCOUNT];
extern const char* ApplyResourceResPonseArgList[ARFUCRESARGCOUNT];
extern const char* ApplyForExecutionArgList[AFFUCARGCOUNT];
extern const char* ApplyForExecutionResPonseArgList[AFFUCRESARGCOUNT];
extern const char* ReleaseResourceArgList[RRFUCARGCOUNT];
extern const char* ReleaseResourceResArgList[RRFUCRESARGCOUNT];
extern const char* ClientSoapRequestFucList[CLIENTSOAPFUCCOUNT];
extern const char* StringRequestFucList[CLIENTSOAPFUCCOUNT];
extern const char* FinishTaskArgList[FTFUCARGCOUNT];
extern const char* FinishTaskResponseArgList[FTFUCRESARGCOUNT];
extern const char* ErrorMessageArgList[EMFUCARGCOUNT];
extern const char* ErrorMessageResponseArgList[EMFUCRESARGCOUNT];
extern const AGVTaskError_t ErrorCode[];
#endif
