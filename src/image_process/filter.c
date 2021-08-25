/**
 * @file filter.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-25
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "../../inc/image_process/image_process.h"
#include "../../inc/middle_layer/rte_memory.h"
#include "../../inc/middle_layer/rte_log.h"

// http://www.fmwconcepts.com/imagemagick/digital_image_filtering.pdf

void filter_morph(image_t *img, int ksize,int *krn, float m, int b, bool threshold, int offset, bool invert, image_t *mask)
{
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.bpp = img->bpp;
    const int32_t m_int = (int32_t)(65536.0 * m); // m is 1/kernel_weight

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            buf.data = memory_alloc(BANK_MATH, (IMAGE_BINARY_LINE_LEN_BYTES(img) * brows));
            LOG_ASSERT("IMAGE", buf.data);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int acc = 0, ptr = 0;

                    if (x >= ksize && x < img->w-ksize && y >= ksize && y < img->h-ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,y+j);
                            for (int k = -ksize; k <= ksize; k++) {
                                acc += krn[ptr++] * IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,x+k);
                            }
                        }
                    } else {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                                RTE_MIN(RTE_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                acc += krn[ptr++] * IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
                                    RTE_MIN(RTE_MAX(x + k, 0), (img->w - 1)));
                            }
                        }
                    }
                    int32_t tmp = (acc * m_int) >> 16;
                    int pixel = tmp + b;
                    if (pixel < 0) pixel = 0;
                    else if (pixel > 1) pixel = 1;

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_BINARY_MAX;
                        } else {
                            pixel = COLOR_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = RTE_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            memory_free(BANK_MATH, buf.data);
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            buf.data = memory_alloc(BANK_MATH, (IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows));
            LOG_ASSERT("IMAGE", buf.data);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int32_t acc = 0, ptr = 0;

                    if (x >= ksize && x < img->w-ksize && y >= ksize && y < img->h-ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,y+j);
                            for (int k = -ksize; k <= ksize; k++) {
                                acc += krn[ptr++] * IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,x+k);
                            }
                        }
                    } else {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                                RTE_MIN(RTE_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                acc += krn[ptr++] * IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                    RTE_MIN(RTE_MAX(x + k, 0), (img->w - 1)));
                            }
                        }
                    }
                    int32_t tmp = (acc * m_int)>>16;
                    int pixel = tmp + b;
                    if (pixel > COLOR_GRAYSCALE_MAX) pixel = COLOR_GRAYSCALE_MAX;
                    else if (pixel < 0) pixel = 0;

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_GRAYSCALE_BINARY_MAX;
                        } else {
                            pixel = COLOR_GRAYSCALE_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = RTE_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
            }

            memory_free(BANK_MATH, buf.data);
            break;
        }
        case IMAGE_BPP_RGB565: {
            buf.data = memory_alloc(BANK_MATH, (IMAGE_RGB565_LINE_LEN_BYTES(img) * brows));
            LOG_ASSERT("IMAGE", buf.data);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int32_t tmp, r_acc = 0, g_acc = 0, b_acc = 0, ptr = 0;

                    if (x >= ksize && x < img->w-ksize && y >= ksize && y < img->h-ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,y+j);
                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,x+k);
                                r_acc += krn[ptr] * COLOR_RGB565_TO_R5(pixel);
                                g_acc += krn[ptr] * COLOR_RGB565_TO_G6(pixel);
                                b_acc += krn[ptr++] * COLOR_RGB565_TO_B5(pixel);
                            }
                        }
                    } else {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                                RTE_MIN(RTE_MAX(y + j, 0), (img->h - 1)));
                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                    RTE_MIN(RTE_MAX(x + k, 0), (img->w - 1)));
                                r_acc += krn[ptr] * COLOR_RGB565_TO_R5(pixel);
                                g_acc += krn[ptr] * COLOR_RGB565_TO_G6(pixel);
                                b_acc += krn[ptr++] * COLOR_RGB565_TO_B5(pixel);
                            }
                        }
                    }
                    tmp = (r_acc * m_int) >> 16;
                    r_acc = tmp + b;
                    if (r_acc > COLOR_R5_MAX) r_acc = COLOR_R5_MAX;
                    else if (r_acc < 0) r_acc = 0;
                    tmp = (g_acc * m_int) >> 16;
                    g_acc = tmp + b;
                    if (g_acc > COLOR_G6_MAX) g_acc = COLOR_G6_MAX;
                    else if (g_acc < 0) g_acc = 0;
                    tmp = (b_acc * m_int) >> 16;
                    b_acc = tmp + b;
                    if (b_acc > COLOR_B5_MAX) b_acc = COLOR_B5_MAX;
                    else if (b_acc < 0) b_acc = 0;

                    int pixel = COLOR_R5_G6_B5_TO_RGB565(r_acc, g_acc, b_acc);

                    if (threshold) {
                        if (((COLOR_RGB565_TO_Y(pixel) - offset) < COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x))) ^ invert) {
                            pixel = COLOR_RGB565_BINARY_MAX;
                        } else {
                            pixel = COLOR_RGB565_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB565_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = RTE_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB565_LINE_LEN_BYTES(img));
            }

            memory_free(BANK_MATH, buf.data);
            break;
        }
        default: {
            break;
        }
    }
}
