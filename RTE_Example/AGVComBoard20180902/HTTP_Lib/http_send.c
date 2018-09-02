/*
 * dlove_HttpSend.c
 *
 *  Created on: 2013年12月25日
 *      Author: root
 */
 
#include "http_send.h"
#include <string.h>
#include <stdio.h>
 
void ___Dlove_httpSendInit(struct ___Dlove_HTTPSend *_httpSend){
    ___Dlove_String_init(&_httpSend->httpText,1024,512);
    ___Dlove_String_init(&_httpSend->postArgbuf,256,128);
    return ;
}
 
#define ___Dlove_mAddText(strch)    ___Dlove_String_strcatch(&_httpSend->httpText,strch);
void ___Dlove_httpSendSetReqLine(struct ___Dlove_HTTPSend *_httpSend,
        const char *_method,
        const char *_url,
        const char *_ver)
{
    ___Dlove_mAddText(_method);
    ___Dlove_mAddText(" ");
    ___Dlove_mAddText(_url);
    ___Dlove_mAddText(" ");
    ___Dlove_mAddText("HTTP/");
    ___Dlove_mAddText(_ver);
    ___Dlove_mAddText("\r\n");
    return ;
}
void ___Dlove_httpSendSetHTTPLine(struct ___Dlove_HTTPSend *_httpSend,
        const char *_ver,
        const char *_num)
{
    ___Dlove_mAddText("HTTP/");
	  ___Dlove_mAddText(_ver);
    ___Dlove_mAddText(" ");
    ___Dlove_mAddText(_num);
    ___Dlove_mAddText(" ");
    ___Dlove_mAddText("OK");
    ___Dlove_mAddText("\r\n");
    return ;
}
 
void ___Dlove_httpSendAddHead(struct ___Dlove_HTTPSend *_httpSend,
        const char *_name,
        const char *_value)
{
    ___Dlove_mAddText(_name);
    ___Dlove_mAddText(": ");
    ___Dlove_mAddText(_value);
    ___Dlove_mAddText("\r\n");
    return ;
}
 
#define ___Dlove_mAddPostText(strch)    ___Dlove_String_strcatch(&_httpSend->postArgbuf,strch);
void ___Dlove_httpSendAddPostArg(struct ___Dlove_HTTPSend *_httpSend,
        const char *_name,
        const char *_value)
{
    ___Dlove_mAddPostText(_name);
    ___Dlove_mAddPostText("=");
    ___Dlove_mAddPostText(_value);
    ___Dlove_mAddPostText("&");
    /* 这个有可能多添加一个'&' */
    return ;
}
 
/* 功能：将_value按照10进制转换为字符串 */
void ___Dlove_itoa(char *_buf,int _value){
    sprintf(_buf,"%d",_value);
    return ;
}
 
void ___Dlove_httpSendSetOver(struct ___Dlove_HTTPSend *_httpSend){
    /* 添加过 post 参数 */
    if(_httpSend->postArgbuf.size != 0){
        char lenbuf[12]={0}; /* int 类型的最大值转化为字符串后为10位 */
        /* 获取post实体的总长度并转换为字符串 */
        _httpSend->postArgbuf.buf[--_httpSend->postArgbuf.size]='\0'; /* 清除在多设置的'&' */
        ___Dlove_itoa(lenbuf,_httpSend->postArgbuf.size);
 
        /* 设置'Content-Type'与'Content-Length'域 */
        ___Dlove_httpSendAddHead(_httpSend,"Content-Type","application/x-www-form-urlencoded");
        ___Dlove_httpSendAddHead(_httpSend,"Content-Length",lenbuf);
        ___Dlove_mAddText("\r\n");
 
        ___Dlove_String_strcatstr(&_httpSend->httpText,&_httpSend->postArgbuf);
    }else
        ___Dlove_mAddText("\r\n");
 		___Dlove_mAddText("\r\n");
    return ;
}
 
//int ___Dlove_httpSendTo(struct ___Dlove_HTTPSend *_httpSend,int sock){
//    if(write(sock,_httpSend->httpText.buf,_httpSend->httpText.size) < 0)
//        return 1;
// 
//    return 0;
//}
 
void ___Dlove_httpSendClr(struct ___Dlove_HTTPSend *_httpSend){
    ___Dlove_String_release(&_httpSend->httpText);
    ___Dlove_String_release(&_httpSend->postArgbuf);
    return ;
}
