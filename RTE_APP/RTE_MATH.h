#ifndef __RTE_MATH_H
#define __RTE_MATH_H
#ifdef __cplusplus  
extern "C" {  
#endif  
	#include "RTE_Include.h"
	#define RTE_MATH_MIN(a,b) (a<b?a:b)
	#define RTE_MATH_MAX(a,b) (a>b?a:b)
	#define RTE_MATH_ABS(x) ((x)>0?(x):(-(x)))
#ifdef __cplusplus  
}  
#endif  
#endif
