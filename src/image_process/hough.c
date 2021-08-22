/**
 * @file hough.c
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

rte_error_t hough_transform(image_t *input_image, image_t *output_image, uint32_t hough_space)
{
    image_init(output_image, input_image->w, input_image->h, IMAGE_BPP_BINARY);

    int32_t center_x = input_image->w >> 1;
    int32_t center_y = input_image->h >> 1;
    float hough_diff = M_PI / hough_space;

    uint16_t max = RTE_MAX(input_image->w, input_image->h);

}