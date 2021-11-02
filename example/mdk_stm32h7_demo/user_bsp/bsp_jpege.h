#ifndef __BSP_JPEGE_H
#define __BSP_JPEGE_H

#include "stm32h7xx.h"
#include "rte_include.h"
#include "hal_include.h"

extern void bsp_jpeg_init(void);
extern rte_error_t bsp_jpeg_compress(image_t *src, image_t *dst, int quality);

#endif
