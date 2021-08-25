/**
 * @file binary.c
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
#include "../../inc/data_structure/ds_linklist.h"

typedef struct {
    image_t *bmp;
    image_t *input;
    bool invert;
} binary_iterator_arg_t;

static int ll_iterator_handle(void *item, size_t idx, void *user)
{
    image_t *bmp = ((binary_iterator_arg_t *)user)->bmp;
    image_t *img = ((binary_iterator_arg_t *)user)->input;
    bool invert = ((binary_iterator_arg_t *)user)->invert;
    color_thresholds_list_lnk_data_t *lnk_data = item;
    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *old_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(bmp, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (COLOR_THRESHOLD_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(old_row_ptr, x), lnk_data, invert)) {
                        IMAGE_SET_BINARY_PIXEL_FAST(bmp_row_ptr, x);
                    }
                }
            }
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *old_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(bmp, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (COLOR_THRESHOLD_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(old_row_ptr, x), lnk_data, invert)) {
                        IMAGE_SET_BINARY_PIXEL_FAST(bmp_row_ptr, x);
                    }
                }
            }
            break;
        }
        case IMAGE_BPP_RGB565: {
            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *old_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(bmp, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (COLOR_THRESHOLD_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(old_row_ptr, x), lnk_data, invert)) {
                        IMAGE_SET_BINARY_PIXEL_FAST(bmp_row_ptr, x);
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }
    return 1;
}

void binary_image(image_t *out, image_t *img, linked_list_t *thresholds, bool invert, bool zero, image_t *mask)
{
    image_t bmp;
    bmp.w = img->w;
    bmp.h = img->h;
    bmp.bpp = IMAGE_BPP_BINARY;
    bmp.data = memory_calloc(BANK_MATH, image_size(&bmp));
    LOG_ASSERT("IMAGE", bmp.data);

    binary_iterator_arg_t user_arg = {
        .bmp = &bmp,
        .input = img,
        .invert = invert,
    };
    list_foreach_value(thresholds, ll_iterator_handle, &user_arg);

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            if (!zero) {
                for (int y = 0, yy = img->h; y < yy; y++) {
                    uint32_t *old_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                    uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&bmp, y);
                    uint32_t *out_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y);
                    for (int x = 0, xx = img->w; x < xx; x++) {
                        int pixel = ((!mask) || image_get_mask_pixel(mask, x, y))
                            ? IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr, x)
                            : IMAGE_GET_BINARY_PIXEL_FAST(old_row_ptr, x);
                        IMAGE_PUT_BINARY_PIXEL_FAST(out_row_ptr, x, pixel);
                    }
                }
            } else {
                for (int y = 0, yy = img->h; y < yy; y++) {
                    uint32_t *old_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                    uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&bmp, y);
                    uint32_t *out_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y);
                    for (int x = 0, xx = img->w; x < xx; x++) {
                        int pixel = IMAGE_GET_BINARY_PIXEL_FAST(old_row_ptr, x);
                        if (((!mask) || image_get_mask_pixel(mask, x, y))
                            && IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr, x)) pixel = 0;
                        IMAGE_PUT_BINARY_PIXEL_FAST(out_row_ptr, x, pixel);
                    }
                }
            }
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            if (out->bpp == IMAGE_BPP_BINARY) {
                if (!zero) {
                    for (int y = 0, yy = img->h; y < yy; y++) {
                        uint8_t *old_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                        uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&bmp, y);
                        uint32_t *out_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y);
                        for (int x = 0, xx = img->w; x < xx; x++) {
                            int pixel = ((!mask) || image_get_mask_pixel(mask, x, y))
                                ? IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr, x)
                                : COLOR_GRAYSCALE_TO_BINARY(IMAGE_GET_GRAYSCALE_PIXEL_FAST(old_row_ptr, x));
                            IMAGE_PUT_BINARY_PIXEL_FAST(out_row_ptr, x, pixel);
                        }
                    }
                } else {
                    for (int y = 0, yy = img->h; y < yy; y++) {
                        uint8_t *old_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                        uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&bmp, y);
                        uint32_t *out_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y);
                        for (int x = 0, xx = img->w; x < xx; x++) {
                            int pixel = COLOR_GRAYSCALE_TO_BINARY(IMAGE_GET_GRAYSCALE_PIXEL_FAST(old_row_ptr, x));
                            if (((!mask) || image_get_mask_pixel(mask, x, y))
                                && IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr, x)) pixel = 0;
                            IMAGE_PUT_BINARY_PIXEL_FAST(out_row_ptr, x, pixel);
                        }
                    }
                }
            } else {
                if (!zero) {
                    for (int y = 0, yy = img->h; y < yy; y++) {
                        uint8_t *old_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                        uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&bmp, y);
                        uint8_t *out_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(out, y);
                        for (int x = 0, xx = img->w; x < xx; x++) {
                            int pixel = ((!mask) || image_get_mask_pixel(mask, x, y))
                                ? COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr, x))
                                : IMAGE_GET_GRAYSCALE_PIXEL_FAST(old_row_ptr, x);
                            IMAGE_PUT_GRAYSCALE_PIXEL_FAST(out_row_ptr, x, pixel);
                        }
                    }
                } else {
                    for (int y = 0, yy = img->h; y < yy; y++) {
                        uint8_t *old_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                        uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&bmp, y);
                        uint8_t *out_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(out, y);
                        for (int x = 0, xx = img->w; x < xx; x++) {
                            int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(old_row_ptr, x);
                            if (((!mask) || image_get_mask_pixel(mask, x, y))
                                && IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr, x)) pixel = 0;
                            IMAGE_PUT_GRAYSCALE_PIXEL_FAST(out_row_ptr, x, pixel);
                        }
                    }
                }
            }
            break;
        }
        case IMAGE_BPP_RGB565: {
            if (out->bpp == IMAGE_BPP_BINARY) {
                if (!zero) {
                    for (int y = 0, yy = img->h; y < yy; y++) {
                        uint16_t *old_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                        uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&bmp, y);
                        uint32_t *out_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y);
                        for (int x = 0, xx = img->w; x < xx; x++) {
                            int pixel = ((!mask) || image_get_mask_pixel(mask, x, y))
                                ? IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr, x)
                                : COLOR_RGB565_TO_BINARY(IMAGE_GET_RGB565_PIXEL_FAST(old_row_ptr, x));
                            IMAGE_PUT_BINARY_PIXEL_FAST(out_row_ptr, x, pixel);
                        }
                    }
                } else {
                    for (int y = 0, yy = img->h; y < yy; y++) {
                        uint16_t *old_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                        uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&bmp, y);
                        uint32_t *out_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y);
                        for (int x = 0, xx = img->w; x < xx; x++) {
                            int pixel = COLOR_RGB565_TO_BINARY(IMAGE_GET_RGB565_PIXEL_FAST(old_row_ptr, x));
                            if (((!mask) || image_get_mask_pixel(mask, x, y))
                                && IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr, x)) pixel = 0;
                            IMAGE_PUT_BINARY_PIXEL_FAST(out_row_ptr, x, pixel);
                        }
                    }
                }
            } else {
                if (!zero) {
                    for (int y = 0, yy = img->h; y < yy; y++) {
                        uint16_t *old_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                        uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&bmp, y);
                        uint16_t *out_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(out, y);
                        for (int x = 0, xx = img->w; x < xx; x++) {
                            int pixel = ((!mask) || image_get_mask_pixel(mask, x, y))
                                ? COLOR_BINARY_TO_RGB565(IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr, x))
                                : IMAGE_GET_RGB565_PIXEL_FAST(old_row_ptr, x);
                            IMAGE_PUT_RGB565_PIXEL_FAST(out_row_ptr, x, pixel);
                        }
                    }
                } else {
                    for (int y = 0, yy = img->h; y < yy; y++) {
                        uint16_t *old_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                        uint32_t *bmp_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&bmp, y);
                        uint16_t *out_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(out, y);
                        for (int x = 0, xx = img->w; x < xx; x++) {
                            int pixel = IMAGE_GET_RGB565_PIXEL_FAST(old_row_ptr, x);
                            if (((!mask) || image_get_mask_pixel(mask, x, y))
                                && IMAGE_GET_BINARY_PIXEL_FAST(bmp_row_ptr, x)) pixel = 0;
                            IMAGE_PUT_RGB565_PIXEL_FAST(out_row_ptr, x, pixel);
                        }
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }

    memory_free(BANK_MATH, bmp.data);
}

static void binary_erode_dilate(image_t *img, int ksize, int threshold, int e_or_d, image_t *mask)
{
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.bpp = img->bpp;

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            buf.data = memory_alloc(BANK_MATH, (IMAGE_BINARY_LINE_LEN_BYTES(img) * brows));
            LOG_ASSERT("IMAGE", buf.data);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));
                int acc = 0;

                for (int x = 0, xx = img->w; x < xx; x++) {
                    int pixel = IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x);
                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);

                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        continue; // Short circuit.
                    }
                    if (x > ksize && x < img->w-ksize && y >= ksize && y < img->h-ksize) { // faster
                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,y+j);
                            // subtract old left column and add new right column
                            acc -= IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,x-ksize-1);
                            acc += IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,x+ksize);
                        }
                    } else { // slower (checks boundaries per pixel)
                        acc = e_or_d ? 0 : -1; // Don't count center pixel...
                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                                RTE_MIN(RTE_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                acc += IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
                                    RTE_MIN(RTE_MAX(x + k, 0), (img->w - 1)));
                            }
                        }
                    }

                    if (!e_or_d) {
                        // Preserve original pixel value... or clear it.
                        if (acc < threshold) IMAGE_CLEAR_BINARY_PIXEL_FAST(buf_row_ptr, x);
                    } else {
                        // Preserve original pixel value... or set it.
                        if (acc > threshold) IMAGE_SET_BINARY_PIXEL_FAST(buf_row_ptr, x);
                    }
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
                int acc = 0;

                for (int x = 0, xx = img->w; x < xx; x++) {
                    int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);

                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        continue; // Short circuit.
                    }

                    if (x > ksize && x < img->w-ksize && y >= ksize && y < img->h-ksize) { // faster
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,y+j);
                            // subtract old left edge and add new right edge to sum
                            acc -= (IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,x-ksize-1) & 1); // values have already been thresholded to 0x00 or 0xFF
                            acc += (IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,x+ksize) & 1);
                        } // for j
                    } else { // slower way which checks boundaries per pixel
                        acc = e_or_d ? 0 : -1; // Don't count center pixel...
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                                RTE_MIN(RTE_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                acc += ((IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                    RTE_MIN(RTE_MAX(x + k, 0), (img->w - 1)))) >> 7);
                            }  // for k
                        } // for j
                    }

                    if (!e_or_d) {
                        // Preserve original pixel value... or clear it.
                        if (acc < threshold) IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x,
                                                                            COLOR_GRAYSCALE_BINARY_MIN);
                    } else {
                        // Preserve original pixel value... or set it.
                        if (acc > threshold) IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x,
                                                                            COLOR_GRAYSCALE_BINARY_MAX);
                    }
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
                int acc = 0;

                for (int x = 0, xx = img->w; x < xx; x++) {
                    int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x);
                    IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);

                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        continue; // Short circuit.
                    }

                    if (x > ksize && x < img->w-ksize && y >= ksize && y < img->h-ksize) { // faster
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,y+j);
                            // subtract old left column and add new right column
                            acc -= IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,x-ksize-1) & 1; // already 0 or FFFF
                            acc += IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,x+ksize) & 1; // (pre-thresholded)
                        }
                    } else { // need to check boundary conditions for each pixel
                        acc = e_or_d ? 0 : -1; // Don't count center pixel...
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                                RTE_MIN(RTE_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                acc += (IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                    RTE_MIN(RTE_MAX(x + k, 0), (img->w - 1)))) & 1; // already 0 or FFFF
                            }
                        }
                    }

                    if (!e_or_d) {
                        // Preserve original pixel value... or clear it.
                        if (acc < threshold) IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x,
                                                                         COLOR_RGB565_BINARY_MIN);
                    } else {
                        // Preserve original pixel value... or set it.
                        if (acc > threshold) IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x,
                                                                         COLOR_RGB565_BINARY_MAX);
                    }
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

void binary_erode(image_t *img, int ksize, int threshold, image_t *mask)
{
    // Threshold should be equal to (((ksize*2)+1)*((ksize*2)+1))-1
    // for normal operation. E.g. for ksize==3 -> threshold==8
    // Basically you're adjusting the number of data that
    // must be set in the kernel (besides the center) for the output to be 1.
    // Erode normally requires all data to be 1.
    binary_erode_dilate(img, ksize, threshold, 0, mask);
}

