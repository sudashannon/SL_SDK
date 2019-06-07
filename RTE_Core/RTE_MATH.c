/**
  ******************************************************************************
  * @file    RTE_MATH.h
  * @author  Shan Lei ->>lurenjia.tech ->>https://github.com/sudashannon
  * @brief   RTE自带的一些数学运算。
  * @version V1.0 2018/11/02 第一版
  * @History V1.0 创建
  ******************************************************************************
  */
#include "RTE_Math.h"
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
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
static const uint32_t crc32_table[] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/**
 * Calculate the CRC32 value of a memory buffer.
 *
 * @param crc accumulated CRC32 value, must be 0 on first call
 * @param buf buffer to calculate CRC32 value for
 * @param size bytes in buffer
 *
 * @return calculated CRC32 value
 */
uint32_t MATH_CRC32(uint32_t crc, const void *buf, size_t size)
{
    const uint8_t *p;
    p = (const uint8_t *)buf;
    crc = crc ^ ~0U;
    while (size--) {
        crc = crc32_table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ ~0U;
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
int MATH_StringToHex(const char *str, unsigned char *out)
{
    char *p = (char *)str;
    char high = 0, low = 0;
    int tmplen = strlen(p), cnt = 0;
    tmplen = strlen(p);
    while(cnt < (tmplen / 2))
    {
        high = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
		low = (*(++ p) > '9' && ((*p <= 'F') || (*p <= 'f'))) ? *(p) - 48 - 7 : *(p) - 48;
        out[cnt] = ((high & 0x0f) << 4 | (low & 0x0f));
        p ++;
        cnt ++;
    }
    if(tmplen % 2 != 0) out[cnt] = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
    return tmplen / 2 + tmplen % 2;
}

