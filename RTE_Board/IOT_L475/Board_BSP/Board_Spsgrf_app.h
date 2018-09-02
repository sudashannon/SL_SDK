/**
******************************************************************************
  * @file    spsgrf_app.h
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    21-April-2017
  * @brief   This file provides some abstraction (upper layer) to SPSGRF driver
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright © 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPSGRF_APP_H
#define __SPSGRF_APP_H


/* Includes ------------------------------------------------------------------*/
#include "Board_Spsgrf.h"
#include "stm32l4xx.h"
#include "Board_Spsgrf_io.h"

#ifdef __cplusplus
  "C" {
#endif


/**
 * @addtogroup SPSGRF_APP
 * @{
 */

/**
 * @defgroup SPSGRF_APP_Exported_Constants                SPSGR_GPIO Exported Constants
 * @{
 */
   
#define CUT_MAX_NO 3
#define CUT_2_1v3 0x0103
#define CUT_2_1v4 0x0104
#define CUT_3_0   0x0130

/**
* @}
*/

/**
 * @defgroup SPSGRF_APP_Exported_Types                SPSGR_GPIO Exported Types
 * @{
 */
    
typedef struct 
{
  uint16_t nSpiritVersion;
  SpiritVersion xSpiritVersion;
}SpiritVersionMap;


/**
 * @brief  Range extender type
 */
typedef enum
{
  RANGE_EXT_NONE = 0x00,
  RANGE_EXT_SKYWORKS_169,
  RANGE_EXT_SKYWORKS_868
} RangeExtType;

/**
* @}
*/

/**
 * @defgroup SPSGRF_APP_Exported_Functions                SPSGR_GPIO Exported Functions
 * @{
 */
void SpiritManagementIdentificationRFBoard(void);

RangeExtType SpiritManagementGetRangeExtender(void);
void SpiritManagementSetRangeExtender(RangeExtType xRangeType);
void SpiritManagementRangeExtInit(void);
void SpiritManagementSetBand(uint8_t value);
uint8_t SpiritManagementGetBand(void);

uint8_t SdkEvalGetHasEeprom(void);

void Spirit1InterfaceInit(void);
void Spirit1GpioIrqInit(SGpioInit *pGpioIRQ);
void Spirit1RadioInit(SRadioInit *pRadioInit);
void Spirit1SetPower(uint8_t cIndex, float fPowerdBm);
void Spirit1PacketConfig(void);
void Spirit1SetPayloadlength(uint8_t length);
void Spirit1SetDestinationAddress(uint8_t address);
void Spirit1EnableTxIrq(void);
void Spirit1EnableRxIrq(void);
void Spirit1DisableIrq(void);
void Spirit1SetRxTimeout(float cRxTimeOut);
void Spirit1EnableSQI(void);
void Spirit1SetRssiTH(int dbmValue);
float Spirit1GetRssiTH(void);
void Spirit1ClearIRQ(void);
void Spirit1StartRx(void);
void Spirit1GetRxPacket(uint8_t *buffer, uint8_t *size );
void Spirit1StartTx(uint8_t *buffer, uint8_t size);
int32_t SpiritManagementGetOffset(void);
void SpiritManagementSetOffset(int32_t value);
void SdkEvalSetHasEeprom(uint8_t eeprom);

/**
* @}
*/

/**
* @}
*/

#ifdef __cplusplus
}
#endif


#endif /* __SPSGRF_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

