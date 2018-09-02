
/*
 * dlove_HttpSend.h
 *
 *  Created on: 2013年12月25日
 *      Author: root
 */
 
#ifndef DLOVE_HTTPSEND_H_
#define DLOVE_HTTPSEND_H_
 
#include "http_string.h"
 
struct ___Dlove_HTTPSend{
    struct ___Dlove_String httpText;
    struct ___Dlove_String postArgbuf;      /* post 参数会首先放在这个地方 */
};
 
/* 功能：初始化一个httpSend对象 */
void ___Dlove_httpSendInit(struct ___Dlove_HTTPSend *_httpSend);
 
/* 功能：设置http请求报文的请求行
 * 参数：
 *  ver,http版本号,"1.0"/"1.1"
 * 注意：url 是直接追加到 http 报文后面的,所以格式一定要正确 */
void ___Dlove_httpSendSetReqLine(struct ___Dlove_HTTPSend *_httpSend,
        const char *_method,
        const char *_url,
        const char *_ver);
 /* 功能：设置http响应报文的响应行
 * 参数：
 *  ver,版本号,"1.0"/"1.1"  num，序号
 * 注意：url 是直接追加到 http 报文后面的,所以格式一定要正确 */
void ___Dlove_httpSendSetHTTPLine(struct ___Dlove_HTTPSend *_httpSend,
        const char *_ver,
        const char *_num);
/* 功能：添加 http 的首部字段 */
void ___Dlove_httpSendAddHead(struct ___Dlove_HTTPSend *_httpSend,
        const char *_name,
        const char *_value);
 
/* 功能：添加post参数 */
void ___Dlove_httpSendAddPostArg(struct ___Dlove_HTTPSend *_httpSend,
        const char *_name,
        const char *_value);
 
/* 功能：调用这个函数标志着http报文构建结束
 *  对于没有设置post参数的请求报文，函数只是简单的追加"\r\n"
 *  对于设置了post参数的请求报文，函数会自动设置 content-type,content-length；然后将post参数追加到报文之后 */
void ___Dlove_httpSendSetOver(struct ___Dlove_HTTPSend *_httpSend);
 
///* 功能：将构建完成的http请求报文通过套接字发送出去
// * 返回：发送成功返回0；否则返回1并设置errno*/
//int ___Dlove_httpSendTo(struct ___Dlove_HTTPSend *_httpSend,int sock);
// 
/* 功能：释放httpSend所占用的资源 */
void ___Dlove_httpSendClr(struct ___Dlove_HTTPSend *_httpSend);
 
#endif /* DLOVE_HTTPSEND_H_ */
