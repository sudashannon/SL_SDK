/**
 * @file hal_spi.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-07
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __HAL_SPI_H
#define __HAL_SPI_H
#include "RTE_Components.h"
#include CMSIS_device_header
#include "hal.h"
#include "cmsis_os2.h"

typedef enum
{
	spi_lcd = 0,
	spi_N,
} spi_name_t;

#endif
