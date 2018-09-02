/*
 * ___Dlove_String.h
 *
 *  Created on: 2013年12月24日
 *      Author: root
 */
 
#ifndef DLOVE_STRING_H_
#define DLOVE_STRING_H_
 
/* C字符串类型 */
struct ___Dlove_String{
    char    *buf;
    int     capacity;       /* 字符串的容量,即字符串最大可以容纳的字符个数 */
    int     size;           /* 字符串中字符数 */
    int     increment;      /* 当字符串大小不足时,需要重新分配空间,新分配 increment 字节的空间  */
};
 
/* 功能：初始化一个 str 对象,负责初始内存的分配
 * 参数：
 *  _capacity: 指定了字符串的初始容量,大于 0
 *  _increment: 指定了字符串的每次重新分配空间时的增量 */
void ___Dlove_String_init(struct ___Dlove_String *_str,int _capacity,int _increment);
 
/* 功能：将 str 添加到 string 的未尾
 * 注意：
 *  如果 string 的容量不足,则会自动分配 */
void ___Dlove_String_strcatch(struct ___Dlove_String *string,const char *str);
void ___Dlove_String_strcatstr(struct ___Dlove_String *string,const struct ___Dlove_String *str);
 
/* 功能：释放字符串 _str 所占用的空间
 * 注意：此后 _str->buf==NULL,_str->size与_str->capacity初始化为0,_str->increment不变
 *  所以仍然可以重新使用这个变量 */
void ___Dlove_String_release(struct ___Dlove_String *_str);
 
/* 接口 */
#define String          struct ___Dlove_String
#define strCatch        ___Dlove_String_strcatch
#define strCatstr       ___Dlove_String_strcatstr
#define strInit         ___Dlove_String_init
#define strClr          ___Dlove_String_release
 
#endif /* DLOVE_STRING_H_ */
