/**
 * @file ds_image.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-08
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __DS_IMAGE_H
#define __DS_IMAGE_H

#if defined (__ARMCC_VERSION) || defined(__CC_ARM) || defined(__CLANG_ARM)
#include <cmsis_compiler.h>
#endif

#include "../middle_layer/rte.h"

/**
 * @brief Opaque structure representing the actual linked list descriptor
 */
typedef struct _linked_list_s linked_list_t;

typedef enum image_bpp
{
    IMAGE_BPP_BINARY,       // BPP = 0
    IMAGE_BPP_GRAYSCALE,    // BPP = 1
    IMAGE_BPP_RGB565,       // BPP = 2
    IMAGE_BPP_BAYER,        // BPP = 3
    IMAGE_BPP_JPEG          // BPP > 3
} image_bpp_t;

typedef struct image {
    int32_t w;
    int32_t h;
    uint32_t bpp;
    uint8_t *data;
} image_t;

#define IMAGE_IS_MUTABLE(image) \
({ \
    __typeof__ (image) _image = (image); \
    (_image->bpp == IMAGE_BPP_BINARY) || \
    (_image->bpp == IMAGE_BPP_GRAYSCALE) || \
    (_image->bpp == IMAGE_BPP_RGB565); \
})

#define IMAGE_IS_MUTABLE_BAYER(image) \
({ \
    __typeof__ (image) _image = (image); \
    (_image->bpp == IMAGE_BPP_BINARY) || \
    (_image->bpp == IMAGE_BPP_GRAYSCALE) || \
    (_image->bpp == IMAGE_BPP_RGB565) || \
    (_image->bpp == IMAGE_BPP_BAYER); \
})

#define IMAGE_IS_MUTABLE_BAYER_JPEG(image) \
({ \
    __typeof__ (image) _image = (image); \
    (_image->bpp == IMAGE_BPP_BINARY) || \
    (_image->bpp == IMAGE_BPP_GRAYSCALE) || \
    (_image->bpp == IMAGE_BPP_RGB565) || \
    (_image->bpp == IMAGE_BPP_BAYER) || \
    (_image->bpp >= IMAGE_BPP_JPEG); \
})

#define IMAGE_IS_COLOR(image) \
({ \
    __typeof__ (image) _image = (image); \
    (_image->bpp == IMAGE_BPP_RGB565) || \
    (_image->bpp == IMAGE_BPP_BAYER) || \
    (_image->bpp >= IMAGE_BPP_JPEG); \
})

#define IMAGE_BINARY_LINE_LEN(image) (((image)->w + UINT32_T_MASK) >> UINT32_T_SHIFT)
#define IMAGE_BINARY_LINE_LEN_BYTES(image) (IMAGE_BINARY_LINE_LEN(image) * sizeof(uint32_t))

#define IMAGE_GRAYSCALE_LINE_LEN(image) ((image)->w)
#define IMAGE_GRAYSCALE_LINE_LEN_BYTES(image) (IMAGE_GRAYSCALE_LINE_LEN(image) * sizeof(uint8_t))

#define IMAGE_RGB565_LINE_LEN(image) ((image)->w)
#define IMAGE_RGB565_LINE_LEN_BYTES(image) (IMAGE_RGB565_LINE_LEN(image) * sizeof(uint16_t))

#define IMAGE_GET_BINARY_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    (((uint32_t *) _image->data)[(((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] >> (_x & UINT32_T_MASK)) & 1; \
})

#define IMAGE_PUT_BINARY_PIXEL(image, x, y, v) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    __typeof__ (v) _v = (v); \
    size_t _i = (((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT); \
    size_t _j = _x & UINT32_T_MASK; \
    ((uint32_t *) _image->data)[_i] = (((uint32_t *) _image->data)[_i] & (~(1 << _j))) | ((_v & 1) << _j); \
})

#define IMAGE_CLEAR_BINARY_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint32_t *) _image->data)[(((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] &= ~(1 << (_x & UINT32_T_MASK)); \
})

#define IMAGE_SET_BINARY_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint32_t *) _image->data)[(((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] |= 1 << (_x & UINT32_T_MASK); \
})

#define IMAGE_GET_GRAYSCALE_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint8_t *) _image->data)[(_image->w * _y) + _x]; \
})

#define IMAGE_PUT_GRAYSCALE_PIXEL(image, x, y, v) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    __typeof__ (v) _v = (v); \
    ((uint8_t *) _image->data)[(_image->w * _y) + _x] = _v; \
})

#define IMAGE_GET_RGB565_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint16_t *) _image->data)[(_image->w * _y) + _x]; \
})

#define IMAGE_PUT_RGB565_PIXEL(image, x, y, v) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    __typeof__ (v) _v = (v); \
    ((uint16_t *) _image->data)[(_image->w * _y) + _x] = _v; \
})

// Fast Stuff //

#define IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(image, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint32_t *) _image->data) + (((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y); \
})

#define IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    (_row_ptr[_x >> UINT32_T_SHIFT] >> (_x & UINT32_T_MASK)) & 1; \
})

#define IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x, v) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    __typeof__ (v) _v = (v); \
    size_t _i = _x >> UINT32_T_SHIFT; \
    size_t _j = _x & UINT32_T_MASK; \
    _row_ptr[_i] = (_row_ptr[_i] & (~(1 << _j))) | ((_v & 1) << _j); \
})

#define IMAGE_CLEAR_BINARY_PIXEL_FAST(row_ptr, x) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x >> UINT32_T_SHIFT] &= ~(1 << (_x & UINT32_T_MASK)); \
})

#define IMAGE_SET_BINARY_PIXEL_FAST(row_ptr, x) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x >> UINT32_T_SHIFT] |= 1 << (_x & UINT32_T_MASK); \
})

#define IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(image, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint8_t *) _image->data) + (_image->w * _y); \
})

#define IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x]; \
})

#define IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x, v) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    __typeof__ (v) _v = (v); \
    _row_ptr[_x] = _v; \
})

#define IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(image, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint16_t *) _image->data) + (_image->w * _y); \
})

#define IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x]; \
})

#define IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, v) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    __typeof__ (v) _v = (v); \
    _row_ptr[_x] = _v; \
})

/////////////////
// Color Stuff //
/////////////////

typedef struct color_thresholds_list_lnk_data
{
    uint8_t LMin, LMax; // or grayscale
    int8_t AMin, AMax;
    int8_t BMin, BMax;
}
color_thresholds_list_lnk_data_t;

#define COLOR_THRESHOLD_BINARY(pixel, threshold, invert) \
({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (threshold) _threshold = (threshold); \
    __typeof__ (invert) _invert = (invert); \
    ((_threshold->LMin <= _pixel) && (_pixel <= _threshold->LMax)) ^ _invert; \
})

#define COLOR_THRESHOLD_GRAYSCALE(pixel, threshold, invert) \
({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (threshold) _threshold = (threshold); \
    __typeof__ (invert) _invert = (invert); \
    ((_threshold->LMin <= _pixel) && (_pixel <= _threshold->LMax)) ^ _invert; \
})

#define COLOR_THRESHOLD_RGB565(pixel, threshold, invert) \
({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (threshold) _threshold = (threshold); \
    __typeof__ (invert) _invert = (invert); \
    uint8_t _l = COLOR_RGB565_TO_L(_pixel); \
    int8_t _a = COLOR_RGB565_TO_A(_pixel); \
    int8_t _b = COLOR_RGB565_TO_B(_pixel); \
    ((_threshold->LMin <= _l) && (_l <= _threshold->LMax) && \
    (_threshold->AMin <= _a) && (_a <= _threshold->AMax) && \
    (_threshold->BMin <= _b) && (_b <= _threshold->BMax)) ^ _invert; \
})

#define COLOR_BOUND_BINARY(pixel0, pixel1, threshold) \
({ \
    __typeof__ (pixel0) _pixel0 = (pixel0); \
    __typeof__ (pixel1) _pixel1 = (pixel1); \
    __typeof__ (threshold) _threshold = (threshold); \
    (abs(_pixel0 - _pixel1) <= _threshold); \
})

#define COLOR_BOUND_GRAYSCALE(pixel0, pixel1, threshold) \
({ \
    __typeof__ (pixel0) _pixel0 = (pixel0); \
    __typeof__ (pixel1) _pixel1 = (pixel1); \
    __typeof__ (threshold) _threshold = (threshold); \
    (abs(_pixel0 - _pixel1) <= _threshold); \
})

#define COLOR_BOUND_RGB565(pixel0, pixel1, threshold) \
({ \
    __typeof__ (pixel0) _pixel0 = (pixel0); \
    __typeof__ (pixel1) _pixel1 = (pixel1); \
    __typeof__ (threshold) _threshold = (threshold); \
    (abs(COLOR_RGB565_TO_R5(_pixel0) - COLOR_RGB565_TO_R5(_pixel1)) <= COLOR_RGB565_TO_R5(_threshold)) && \
    (abs(COLOR_RGB565_TO_G6(_pixel0) - COLOR_RGB565_TO_G6(_pixel1)) <= COLOR_RGB565_TO_G6(_threshold)) && \
    (abs(COLOR_RGB565_TO_B5(_pixel0) - COLOR_RGB565_TO_B5(_pixel1)) <= COLOR_RGB565_TO_B5(_threshold)); \
})

#define COLOR_BINARY_MIN 0
#define COLOR_BINARY_MAX 1
#define COLOR_GRAYSCALE_BINARY_MIN 0x00
#define COLOR_GRAYSCALE_BINARY_MAX 0xFF
#define COLOR_RGB565_BINARY_MIN 0x0000
#define COLOR_RGB565_BINARY_MAX 0xFFFF

#define COLOR_GRAYSCALE_MIN 0
#define COLOR_GRAYSCALE_MAX 255

#define COLOR_R5_MIN 0
#define COLOR_R5_MAX 31
#define COLOR_G6_MIN 0
#define COLOR_G6_MAX 63
#define COLOR_B5_MIN 0
#define COLOR_B5_MAX 31

#define COLOR_R8_MIN 0
#define COLOR_R8_MAX 255
#define COLOR_G8_MIN 0
#define COLOR_G8_MAX 255
#define COLOR_B8_MIN 0
#define COLOR_B8_MAX 255

#define COLOR_L_MIN 0
#define COLOR_L_MAX 100
#define COLOR_A_MIN -128
#define COLOR_A_MAX 127
#define COLOR_B_MIN -128
#define COLOR_B_MAX 127

#define COLOR_Y_MIN 0
#define COLOR_Y_MAX 255
#define COLOR_U_MIN -128
#define COLOR_U_MAX 127
#define COLOR_V_MIN -128
#define COLOR_V_MAX 127

// RGB565 Stuff //

#define COLOR_RGB565_TO_R5(pixel) (((pixel) >> 11) & 0x1F)
#define COLOR_RGB565_TO_R8(pixel) \
({ \
    __typeof__ (pixel) __pixel = (pixel); \
    __pixel = (__pixel >> 8) & 0xF8; \
    __pixel | (__pixel >> 5); \
})

#define COLOR_RGB565_TO_G6(pixel) (((pixel) >> 5) & 0x3F)
#define COLOR_RGB565_TO_G8(pixel) \
({ \
    __typeof__ (pixel) __pixel = (pixel); \
    __pixel = (__pixel >> 3) & 0xFC; \
    __pixel | (__pixel >> 6); \
})

#define COLOR_RGB565_TO_B5(pixel) ((pixel) & 0x1F)
#define COLOR_RGB565_TO_B8(pixel) \
({ \
    __typeof__ (pixel) __pixel = (pixel); \
    __pixel = (__pixel << 3) & 0xF8; \
    __pixel | (__pixel >> 5); \
})

#define COLOR_R5_G6_B5_TO_RGB565(r5, g6, b5) (((r5) << 11) | ((g6) << 5) | (b5))
#define COLOR_R8_G8_B8_TO_RGB565(r8, g8, b8) ((((r8) & 0xF8) << 8) | (((g8) & 0xFC) << 3) | ((b8) >> 3))

#define COLOR_RGB888_TO_Y(r8, g8, b8) ((((r8) * 38) + ((g8) * 75) + ((b8) * 15)) >> 7) // 0.299R + 0.587G + 0.114B
#define COLOR_RGB565_TO_Y(rgb565) \
({ \
    __typeof__ (rgb565) __rgb565 = (rgb565); \
    int r = COLOR_RGB565_TO_R8(__rgb565); \
    int g = COLOR_RGB565_TO_G8(__rgb565); \
    int b = COLOR_RGB565_TO_B8(__rgb565); \
    COLOR_RGB888_TO_Y(r, g, b); \
})

#define COLOR_Y_TO_RGB888(pixel) ((pixel) * 0x010101)
#define COLOR_Y_TO_RGB565(pixel) \
({ \
    __typeof__ (pixel) __pixel = (pixel); \
    int __rb_pixel = (__pixel >> 3) & 0x1F; \
    (__rb_pixel * 0x0801) + ((__pixel << 3) & 0x7E0); \
})

#define COLOR_RGB888_TO_U(r8, g8, b8) ((((r8) * -21) - ((g8) * 43) + ((b8) * 64)) >> 7) // -0.168736R - 0.331264G + 0.5B
#define COLOR_RGB565_TO_U(rgb565) \
({ \
    __typeof__ (rgb565) __rgb565 = (rgb565); \
    int r = COLOR_RGB565_TO_R8(__rgb565); \
    int g = COLOR_RGB565_TO_G8(__rgb565); \
    int b = COLOR_RGB565_TO_B8(__rgb565); \
    COLOR_RGB888_TO_U(r, g, b); \
})

#define COLOR_RGB888_TO_V(r8, g8, b8) ((((r8) * 64) - ((g8) * 54) - ((b8) * 10)) >> 7) // 0.5R - 0.418688G - 0.081312B
#define COLOR_RGB565_TO_V(rgb565) \
({ \
    __typeof__ (rgb565) __rgb565 = (rgb565); \
    int r = COLOR_RGB565_TO_R8(__rgb565); \
    int g = COLOR_RGB565_TO_G8(__rgb565); \
    int b = COLOR_RGB565_TO_B8(__rgb565); \
    COLOR_RGB888_TO_V(r, g, b); \
})

#define COLOR_RGB565_TO_L(pixel) color_rgb565_to_l(pixel)
#define COLOR_RGB565_TO_A(pixel) color_rgb565_to_a(pixel)
#define COLOR_RGB565_TO_B(pixel) color_rgb565_to_b(pixel)

#define COLOR_LAB_TO_RGB565(l, a, b) color_lab_to_rgb(l, a, b)
#define COLOR_YUV_TO_RGB565(y, u, v) color_yuv_to_rgb((y) + 128, u, v)

#define COLOR_BINARY_TO_GRAYSCALE(pixel) ((pixel) * COLOR_GRAYSCALE_MAX)
#define COLOR_BINARY_TO_RGB565(pixel) COLOR_YUV_TO_RGB565(((pixel) ? 127 : -128), 0, 0)
#define COLOR_RGB565_TO_BINARY(pixel) (COLOR_RGB565_TO_Y(pixel) > (((COLOR_Y_MAX - COLOR_Y_MIN) / 2) + COLOR_Y_MIN))
#define COLOR_RGB565_TO_GRAYSCALE(pixel) COLOR_RGB565_TO_Y(pixel)
#define COLOR_GRAYSCALE_TO_BINARY(pixel) ((pixel) > (((COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN) / 2) + COLOR_GRAYSCALE_MIN))
#define COLOR_GRAYSCALE_TO_RGB565(pixel) COLOR_YUV_TO_RGB565(((pixel) - 128), 0, 0)

typedef struct {
    int w;
    int h;
    uint8_t data[10];
} glyph_t;
extern const glyph_t font[95];

void image_init(image_t *ptr, int32_t w, int32_t h, image_bpp_t bpp);
void image_reuse(image_t *ptr);
void image_copy(image_t *dst, image_t *src);
bool image_get_mask_pixel(image_t *ptr, int x, int y);
int image_get_pixel_fast(int img_bpp, const void *row_ptr, int x);
void image_set_pixel(image_t *img, int x, int y, int p);
void image_draw_line(image_t *img, int x0, int y0, int x1, int y1, int c, int thickness);
void image_draw_rectangle(image_t *img, int rx, int ry, int rw, int rh, int c, int thickness, bool fill);
void image_draw_circle(image_t *img, int cx, int cy, int r, int c, int thickness, bool fill);
void image_draw_ellipse(image_t *img, int cx, int cy, int rx, int ry, int rotation, int c, int thickness, bool fill);
void image_draw_string(image_t *img, int x_off, int y_off, const char *str, int c, float scale, int x_spacing, int y_spacing, bool mono_space,
                       int char_rotation, bool char_hmirror, bool char_vflip, int string_rotation, bool string_hmirror, bool string_hflip);

size_t image_size(image_t *ptr);

void *data_malloc_calculate(uint32_t size);
void *data_calloc_calculate(uint32_t size);
void *data_malloc_object(uint32_t size);
void *data_calloc_object(uint32_t size);
void data_free(void *ptr);

extern const float cos_table[360];
extern const float sin_table[360];

int8_t color_rgb565_to_l(uint16_t pixel);
int8_t color_rgb565_to_a(uint16_t pixel);
int8_t color_rgb565_to_b(uint16_t pixel);
uint16_t color_lab_to_rgb(uint8_t l, int8_t a, int8_t b);
uint16_t color_yuv_to_rgb(uint8_t y, int8_t u, int8_t v);

////////////////
// Line Stuff //
////////////////
typedef struct line {
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
} line_t;

bool line_clip(line_t *l, int x, int y, int w, int h);

/////////////////////
// Rectangle Stuff //
/////////////////////

typedef struct rectangle {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} rectangle_t;

void rectangle_init(rectangle_t *ptr, int x, int y, int w, int h);
void rectangle_copy(rectangle_t *dst, rectangle_t *src);
bool rectangle_equal_fast(rectangle_t *ptr0, rectangle_t *ptr1);
bool rectangle_overlap(rectangle_t *ptr0, rectangle_t *ptr1);
void rectangle_intersected(rectangle_t *dst, rectangle_t *src);
void rectangle_united(rectangle_t *dst, rectangle_t *src);

/////////////////
// Point Stuff //
/////////////////

typedef struct point {
    int16_t x;
    int16_t y;
} point_t;

void point_init(point_t *ptr, int x, int y);
void point_copy(point_t *dst, point_t *src);
bool point_equal_fast(point_t *ptr0, point_t *ptr1);
int point_quadrance(point_t *ptr0, point_t *ptr1);
void point_rotate(int x, int y, float r, int center_x, int center_y, int16_t *new_x, int16_t *new_y);
void point_min_area_rectangle(point_t *corners, point_t *new_corners, int corners_len);


/////////////////
// filter Stuff //
/////////////////
void filter_morph(image_t *img, int ksize,int *krn, float m, int b, bool threshold, int offset, bool invert, image_t *mask);


/////////////////
// binary Stuff //
/////////////////
void binary_image(image_t *out, image_t *img, linked_list_t *thresholds, bool invert, bool zero, image_t *mask);
void binary_erode(image_t *img, int ksize, int threshold, image_t *mask);

/////////////////
// Conv & denoise Stuff //
/////////////////
typedef enum {
    GAUSSIAN = 0,
    LAPLACIAN = 1,
    HIGHPASS = 2,
} denoise_type_t;

// Conv kernels
extern const int8_t kernel_gauss_3[9];
extern const int8_t kernel_gauss_5[25];
extern const int kernel_laplacian_3[9];
extern const int kernel_high_pass_3[9];

void denoise_sepconv3(image_t *img, denoise_type_t filter_type, float m, int32_t b);

/////////////////
// edge Stuff //
/////////////////
void edge_canny(image_t *src, rectangle_t *roi, int32_t low_thresh, int32_t high_thresh);
void edge_simple(image_t *src, rectangle_t *roi, int low_thresh, int high_thresh);

/////////////////
// Search & find Stuff //
/////////////////

typedef struct find_lines_list_lnk_data {
    line_t line;
    uint32_t magnitude;
    int16_t theta, rho;
} find_lines_list_lnk_data_t;

typedef struct find_circles_list_lnk_data {
    point_t p;
    uint16_t r, magnitude;
} find_circles_list_lnk_data_t;

linked_list_t *hough_find_lines(image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                      uint32_t threshold, unsigned int theta_margin, unsigned int rho_margin);

linked_list_t *hough_find_circles(image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                        uint32_t threshold, unsigned int x_margin, unsigned int y_margin, unsigned int r_margin,
                        unsigned int r_min, unsigned int r_max, unsigned int r_step);

#endif
