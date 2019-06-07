#ifndef __RTE_PRINTF_H_
#define __RTE_PRINTF_H_
#ifdef __cplusplus
extern "C" {
#endif
	#include "RTE_Config.h"
	#if RTE_USE_PRINTF == 1
	#include <stdarg.h>
	#include <stddef.h>
	#if RTE_USE_OS == 1
	#include "cmsis_os2.h"
	#endif
	typedef struct
	{
		void (*putchar)(char character);
	#if RTE_USE_OS == 1
		osMutexId_t mutexid;
	#endif
	}printf_t;
	/**
	 * Tiny printf implementation
	 * You have to implement _putchar if you use printf()
	 * To avoid conflicts with the regular printf() API it is overridden by macro defines
	 * and internal underscore-appended functions like printf_() are used
	 * \param format A string that specifies the format of the output
	 * \return The number of characters that are written into the array, not counting the terminating null character
	 */
	int uprintf(const char* format, ...);
	/**
	 * Tiny sprintf implementation
	 * Due to security reasons (buffer overflow) YOU SHOULD CONSIDER USING (V)SNPRINTF INSTEAD!
	 * \param buffer A pointer to the buffer where to store the formatted string. MUST be big enough to store the output!
	 * \param format A string that specifies the format of the output
	 * \return The number of characters that are WRITTEN into the buffer, not counting the terminating null character
	 */
	int usprintf(char* buffer, const char* format, ...);
	/**
	 * Tiny snprintf/vsnprintf implementation
	 * \param buffer A pointer to the buffer where to store the formatted string
	 * \param count The maximum number of characters to store in the buffer, including a terminating null character
	 * \param format A string that specifies the format of the output
	 * \param va A value identifying a variable arguments list
	 * \return The number of characters that COULD have been written into the buffer, not counting the terminating
	 *         null character. A value equal or larger than count indicates truncation. Only when the returned value
	 *         is non-negative and less than count, the string has been completely written.
	 */
	int  usnprintf_(char* buffer, size_t count, const char* format, ...);
	int uvsnprintf_(char* buffer, size_t count, const char* format, va_list va);
	/**
	 * Tiny vprintf implementation
	 * \param format A string that specifies the format of the output
	 * \param va A value identifying a variable arguments list
	 * \return The number of characters that are WRITTEN into the buffer, not counting the terminating null character
	 */
	int uvprintf_(const char* format, va_list va);
	/**
	 * printf with output function
	 * You may use this as dynamic alternative to printf() with its fixed _putchar() output
	 * \param out An output function which takes one character and an argument pointer
	 * \param arg An argument pointer for user data passed to output function
	 * \param format A string that specifies the format of the output
	 * \return The number of characters that are sent to the output function, not counting the terminating null character
	 */
	int ufctprintf(void (*out)(char character, void* arg), void* arg, const char* format, ...);
	void Printf_RegPutC(void (*PutcFunc)(char character));
	void Printf_Init(void);
#ifdef __cplusplus
}
#endif

#endif
#endif  // _PRINTF_H_
