#ifndef __BOARD_JPEG_H
#define __BOARD_JPEG_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
#include "mv_core/imlib.h"
bool Board_JPEG_Encode(image_t *src, image_t *dst, int quality);
#endif
