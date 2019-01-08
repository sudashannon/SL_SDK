/**
  ******************************************************************************
  * @file    RTE_LOG.h
  * @author  Shan Lei ->>lurenjia.tech ->>https://github.com/sudashannon
  * @brief   RTE日志模块，以不同的方式输出程序运行情况。
  * @version V1.1 2019/01/08
	* @History V1.0 创建，移植自lvgl
	           V1.1 修改使其适配RTE环境
  ******************************************************************************
  */
#include "RTE_LOG.h"
#if RTE_USE_LOG == 1
#define LOG_STR "[LOG]"
/**
 * Add a log
 * @param level the level of log. (From `lv_log_level_t` enum)
 * @param file name of the file when the log added
 * @param line line number in the source code where the log added
 * @param dsc description of the log
 */
void RTE_LOG_ADD(RTE_LOG_Level_t level, const char * module ,const char * file, uint32_t line, const char * dsc, ...)
{
	if(level >= _RTE_LOG_LEVEL_NUM) return;      /*Invalid level*/
	static const char * lvl_prefix[] = {"Trace", "Info", "Warn", "Error"};
	RTE_Printf("%10s    <%s:%s>(%s #%d)\t",LOG_STR,module,lvl_prefix[level], file, line);
	va_list vaArgP;
	// Start the varargs processing.
	va_start(vaArgP, dsc);
	RTE_Vprintf(dsc, vaArgP);
	// We're finished with the varargs now.
	va_end(vaArgP);
	RTE_Printf("\r\n");
}
#endif
