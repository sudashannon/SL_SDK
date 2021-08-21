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

const float xyz_table[256] = {
     0.000000f,  0.030353f,  0.060705f,  0.091058f,  0.121411f,  0.151763f,  0.182116f,  0.212469f,
     0.242822f,  0.273174f,  0.303527f,  0.334654f,  0.367651f,  0.402472f,  0.439144f,  0.477695f,
     0.518152f,  0.560539f,  0.604883f,  0.651209f,  0.699541f,  0.749903f,  0.802319f,  0.856813f,
     0.913406f,  0.972122f,  1.032982f,  1.096009f,  1.161225f,  1.228649f,  1.298303f,  1.370208f,
     1.444384f,  1.520851f,  1.599629f,  1.680738f,  1.764195f,  1.850022f,  1.938236f,  2.028856f,
     2.121901f,  2.217388f,  2.315337f,  2.415763f,  2.518686f,  2.624122f,  2.732089f,  2.842604f,
     2.955683f,  3.071344f,  3.189603f,  3.310477f,  3.433981f,  3.560131f,  3.688945f,  3.820437f,
     3.954624f,  4.091520f,  4.231141f,  4.373503f,  4.518620f,  4.666509f,  4.817182f,  4.970657f,
     5.126946f,  5.286065f,  5.448028f,  5.612849f,  5.780543f,  5.951124f,  6.124605f,  6.301002f,
     6.480327f,  6.662594f,  6.847817f,  7.036010f,  7.227185f,  7.421357f,  7.618538f,  7.818742f,
     8.021982f,  8.228271f,  8.437621f,  8.650046f,  8.865559f,  9.084171f,  9.305896f,  9.530747f,
     9.758735f,  9.989873f, 10.224173f, 10.461648f, 10.702310f, 10.946171f, 11.193243f, 11.443537f,
    11.697067f, 11.953843f, 12.213877f, 12.477182f, 12.743768f, 13.013648f, 13.286832f, 13.563333f,
    13.843162f, 14.126329f, 14.412847f, 14.702727f, 14.995979f, 15.292615f, 15.592646f, 15.896084f,
    16.202938f, 16.513219f, 16.826940f, 17.144110f, 17.464740f, 17.788842f, 18.116424f, 18.447499f,
    18.782077f, 19.120168f, 19.461783f, 19.806932f, 20.155625f, 20.507874f, 20.863687f, 21.223076f,
    21.586050f, 21.952620f, 22.322796f, 22.696587f, 23.074005f, 23.455058f, 23.839757f, 24.228112f,
    24.620133f, 25.015828f, 25.415209f, 25.818285f, 26.225066f, 26.635560f, 27.049779f, 27.467731f,
    27.889426f, 28.314874f, 28.744084f, 29.177065f, 29.613827f, 30.054379f, 30.498731f, 30.946892f,
    31.398871f, 31.854678f, 32.314321f, 32.777810f, 33.245154f, 33.716362f, 34.191442f, 34.670406f,
    35.153260f, 35.640014f, 36.130678f, 36.625260f, 37.123768f, 37.626212f, 38.132601f, 38.642943f,
    39.157248f, 39.675523f, 40.197778f, 40.724021f, 41.254261f, 41.788507f, 42.326767f, 42.869050f,
    43.415364f, 43.965717f, 44.520119f, 45.078578f, 45.641102f, 46.207700f, 46.778380f, 47.353150f,
    47.932018f, 48.514994f, 49.102085f, 49.693300f, 50.288646f, 50.888132f, 51.491767f, 52.099557f,
    52.711513f, 53.327640f, 53.947949f, 54.572446f, 55.201140f, 55.834039f, 56.471151f, 57.112483f,
    57.758044f, 58.407842f, 59.061884f, 59.720179f, 60.382734f, 61.049557f, 61.720656f, 62.396039f,
    63.075714f, 63.759687f, 64.447968f, 65.140564f, 65.837482f, 66.538730f, 67.244316f, 67.954247f,
    68.668531f, 69.387176f, 70.110189f, 70.837578f, 71.569350f, 72.305513f, 73.046074f, 73.791041f,
    74.540421f, 75.294222f, 76.052450f, 76.815115f, 77.582222f, 78.353779f, 79.129794f, 79.910274f,
    80.695226f, 81.484657f, 82.278575f, 83.076988f, 83.879901f, 84.687323f, 85.499261f, 86.315721f,
    87.136712f, 87.962240f, 88.792312f, 89.626935f, 90.466117f, 91.309865f, 92.158186f, 93.011086f,
    93.868573f, 94.730654f, 95.597335f, 96.468625f, 97.344529f, 98.225055f, 99.110210f, 100.000000f
};

int8_t color_rgb565_to_l(uint16_t pixel)
{
    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)];
    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)];
    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)];

    float y = ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) * (1.0f / 100.000f);

    y = (y>0.008856f) ? cbrtf(y) : ((y * 7.787037f) + 0.137931f);

    return floorf(116 * y) - 16;
}

int8_t color_rgb565_to_a(uint16_t pixel)
{
    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)];
    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)];
    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)];

    float x = ((r_lin * 0.4124f) + (g_lin * 0.3576f) + (b_lin * 0.1805f)) * (1.0f / 095.047f);
    float y = ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) * (1.0f / 100.000f);

    x = (x>0.008856f) ? cbrtf(x) : ((x * 7.787037f) + 0.137931f);
    y = (y>0.008856f) ? cbrtf(y) : ((y * 7.787037f) + 0.137931f);

    return floorf(500 * (x-y));
}

int8_t color_rgb565_to_b(uint16_t pixel)
{
    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)];
    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)];
    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)];

    float y = ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) * (1.0f / 100.000f);
    float z = ((r_lin * 0.0193f) + (g_lin * 0.1192f) + (b_lin * 0.9505f)) * (1.0f / 108.883f);

    y = (y>0.008856f) ? cbrtf(y) : ((y * 7.787037f) + 0.137931f);
    z = (z>0.008856f) ? cbrtf(z) : ((z * 7.787037f) + 0.137931f);

    return floorf(200 * (y-z));
}

// https://en.wikipedia.org/wiki/Lab_color_space -> CIELAB-CIEXYZ conversions
// https://en.wikipedia.org/wiki/SRGB -> Specification of the transformation
uint16_t color_lab_to_rgb(uint8_t l, int8_t a, int8_t b)
{
    float x = ((l + 16) * 0.008621f) + (a * 0.002f);
    float y = ((l + 16) * 0.008621f);
    float z = ((l + 16) * 0.008621f) - (b * 0.005f);

    x = ((x > 0.206897f) ? (x*x*x) : ((0.128419f * x) - 0.017713f)) * 095.047f;
    y = ((y > 0.206897f) ? (y*y*y) : ((0.128419f * y) - 0.017713f)) * 100.000f;
    z = ((z > 0.206897f) ? (z*z*z) : ((0.128419f * z) - 0.017713f)) * 108.883f;

    float r_lin = ((x * +3.2406f) + (y * -1.5372f) + (z * -0.4986f)) / 100.0f;
    float g_lin = ((x * -0.9689f) + (y * +1.8758f) + (z * +0.0415f)) / 100.0f;
    float b_lin = ((x * +0.0557f) + (y * -0.2040f) + (z * +1.0570f)) / 100.0f;

    r_lin = (r_lin>0.0031308f) ? ((1.055f*powf(r_lin, 0.416666f))-0.055f) : (r_lin*12.92f);
    g_lin = (g_lin>0.0031308f) ? ((1.055f*powf(g_lin, 0.416666f))-0.055f) : (g_lin*12.92f);
    b_lin = (b_lin>0.0031308f) ? ((1.055f*powf(b_lin, 0.416666f))-0.055f) : (b_lin*12.92f);

    uint32_t red   = RTE_MAX(RTE_MIN(floorf(r_lin * COLOR_R8_MAX), COLOR_R8_MAX), COLOR_R8_MIN);
    uint32_t green = RTE_MAX(RTE_MIN(floorf(g_lin * COLOR_G8_MAX), COLOR_G8_MAX), COLOR_G8_MIN);
    uint32_t blue  = RTE_MAX(RTE_MIN(floorf(b_lin * COLOR_B8_MAX), COLOR_B8_MAX), COLOR_B8_MIN);

    return COLOR_R8_G8_B8_TO_RGB565(red, green, blue);
}

// https://en.wikipedia.org/wiki/YCbCr -> JPEG Conversion
uint16_t color_yuv_to_rgb(uint8_t y, int8_t u, int8_t v)
{
    uint32_t r = RTE_MAX(RTE_MIN(y + ((91881 * v) >> 16), COLOR_R8_MAX), COLOR_R8_MIN);
    uint32_t g = RTE_MAX(RTE_MIN(y - (((22554 * u) + (46802 * v)) >> 16), COLOR_G8_MAX), COLOR_G8_MIN);
    uint32_t b = RTE_MAX(RTE_MIN(y + ((116130 * u) >> 16), COLOR_B8_MAX), COLOR_B8_MIN);

    return COLOR_R8_G8_B8_TO_RGB565(r, g, b);
}

void image_init(image_t *ptr, int w, int h, image_bpp_t bpp)
{
    ptr->w = w;
    ptr->h = h;
    ptr->bpp = bpp;
    // data will be allocated in senser snap shot API.
    ptr->data = NULL;
}

void image_copy(image_t *dst, image_t *src)
{
    memcpy(dst, src, sizeof(image_t));
}

void image_reuse(image_t *ptr)
{
    memory_free(BANK_FB, ptr->data);
    ptr->data = NULL;
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
