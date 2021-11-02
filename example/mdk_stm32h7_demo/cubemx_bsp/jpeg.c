/**
  ******************************************************************************
  * @file    jpeg.c
  * @brief   This file provides code for the configuration
  *          of the JPEG instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "jpeg.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

JPEG_HandleTypeDef hjpeg;
MDMA_HandleTypeDef hmdma_jpeg_infifo_th;
MDMA_HandleTypeDef hmdma_jpeg_outfifo_th;

/* JPEG init function */
void MX_JPEG_Init(void)
{

  /* USER CODE BEGIN JPEG_Init 0 */

  /* USER CODE END JPEG_Init 0 */

  /* USER CODE BEGIN JPEG_Init 1 */

  /* USER CODE END JPEG_Init 1 */
  hjpeg.Instance = JPEG;
  if (HAL_JPEG_Init(&hjpeg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN JPEG_Init 2 */

  /* USER CODE END JPEG_Init 2 */

}

void HAL_JPEG_MspInit(JPEG_HandleTypeDef* jpegHandle)
{

  if(jpegHandle->Instance==JPEG)
  {
  /* USER CODE BEGIN JPEG_MspInit 0 */

  /* USER CODE END JPEG_MspInit 0 */
    /* JPEG clock enable */
    __HAL_RCC_JPEG_CLK_ENABLE();

    /* JPEG MDMA Init */
    /* JPEG_INFIFO_TH Init */
    hmdma_jpeg_infifo_th.Instance = MDMA_Channel0;
    hmdma_jpeg_infifo_th.Init.Request = MDMA_REQUEST_JPEG_INFIFO_TH;
    hmdma_jpeg_infifo_th.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
    hmdma_jpeg_infifo_th.Init.Priority = MDMA_PRIORITY_VERY_HIGH;
    hmdma_jpeg_infifo_th.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_jpeg_infifo_th.Init.SourceInc = MDMA_SRC_INC_BYTE;
    hmdma_jpeg_infifo_th.Init.DestinationInc = MDMA_DEST_INC_DISABLE;
    hmdma_jpeg_infifo_th.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    hmdma_jpeg_infifo_th.Init.DestDataSize = MDMA_DEST_DATASIZE_WORD;
    hmdma_jpeg_infifo_th.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_jpeg_infifo_th.Init.BufferTransferLength = 32;
    hmdma_jpeg_infifo_th.Init.SourceBurst = MDMA_SOURCE_BURST_32BEATS;
    hmdma_jpeg_infifo_th.Init.DestBurst = MDMA_DEST_BURST_16BEATS;
    hmdma_jpeg_infifo_th.Init.SourceBlockAddressOffset = 0;
    hmdma_jpeg_infifo_th.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_jpeg_infifo_th) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(jpegHandle,hdmain,hmdma_jpeg_infifo_th);

    /* JPEG_OUTFIFO_TH Init */
    hmdma_jpeg_outfifo_th.Instance = MDMA_Channel1;
    hmdma_jpeg_outfifo_th.Init.Request = MDMA_REQUEST_JPEG_OUTFIFO_TH;
    hmdma_jpeg_outfifo_th.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
    hmdma_jpeg_outfifo_th.Init.Priority = MDMA_PRIORITY_VERY_HIGH;
    hmdma_jpeg_outfifo_th.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_jpeg_outfifo_th.Init.SourceInc = MDMA_SRC_INC_DISABLE;
    hmdma_jpeg_outfifo_th.Init.DestinationInc = MDMA_DEST_INC_BYTE;
    hmdma_jpeg_outfifo_th.Init.SourceDataSize = MDMA_SRC_DATASIZE_WORD;
    hmdma_jpeg_outfifo_th.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
    hmdma_jpeg_outfifo_th.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_jpeg_outfifo_th.Init.BufferTransferLength = 32;
    hmdma_jpeg_outfifo_th.Init.SourceBurst = MDMA_SOURCE_BURST_32BEATS;
    hmdma_jpeg_outfifo_th.Init.DestBurst = MDMA_DEST_BURST_32BEATS;
    hmdma_jpeg_outfifo_th.Init.SourceBlockAddressOffset = 0;
    hmdma_jpeg_outfifo_th.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_jpeg_outfifo_th) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(jpegHandle,hdmaout,hmdma_jpeg_outfifo_th);

    /* JPEG interrupt Init */
    HAL_NVIC_SetPriority(JPEG_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(JPEG_IRQn);
  /* USER CODE BEGIN JPEG_MspInit 1 */

  /* USER CODE END JPEG_MspInit 1 */
  }
}

void HAL_JPEG_MspDeInit(JPEG_HandleTypeDef* jpegHandle)
{

  if(jpegHandle->Instance==JPEG)
  {
  /* USER CODE BEGIN JPEG_MspDeInit 0 */

  /* USER CODE END JPEG_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_JPEG_CLK_DISABLE();

    /* JPEG MDMA DeInit */
    HAL_MDMA_DeInit(jpegHandle->hdmain);
    HAL_MDMA_DeInit(jpegHandle->hdmaout);

    /* JPEG interrupt Deinit */
    HAL_NVIC_DisableIRQ(JPEG_IRQn);
  /* USER CODE BEGIN JPEG_MspDeInit 1 */

  /* USER CODE END JPEG_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
