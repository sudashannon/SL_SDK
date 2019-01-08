#ifndef __RTE_MATH_H
#define __RTE_MATH_H
#ifdef __cplusplus  
extern "C" {  
#endif  
	#include <stdint.h>
	#include <stdbool.h>
	#define M_PI    3.14159265f
	#define M_PI_2  1.57079632f
	#define M_PI_4  0.78539816f
	
	#define MATH_LOG2_2(x)    (((x) &                0x2ULL) ? ( 2                        ) :             1) // NO ({ ... }) !
	#define MATH_LOG2_4(x)    (((x) &                0xCULL) ? ( 2 +  MATH_LOG2_2((x) >>  2)) :  MATH_LOG2_2(x)) // NO ({ ... }) !
	#define MATH_LOG2_8(x)    (((x) &               0xF0ULL) ? ( 4 +  MATH_LOG2_4((x) >>  4)) :  MATH_LOG2_4(x)) // NO ({ ... }) !
	#define MATH_LOG2_16(x)   (((x) &             0xFF00ULL) ? ( 8 +  MATH_LOG2_8((x) >>  8)) :  MATH_LOG2_8(x)) // NO ({ ... }) !
	#define MATH_LOG2_32(x)   (((x) &         0xFFFF0000ULL) ? (16 + MATH_LOG2_16((x) >> 16)) : MATH_LOG2_16(x)) // NO ({ ... }) !
	#define MATH_LOG2(x)      (((x) & 0xFFFFFFFF00000000ULL) ? (32 + MATH_LOG2_32((x) >> 32)) : MATH_LOG2_32(x)) // NO ({ ... }) !

	#define MATH_MAX(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
	#define MATH_MIN(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
	#define MATH_DIV(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a / _b) : 0; })
	#define MATH_MOD(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a % _b) : 0; })
	#define MATH_ABS(x)       ({ __typeof__ (x) _x = (x); (_x)>0?(_x):(-(_x)) ;})
		
	#define INT8_T_BITS     (sizeof(int8_t) * 8)
	#define INT8_T_MASK     (INT8_T_BITS - 1)
	#define INT8_T_SHIFT    MATH_LOG2(INT8_T_MASK)

	#define INT16_T_BITS    (sizeof(int16_t) * 8)
	#define INT16_T_MASK    (INT16_T_BITS - 1)
	#define INT16_T_SHIFT   MATH_LOG2(INT16_T_MASK)

	#define INT32_T_BITS    (sizeof(int32_t) * 8)
	#define INT32_T_MASK    (INT32_T_BITS - 1)
	#define INT32_T_SHIFT   MATH_LOG2(INT32_T_MASK)

	#define INT64_T_BITS    (sizeof(int64_t) * 8)
	#define INT64_T_MASK    (INT64_T_BITS - 1)
	#define INT64_T_SHIFT   MATH_LOG2(INT64_T_MASK)

	#define UINT8_T_BITS    (sizeof(uint8_t) * 8)
	#define UINT8_T_MASK    (UINT8_T_BITS - 1)
	#define UINT8_T_SHIFT   MATH_LOG2(UINT8_T_MASK)

	#define UINT16_T_BITS   (sizeof(uint16_t) * 8)
	#define UINT16_T_MASK   (UINT16_T_BITS - 1)
	#define UINT16_T_SHIFT  MATH_LOG2(UINT16_T_MASK)

	#define UINT32_T_BITS   (sizeof(uint32_t) * 8)
	#define UINT32_T_MASK   (UINT32_T_BITS - 1)
	#define UINT32_T_SHIFT  MATH_LOG2(UINT32_T_MASK)

	#define UINT64_T_BITS   (sizeof(uint64_t) * 8)
	#define UINT64_T_MASK   (UINT64_T_BITS - 1)
	#define UINT64_T_SHIFT  MATH_LOG2(UINT64_T_MASK)

	#define MATH_DEG2RAD(x)   (((x)*M_PI)/180)
	#define MATH_RAD2DEG(x)   (((x)*180)/M_PI)
	
	
	
	unsigned short MATH_CRC16_CCITT(unsigned char *puchMsg, unsigned int usDataLen);
	unsigned short MATH_CRC16_CCITT_FALSE(unsigned char *puchMsg, unsigned int usDataLen)  ;
	unsigned short MATH_CRC16_XMODEM(unsigned char *puchMsg, unsigned int usDataLen)  ;
	unsigned short MATH_CRC16_X25(unsigned char *puchMsg, unsigned int usDataLen)  ;
	unsigned short MATH_CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen)  ;
	unsigned short MATH_CRC16_IBM(unsigned char *puchMsg, unsigned int usDataLen)  ;
	unsigned short MATH_CRC16_MAXIM(unsigned char *puchMsg, unsigned int usDataLen)  ;
	unsigned short MATH_CRC16_USB(unsigned char *puchMsg, unsigned int usDataLen)  ;
	/**
	 * Convert a number to string
	 * @param num a number
	 * @param buf pointer to a `char` buffer. The result will be stored here (max 10 elements)
	 * @return same as `buf` (just for convenience)
	 */
	char * MATH_Num2Str(int32_t num, char * buf);

	/**
	 * Return with sinus of an angle
	 * @param angle
	 * @return sinus of 'angle'. sin(-90) = -32767, sin(90) = 32767
	 */
	int16_t MATH_TrigoSin(int16_t angle);

	/**
	 * Calculate a value of a Cubic Bezier function.
	 * @param t time in range of [0..LV_BEZIER_VAL_MAX]
	 * @param u0 start values in range of [0..LV_BEZIER_VAL_MAX]
	 * @param u1 control value 1 values in range of [0..LV_BEZIER_VAL_MAX]
	 * @param u2 control value 2 in range of [0..LV_BEZIER_VAL_MAX]
	 * @param u3 end values in range of [0..LV_BEZIER_VAL_MAX]
	 * @return the value calculated from the given parameters in range of [0..LV_BEZIER_VAL_MAX]
	 */
	int32_t MATH_Bezier3(uint32_t t, int32_t u0, int32_t u1, int32_t u2, int32_t u3);

#ifdef __cplusplus  
}  
#endif  
#endif
