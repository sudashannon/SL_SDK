/**
 * @file edge.c
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

typedef struct gvec {
    uint16_t t;
    uint16_t g;
} gvec_t;

void edge_canny(image_t *src, rectangle_t *roi, int32_t low_thresh, int32_t high_thresh)
{
    int32_t w = src->w;

    gvec_t *gm = memory_calloc(BANK_MATH, roi->w * roi->h * sizeof(gvec_t));

    //1. Noise Reduction with a Gaussian filter
    denoise_sepconv3(src, GAUSSIAN, 1.0f/16.0f, 0);

    //2. Finding Image Gradients
    for (int32_t gy = 1, y = roi->y + 1; y < roi->y + roi->h - 1; y++, gy++) {
        for (int32_t gx = 1, x = roi->x + 1; x < roi->x +roi->w - 1; x++, gx++) {
            int32_t vx = 0, vy = 0;
            // sobel kernel in the horizontal direction
            vx  = src->data [(y-1)*w+x-1]
                - src->data [(y-1)*w+x+1]
                + (src->data[(y+0)*w+x-1]<<1)
                - (src->data[(y+0)*w+x+1]<<1)
                + src->data [(y+1)*w+x-1]
                - src->data [(y+1)*w+x+1];

            // sobel kernel in the vertical direction
            vy  = src->data [(y-1)*w+x-1]
                + (src->data[(y-1)*w+x+0]<<1)
                + src->data [(y-1)*w+x+1]
                - src->data [(y+1)*w+x-1]
                - (src->data[(y+1)*w+x+0]<<1)
                - src->data [(y+1)*w+x+1];

            // Find magnitude
            int32_t g = (int32_t) sqrtf(vx*vx + vy*vy);
            // Find the direction and round angle to 0, 45, 90 or 135
            int32_t t = (int32_t) fabsf((atan2f(vy, vx)*180.0f/M_PI));
            if (t < 22) {
                t = 0;
            } else if (t < 67) {
                t = 45;
            } else if (t < 112) {
                t = 90;
            } else if (t < 160) {
                t = 135;
            } else if (t <= 180) {
                t = 0;
            }

            gm[gy*roi->w+gx].t = t;
            gm[gy*roi->w+gx].g = g;
        }
    }

    // 3. Hysteresis Thresholding
    // 4. Non-maximum Suppression and output
    for (int32_t gy=0, y=roi->y; y<roi->y+roi->h; y++, gy++) {
        for (int32_t gx=0, x=roi->x; x<roi->x+roi->w; x++, gx++) {
            int32_t i = y*w+x;
            gvec_t *va=NULL, *vb=NULL, *vc = &gm[gy*roi->w+gx];

            // Clear the borders
            if (y == (roi->y) || y == (roi->y+roi->h-1) ||
                x == (roi->x) || x == (roi->x+roi->w-1)) {
                src->data[i] = 0;
                continue;
            }

            if (vc->g < low_thresh) {
                // Not an edge
                src->data[i] = 0;
                continue;
            // Check if strong or weak edge
            } else if (vc->g >= high_thresh ||
                       gm[(gy-1)*roi->w+(gx-1)].g >= high_thresh ||
                       gm[(gy-1)*roi->w+(gx+0)].g >= high_thresh ||
                       gm[(gy-1)*roi->w+(gx+1)].g >= high_thresh ||
                       gm[(gy+0)*roi->w+(gx-1)].g >= high_thresh ||
                       gm[(gy+0)*roi->w+(gx+1)].g >= high_thresh ||
                       gm[(gy+1)*roi->w+(gx-1)].g >= high_thresh ||
                       gm[(gy+1)*roi->w+(gx+0)].g >= high_thresh ||
                       gm[(gy+1)*roi->w+(gx+1)].g >= high_thresh) {
                vc->g = vc->g;
            } else { // Not an edge
                src->data[i] = 0;
                continue;
            }

            switch (vc->t) {
                case 0: {
                    va = &gm[(gy+0)*roi->w+(gx-1)];
                    vb = &gm[(gy+0)*roi->w+(gx+1)];
                    break;
                }

                case 45: {
                    va = &gm[(gy+1)*roi->w+(gx-1)];
                    vb = &gm[(gy-1)*roi->w+(gx+1)];
                    break;
                }

                case 90: {
                    va = &gm[(gy+1)*roi->w+(gx+0)];
                    vb = &gm[(gy-1)*roi->w+(gx+0)];
                    break;
                }

                case 135: {
                    va = &gm[(gy+1)*roi->w+(gx+1)];
                    vb = &gm[(gy-1)*roi->w+(gx-1)];
                    break;
                }
            }

            if (!(vc->g > va->g && vc->g > vb->g)) {
                src->data[i] = 0;
            } else {
                src->data[i] = 255;
            }
        }
    }

    memory_free(BANK_MATH, gm);
}