/**
 * @file ds_image.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-08
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../inc/data_structure/ds_image.h"
#include "../../inc/middle_layer/rte_memory.h"

void image_init(image_t *ptr, uint8_t *buffer, int w, int h, image_bpp_t bpp)
{
    ptr->w = w;
    ptr->h = h;
    ptr->bpp = bpp;
    ptr->data = buffer;
}

void image_copy(image_t *dst, image_t *src)
{
    memcpy(dst, src, sizeof(image_t));
}

size_t image_size(image_t *ptr)
{
    if (ptr->bpp < 0) {
        return 0;
    }

    switch (ptr->bpp) {
        case IMAGE_BPP_BINARY: {
            return IMAGE_BINARY_LINE_LEN_BYTES(ptr) * ptr->h;
        }
        case IMAGE_BPP_GRAYSCALE: {
            return IMAGE_GRAYSCALE_LINE_LEN_BYTES(ptr) * ptr->h;
        }
        case IMAGE_BPP_RGB565: {
            return IMAGE_RGB565_LINE_LEN_BYTES(ptr) * ptr->h;
        }
        case IMAGE_BPP_BAYER: {
            return ptr->w * ptr->h;
        }
        default: { // JPEG
            return ptr->bpp;
        }
    }
}
