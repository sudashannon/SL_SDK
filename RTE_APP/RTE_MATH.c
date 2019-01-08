/**
  ******************************************************************************
  * @file    RTE_MATH.h
  * @author  Shan Lei ->>lurenjia.tech ->>https://github.com/sudashannon
  * @brief   RTE自带的一些数学运算。
  * @version V1.0 2018/11/02 第一版
	* @History V1.0 创建
  ******************************************************************************
  */
#include "RTE_MATH.h"
static void InvertUint8(unsigned char *dBuf,unsigned char *srcBuf)  
{  
    int i;  
    unsigned char tmp[4];  
    tmp[0] = 0;  
    for(i=0;i< 8;i++)  
    {  
      if(srcBuf[0]& (1 << i))  
        tmp[0]|=1<<(7-i);  
    }  
    dBuf[0] = tmp[0];  
      
}  
static void InvertUint16(unsigned short *dBuf,unsigned short *srcBuf)  
{  
    int i;  
    unsigned short tmp[4];  
    tmp[0] = 0;  
    for(i=0;i< 16;i++)  
    {  
      if(srcBuf[0]& (1 << i))  
        tmp[0]|=1<<(15 - i);  
    }  
    dBuf[0] = tmp[0];  
}  
unsigned short MATH_CRC16_CCITT(unsigned char *puchMsg, unsigned int usDataLen)  
{  
  unsigned short wCRCin = 0x0000;  
  unsigned short wCPoly = 0x1021;  
  unsigned char wChar = 0;  
    
  while (usDataLen--)     
  {  
        wChar = *(puchMsg++);  
        InvertUint8(&wChar,&wChar);  
        wCRCin ^= (wChar << 8);  
        for(int i = 0;i < 8;i++)  
        {  
          if(wCRCin & 0x8000)  
            wCRCin = (wCRCin << 1) ^ wCPoly;  
          else  
            wCRCin = wCRCin << 1;  
        }  
  }  
  InvertUint16(&wCRCin,&wCRCin);  
  return (wCRCin) ;  
}  
unsigned short MATH_CRC16_CCITT_FALSE(unsigned char *puchMsg, unsigned int usDataLen)  
{  
  unsigned short wCRCin = 0xFFFF;  
  unsigned short wCPoly = 0x1021;  
  unsigned char wChar = 0;  
    
  while (usDataLen--)     
  {  
        wChar = *(puchMsg++);  
        wCRCin ^= (wChar << 8);  
        for(int i = 0;i < 8;i++)  
        {  
          if(wCRCin & 0x8000)  
            wCRCin = (wCRCin << 1) ^ wCPoly;  
          else  
            wCRCin = wCRCin << 1;  
        }  
  }  
  return (wCRCin) ;  
}  
unsigned short MATH_CRC16_XMODEM(unsigned char *puchMsg, unsigned int usDataLen)  
{  
  unsigned short wCRCin = 0x0000;  
  unsigned short wCPoly = 0x1021;  
  unsigned char wChar = 0;  
    
  while (usDataLen--)     
  {  
        wChar = *(puchMsg++);  
        wCRCin ^= (wChar << 8);  
        for(int i = 0;i < 8;i++)  
        {  
          if(wCRCin & 0x8000)  
            wCRCin = (wCRCin << 1) ^ wCPoly;  
          else  
            wCRCin = wCRCin << 1;  
        }  
  }  
  return (wCRCin) ;  
}  
  
unsigned short MATH_CRC16_X25(unsigned char *puchMsg, unsigned int usDataLen)  
{  
  unsigned short wCRCin = 0xFFFF;  
  unsigned short wCPoly = 0x1021;  
  unsigned char wChar = 0;  
    
  while (usDataLen--)     
  {  
        wChar = *(puchMsg++);  
        InvertUint8(&wChar,&wChar);  
        wCRCin ^= (wChar << 8);  
        for(int i = 0;i < 8;i++)  
        {  
          if(wCRCin & 0x8000)  
            wCRCin = (wCRCin << 1) ^ wCPoly;  
          else  
            wCRCin = wCRCin << 1;  
        }  
  }  
  InvertUint16(&wCRCin,&wCRCin);  
  return (wCRCin^0xFFFF) ;  
}  
  
unsigned short MATH_CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen)  
{  
  unsigned short wCRCin = 0xFFFF;  
  unsigned short wCPoly = 0x8005;  
  unsigned char wChar = 0;  
    
  while (usDataLen--)     
  {  
        wChar = *(puchMsg++);  
        InvertUint8(&wChar,&wChar);  
        wCRCin ^= (wChar << 8);  
        for(int i = 0;i < 8;i++)  
        {  
          if(wCRCin & 0x8000)  
            wCRCin = (wCRCin << 1) ^ wCPoly;  
          else  
            wCRCin = wCRCin << 1;  
        }  
  }  
  InvertUint16(&wCRCin,&wCRCin);  
  return (wCRCin) ;  
}  
unsigned short MATH_CRC16_IBM(unsigned char *puchMsg, unsigned int usDataLen)  
{  
  unsigned short wCRCin = 0x0000;  
  unsigned short wCPoly = 0x8005;  
  unsigned char wChar = 0;  
    
  while (usDataLen--)     
  {  
        wChar = *(puchMsg++);  
        InvertUint8(&wChar,&wChar);  
        wCRCin ^= (wChar << 8);  
        for(int i = 0;i < 8;i++)  
        {  
          if(wCRCin & 0x8000)  
            wCRCin = (wCRCin << 1) ^ wCPoly;  
          else  
            wCRCin = wCRCin << 1;  
        }  
  }  
  InvertUint16(&wCRCin,&wCRCin);  
  return (wCRCin) ;  
}  
unsigned short MATH_CRC16_MAXIM(unsigned char *puchMsg, unsigned int usDataLen)  
{  
  unsigned short wCRCin = 0x0000;  
  unsigned short wCPoly = 0x8005;  
  unsigned char wChar = 0;  
    
  while (usDataLen--)     
  {  
        wChar = *(puchMsg++);  
        InvertUint8(&wChar,&wChar);  
        wCRCin ^= (wChar << 8);  
        for(int i = 0;i < 8;i++)  
        {  
          if(wCRCin & 0x8000)  
            wCRCin = (wCRCin << 1) ^ wCPoly;  
          else  
            wCRCin = wCRCin << 1;  
        }  
  }  
  InvertUint16(&wCRCin,&wCRCin);  
  return (wCRCin^0xFFFF) ;  
}  
unsigned short MATH_CRC16_USB(unsigned char *puchMsg, unsigned int usDataLen)  
{  
  unsigned short wCRCin = 0xFFFF;  
  unsigned short wCPoly = 0x8005;  
  unsigned char wChar = 0;  
    
  while (usDataLen--)     
  {  
        wChar = *(puchMsg++);  
        InvertUint8(&wChar,&wChar);  
        wCRCin ^= (wChar << 8);  
        for(int i = 0;i < 8;i++)  
        {  
          if(wCRCin & 0x8000)  
            wCRCin = (wCRCin << 1) ^ wCPoly;  
          else  
            wCRCin = wCRCin << 1;  
        }  
  }  
  InvertUint16(&wCRCin,&wCRCin);  
  return (wCRCin^0xFFFF) ;  
}  
/**********************
 *  STATIC VARIABLES
 **********************/
static int16_t sin0_90_table[] = {
    0,     572,    1144,   1715,   2286,   2856,   3425,   3993,   4560,   5126,
    5690,  6252,   6813,   7371,   7927,   8481,   9032,   9580,   10126,  10668,
    11207, 11743,  12275,  12803,  13328,  13848,  14364,  14876,  15383,  15886,
    16383, 16876,  17364,  17846,  18323,  18794,  19260,  19720,  20173,  20621,
    21062, 21497,  21925,  22347,  22762,  23170,  23571,  23964,  24351,  24730,
    25101, 25465,  25821,  26169,  26509,  26841,  27165,  27481,  27788,  28087,
    28377, 28659,  28932,  29196,  29451,  29697,  29934,  30162,  30381,  30591,
    30791, 30982,  31163,  31335,  31498,  31650,  31794,  31927,  32051,  32165,
    32269, 32364,  32448,  32523,  32587,  32642,  32687,  32722,  32747,  32762
};
/**
 * Convert a number to string
 * @param num a number
 * @param buf pointer to a `char` buffer. The result will be stored here (max 10 elements)
 * @return same as `buf` (just for convenience)
 */
char * MATH_Num2Str(int32_t num, char * buf)
{
    char * buf_ori = buf;
    if(num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    } else if(num < 0) {
        (*buf) = '-';
        buf++;
        num = MATH_ABS(num);
    }
    uint32_t output = 0;
    int8_t i;

    for(i = 31; i >= 0; i--) {
        if((output & 0xF) >= 5)
            output += 3;
        if(((output & 0xF0) >> 4) >= 5)
            output += (3 << 4);
        if(((output & 0xF00) >> 8) >= 5)
            output += (3 << 8);
        if(((output & 0xF000) >> 12) >= 5)
            output += (3 << 12);
        if(((output & 0xF0000) >> 16) >= 5)
            output += (3 << 16);
        if(((output & 0xF00000) >> 20) >= 5)
            output += (3 << 20);
        if(((output & 0xF000000) >> 24) >= 5)
            output += (3 << 24);
        if(((output & 0xF0000000) >> 28) >= 5)
            output += (3 << 28);
        output = (output << 1) | ((num >> i) & 1);
    }

    uint8_t digit;
    bool leading_zero_ready = false;
    for(i = 28; i >= 0; i -= 4) {
        digit = ((output >> i) & 0xF) + '0';
        if(digit == '0' && leading_zero_ready == false) continue;

        leading_zero_ready = true;
        (*buf) = digit;
        buf++;
    }

    (*buf) = '\0';

    return buf_ori;
}

/**
 * Return with sinus of an angle
 * @param angle
 * @return sinus of 'angle'. sin(-90) = -32767, sin(90) = 32767
 */
int16_t MATH_TrigoSin(int16_t angle)
{
    int16_t ret = 0;
    angle = angle % 360;

    if(angle < 0) angle = 360 + angle;

    if(angle < 90) {
        ret = sin0_90_table[angle];
    } else if(angle >= 90 && angle < 180) {
        angle = 179 - angle;
        ret = sin0_90_table[angle];
    } else if(angle >= 180 && angle < 270) {
        angle = angle - 180;
        ret = - sin0_90_table[angle];
    } else { /*angle >=270*/
        angle = 359 - angle;
        ret = - sin0_90_table[angle];
    }

    return ret;
}

/**
 * Calculate a value of a Cubic Bezier function.
 * @param t time in range of [0..LV_BEZIER_VAL_MAX]
 * @param u0 start values in range of [0..LV_BEZIER_VAL_MAX]
 * @param u1 control value 1 values in range of [0..LV_BEZIER_VAL_MAX]
 * @param u2 control value 2 in range of [0..LV_BEZIER_VAL_MAX]
 * @param u3 end values in range of [0..LV_BEZIER_VAL_MAX]
 * @return the value calculated from the given parameters in range of [0..LV_BEZIER_VAL_MAX]
 */
int32_t MATH_Bezier3(uint32_t t, int32_t u0, int32_t u1, int32_t u2, int32_t u3)
{
    uint32_t t_rem = 1024 - t;
    uint32_t t_rem2 = (t_rem * t_rem) >> 10;
    uint32_t t_rem3 = (t_rem2 * t_rem) >> 10;
    uint32_t t2 = (t * t) >> 10;
    uint32_t t3 = (t2 * t) >> 10;


    uint32_t v1 = ((uint32_t)t_rem3 * u0) >> 10;
    uint32_t v2 = ((uint32_t)3 * t_rem2 * t * u1) >> 20;
    uint32_t v3 = ((uint32_t)3 * t_rem * t2 * u2) >> 20;
    uint32_t v4 = ((uint32_t)t3 * u3) >> 10;

    return v1 + v2 + v3 + v4;

}

