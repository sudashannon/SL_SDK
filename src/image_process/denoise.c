/**
 * @file denoise.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-22
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../inc/image_process/image_process.h"
#include "../../inc/middle_layer/rte_memory.h"
#include "../../inc/middle_layer/rte_log.h"

const int8_t kernel_gauss_3[3*3] = {
    1, 2, 1,
    2, 4, 2,
    1, 2, 1,
};

const int8_t kernel_gauss_5[5*5] = {
    1,  4,  6,  4, 1,
    4, 16, 24, 16, 4,
    6, 24, 36, 24, 6,
    4, 16, 24, 16, 4,
    1,  4,  6,  4, 1
};

const int kernel_laplacian_3[3*3] = {
    -1, -1, -1,
    -1,  8, -1,
    -1, -1, -1
};

const int kernel_high_pass_3[3*3] = {
    -1, -1, -1,
    -1, +8, -1,
    -1, -1, -1
};

static int8_t *filter_kernel[] = {
    kernel_gauss_3,
    kernel_laplacian_3,
    kernel_high_pass_3,
};

void denoise_sepconv3(image_t *img, denoise_type_t filter_type, float m, int32_t b)
{
    if (img->bpp != IMAGE_BPP_GRAYSCALE)
        return;
    int8_t *conv_kernel = filter_kernel[filter_type];
    uint8_t k_size = 3;
    int32_t *line_buffer = data_malloc_calculate(img->w * sizeof(int32_t) * 2);

    for (int32_t h = 0; h < img->h - k_size; h++) {
        // NOTE: This doesn't deal with borders right now. Adding if
        // statements in the inner loop will slow it down significantly.
        for (int32_t w = 0; w < img->w; w++) {
            int32_t acc = 0;
            acc = __SMLAD(conv_kernel[0], IMAGE_GET_GRAYSCALE_PIXEL(img, w, h + 0), acc);
            acc = __SMLAD(conv_kernel[1], IMAGE_GET_GRAYSCALE_PIXEL(img, w, h + 1), acc);
            acc = __SMLAD(conv_kernel[2], IMAGE_GET_GRAYSCALE_PIXEL(img, w, h + 2), acc);
            line_buffer[((h % 2) * img->w) + w] = acc;
        }
        if (h > 0) {
            // flush buffer
            for (int32_t w = 0; w < img->w - k_size; w++) {
                int32_t acc = 0;
                acc = __SMLAD(conv_kernel[0], line_buffer[((h - 1) % 2) * img->w + w + 0], acc);
                acc = __SMLAD(conv_kernel[1], line_buffer[((h - 1) % 2) * img->w + w + 1], acc);
                acc = __SMLAD(conv_kernel[2], line_buffer[((h - 1) % 2) * img->w + w + 2], acc);
                acc = (acc * m) + b; // scale, offset, and clamp
                acc = RTE_MAX(RTE_MIN(acc, COLOR_GRAYSCALE_MAX), 0);
                IMAGE_PUT_GRAYSCALE_PIXEL(img, (w + 1), (h), acc);
            }
        }
    }
    data_free(line_buffer);
}
