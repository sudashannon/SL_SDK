#ifndef GUI_PORT_H
#define GUI_PORT_H
#include "RTE_Config.h"
#include "RTE_RoundRobin.h"
#include "RTE_Memory.h"
#include "RTE_MATH.h"
#include "RTE_LinkList.h"
/*********************
 *  时基部分重定向
 *********************/
#define lv_tick_get                              RTE_RoundRobin_GetTick
#define lv_tick_elaps                            RTE_RoundRobin_TickElaps
#define lv_task_create(name,task,time,pri,arg)   RTE_RoundRobin_CreateTimer(1,name,time,1,1,task,arg);
/*********************
 *  内存管理重定向
 *********************/
#define lv_mem_alloc(size)        Memory_Alloc(BANK_RTE,size)
#define lv_mem_free(data)         Memory_Free(BANK_RTE,data)
#define lv_mem_assert             RTE_AssertParam
#define lv_mem_realloc(data,size) Memory_Realloc(BANK_RTE,data,size)
#define lv_mem_get_size(data)     Memory_GetDataSize(data)
/*********************
 *  链表重定向
 *********************/
#define lv_ll_t               RTE_LL_t
#define lv_ll_init(l,size)    RTE_LL_Init(l,size)
#define lv_ll_ins_head(l)     RTE_LL_InsHead(l)
#define lv_ll_get_head(l)     RTE_LL_GetHead(l)
#define lv_ll_get_next(l,a)   RTE_LL_GetNext(l,a)
#define lv_ll_rem(l,n)        RTE_LL_Remove(l,n)
#define LL_READ(l,i)          RTE_LL_READ(l, i)
#define LL_READ_BACK(l,i)     RTE_LL_READ_BACK(l, i)
#define lv_ll_ins_prev(l,n_act)     RTE_LL_InsPrev(l,n_act)
#define lv_ll_ins_tail(l)	    RTE_LL_InsTail(l)
#define lv_ll_clear(l)	      RTE_LL_Clear(l)
#define lv_ll_get_tail(l)	    RTE_LL_GetTail(l)
#define lv_ll_get_prev(l,n_act)	RTE_LL_GetPrev(l,n_act)
#define lv_ll_chg_list(l1,l2,n)     RTE_LL_MoveList(l1,l2,n)
/*********************
 *  日志重定向
 *********************/
#if 0
#define lv_log_add(level, file, line, dsc) RTE_LOG_ADD(level,"[GUI]",file, line, dsc)
#define LV_LOG_TRACE(dsc) RTE_LOG_TRACE("[GUI]",dsc)
#define LV_LOG_INFO(dsc) RTE_LOG_INFO("[GUI]",dsc)
#define LV_LOG_WARN(dsc) RTE_LOG_WARN("[GUI]",dsc)
#define LV_LOG_ERROR(dsc) RTE_LOG_ERROR("[GUI]",dsc)
#else
#define lv_log_add(level, file, line, dsc) {};
#define LV_LOG_TRACE(dsc) {};
#define LV_LOG_INFO(dsc) {};
#define LV_LOG_WARN(dsc) {};
#define LV_LOG_ERROR(dsc) {};
#endif
	/*********************
 *  时基部分重定向
 *********************/
#define MATH_TrigoSin_MAX    32767
#define LV_TRIGO_SHIFT      15      /* >> LV_TRIGO_SHIFT to normalize*/

#define LV_BEZIER_VAL_MAX       1024    /*Max time in Bezier functions (not [0..1] to use integers) */
#define LV_BEZIER_VAL_SHIFT     10      /*log2(LV_BEZIER_VAL_MAX): used to normalize up scale       */
#endif
