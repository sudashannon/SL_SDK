#ifndef __UARTSTDIO_H__
#define __UARTSTDIO_H__
#include "RTE_Include.h"
#include <stdarg.h>
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif
extern int RTE_Putchar (int ch);
extern int RTE_Getchar (void);
extern void RTE_Puts (const char *pcString,uint16_t length);
extern void RTE_Printf(const char *pcString, ...);
extern void RTE_Vprintf(const char *pcString, va_list vaArgP);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __UARTSTDIO_H__
