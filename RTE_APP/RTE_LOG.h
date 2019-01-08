#ifndef __RTE_LOG_H
#define __RTE_LOG_H
#ifdef __cplusplus  
extern "C" {  
#endif  
	#include "RTE_Config.h"
	#if RTE_USE_LOG == 1
	#include <stdint.h>
	#include <stdarg.h>
	#include "RTE_UStdout.h"
	#define RTE_LOG_LEVEL_TRACE 0     /*A lot of logs to give detailed information*/
	#define RTE_LOG_LEVEL_INFO  1     /*Log important events*/
	#define RTE_LOG_LEVEL_WARN  2     /*Log if something unwanted happened but didn't caused problem*/
	#define RTE_LOG_LEVEL_ERROR 3     /*Only critical issue, when the system may fail*/
	#define _RTE_LOG_LEVEL_NUM  4
	typedef uint8_t RTE_LOG_Level_t;
	/**
	 * Add a log
	 * @param level the level of log. (From `lv_log_level_t` enum)
	 * @param file name of the file when the log added
	 * @param line line number in the source code where the log added
	 * @param dsc description of the log
	 */
	void RTE_LOG_ADD(RTE_LOG_Level_t level, const char * module ,const char * file, uint32_t line, const char * dsc, ...);
	#define RTE_LOG_TRACE(dsc,args...)   RTE_LOG_ADD(RTE_LOG_LEVEL_TRACE, mod,__FILE__, __LINE__, dsc, ##args);
	#define RTE_LOG_INFO(mod,dsc,args...)    RTE_LOG_ADD(RTE_LOG_LEVEL_INFO,mod, __FILE__, __LINE__, dsc, ##args);
	#define RTE_LOG_WARN(mod,dsc,args...)    RTE_LOG_ADD(RTE_LOG_LEVEL_WARN,mod, __FILE__, __LINE__, dsc, ##args);
	#define RTE_LOG_ERROR(mod,dsc,args...)   RTE_LOG_ADD(RTE_LOG_LEVEL_ERROR,mod, __FILE__, __LINE__, dsc, ##args);
	#else
	#define RTE_LOG_TRACE(dsc) {;}
	#define RTE_LOG_INFO(dsc) {;}
	#define RTE_LOG_WARN(dsc) {;}
	#define RTE_LOG_ERROR(dsc) {;}
	#endif
#ifdef __cplusplus  
}  
#endif  
#endif
		
