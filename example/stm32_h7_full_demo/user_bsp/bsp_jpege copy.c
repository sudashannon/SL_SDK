/**
 * @file bsp_jpege.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-09-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "bsp_jpege.h"
#include "jpeg.h"

#define MCU_W                       (8)
#define MCU_H                       (8)
#define JPEG_444_GS_MCU_SIZE        ((MCU_W) * (MCU_H))
#define JPEG_444_YCBCR_MCU_SIZE     ((JPEG_444_GS_MCU_SIZE) * 3)

#define JPEG_BINARY_0 0x00
#define JPEG_BINARY_1 0xFF

// Expand 4 bits to 32 for binary to grayscale - process 4 pixels at a time
static const uint32_t jpeg_expand[16] = {0x0, 0xff, 0xff00, 0xffff, 0xff0000,
    0xff00ff, 0xffff00, 0xffffff, 0xff000000, 0xff0000ff, 0xff00ff00,
    0xff00ffff, 0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff};

static int jpeg_clean_trailing_bytes(int bpp, uint8_t *data)
{
    while ((bpp > 1) && ((data[bpp-2] != 0xFF) || (data[bpp-1] != 0xD9))) {
        bpp -= 1;
    }

    return bpp;
}

static void jpeg_get_mcu(image_t *src, int x_offset, int y_offset, int dx, int dy, int8_t *Y0, int8_t *CB, int8_t *CR)
{
    switch (src->bpp) {
        case IMAGE_BPP_BINARY: {
            if ((dx != MCU_W) || (dy != MCU_H)) { // partial MCU, fill with 0's to start
                memset(Y0, 0, JPEG_444_GS_MCU_SIZE);
            }

            for (int y = y_offset, yy = y + dy; y < yy; y++) {
                uint32_t *rp = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(src, y);
                uint8_t pixels = rp[x_offset >> UINT32_T_SHIFT] >> (x_offset & UINT32_T_MASK);

                if (dx == MCU_W) {
                    *((uint32_t *) Y0) = jpeg_expand[pixels & 0xf];
                    *(((uint32_t *) Y0) + 1) = jpeg_expand[pixels >> 4];
                } else if (dx >= 4) {
                    *((uint32_t *) Y0) = jpeg_expand[pixels & 0xf];

                    if (dx >= 6) {
                        *(((uint16_t *) Y0) + 2) = jpeg_expand[pixels >> 4];

                        if (dx & 1) {
                            Y0[6] = (pixels & 0x40) ? JPEG_BINARY_1 : JPEG_BINARY_0;
                        }
                    } else if (dx & 1) {
                        Y0[4] = (pixels & 0x10) ? JPEG_BINARY_1 : JPEG_BINARY_0;
                    }
                } else if (dx >= 2) {
                    *((uint16_t *) Y0) = jpeg_expand[pixels & 0x3];

                    if (dx & 1) {
                        Y0[2] = (pixels & 0x4) ? JPEG_BINARY_1 : JPEG_BINARY_0;
                    }
                } else {
                    *Y0 = (pixels & 0x1) ? JPEG_BINARY_1 : JPEG_BINARY_0;
                }

                Y0 += MCU_W;
            }
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            if ((dx != MCU_W) || (dy != MCU_H)) { // partial MCU, fill with 0's to start
                memset(Y0, 0, JPEG_444_GS_MCU_SIZE);
            }

            for (int y = y_offset, yy = y + dy; y < yy; y++) {
                uint8_t *rp = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(src, y) + x_offset;

                if (dx == MCU_W) {
                    *((uint32_t *) Y0) = *((uint32_t *) rp);
                    *(((uint32_t *) Y0) + 1) = *(((uint32_t *) rp) + 1);
                } else if (dx >= 4) {
                    *((uint32_t *) Y0) = *((uint32_t *) rp);

                    if (dx >= 6) {
                        *(((uint16_t *) Y0) + 2) = *(((uint16_t *) rp) + 2);

                        if (dx & 1) {
                            Y0[6] = rp[6];
                        }
                    } else if (dx & 1) {
                        Y0[4] = rp[4];
                    }
                } else if (dx >= 2) {
                    *((uint16_t *) Y0) = *((uint16_t *) rp);

                    if (dx & 1) {
                        Y0[2] = rp[2];
                    }
                } else {
                    *Y0 = *rp;
                }

                Y0 += MCU_W;
            }
            break;
        }
        case IMAGE_BPP_RGB565: {
            if ((dx != MCU_W) || (dy != MCU_H)) { // partial MCU, fill with 0's to start
                memset(Y0, 0, JPEG_444_YCBCR_MCU_SIZE);
            }

            for (int y = y_offset, yy = y + dy, index = 0; y < yy; y++) {
                uint32_t *rp = (uint32_t *) (IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(src, y) + x_offset);

                for (int x = 0, xx = dx - 1; x < xx; x += 2, index += 2) {
                    int pixels = *rp++;
                    int r_pixels = ((pixels >> 8) & 0xf800f8) | ((pixels >> 13) & 0x70007);
                    int g_pixels = ((pixels >> 3) & 0xfc00fc) | ((pixels >> 9) & 0x30003);
                    int b_pixels = ((pixels << 3) & 0xf800f8) | ((pixels >> 2) & 0x70007);

                    int y = ((r_pixels * 38) + (g_pixels * 75) + (b_pixels * 15)) >> 7;

                    Y0[index] = y, Y0[index + 1] = y >> 16;
                    int u = __SSUB16(b_pixels * 64, (r_pixels * 21) + (g_pixels * 43)) >> 7;
                    u ^= 0x800080;

                    CB[index] = u, CB[index + 1] = u >> 16;

                    int v = __SSUB16(r_pixels * 64, (g_pixels * 54) + (b_pixels * 10)) >> 7;
                    v ^= 0x800080;

                    CR[index] = v, CR[index + 1] = v >> 16;
                }

                if (dx & 1) {
                    int pixel = *((uint16_t *) rp);
                    int r = COLOR_RGB565_TO_R8(pixel);
                    int g = COLOR_RGB565_TO_G8(pixel);
                    int b = COLOR_RGB565_TO_B8(pixel);

                    int y0 = COLOR_RGB888_TO_Y(r, g, b);
                    Y0[index] = y0;

                    int cb = COLOR_RGB888_TO_U(r, g, b);
                    cb ^= 0x80;

                    CB[index] = cb;

                    int cr = COLOR_RGB888_TO_V(r, g, b);
                    cr ^= 0x80;

                    CR[index++] = cr;
                }

                index += MCU_W - dx;
            }
            break;
        }
        case IMAGE_BPP_BAYER: {
            if ((dx != MCU_W) || (dy != MCU_H)) { // partial MCU, fill with 0's to start
                memset(Y0, 0, JPEG_444_YCBCR_MCU_SIZE);
            }

            int src_w = src->w, w_limit = src_w - 1, w_limit_m_1 = w_limit - 1;
            int src_h = src->h, h_limit = src_h - 1, h_limit_m_1 = h_limit - 1;

            if (x_offset && y_offset && (x_offset < (src_w - MCU_W)) && (y_offset < (src_h - MCU_H))) {
                for (int y = y_offset - 1, yy = y + MCU_H - 1, index_e = 0, index_o = MCU_W; y < yy; y += 2,
                        index_e += MCU_W,
                        index_o += MCU_W) {
                    uint8_t *rowptr_grgr_0 = src->data + (y * src_w);
                    uint8_t *rowptr_bgbg_1 = rowptr_grgr_0 + src_w;
                    uint8_t *rowptr_grgr_2 = rowptr_bgbg_1 + src_w;
                    uint8_t *rowptr_bgbg_3 = rowptr_grgr_2 + src_w;

                    for (int x = x_offset - 1, xx = x + MCU_W - 1; x < xx; x += 2, index_e += 2, index_o += 2) {
                        uint32_t row_grgr_0 = *((uint32_t *) (rowptr_grgr_0 + x));
                        uint32_t row_bgbg_1 = *((uint32_t *) (rowptr_bgbg_1 + x));
                        uint32_t row_grgr_2 = *((uint32_t *) (rowptr_grgr_2 + x));
                        uint32_t row_bgbg_3 = *((uint32_t *) (rowptr_bgbg_3 + x));

                        int row_01 = __UHADD8(row_grgr_0, row_grgr_2);
                        int row_1g = __UHADD8(row_bgbg_1, __PKHTB(row_bgbg_1, row_bgbg_1, 16));

                        int r_pixels_0 = __UXTB16(__UHADD8(row_01, __PKHTB(row_01, row_01, 16)));
                        int g_pixels_0 = __UXTB16(__UHADD8(row_1g, __PKHTB(row_1g, row_01, 8)));
                        int b_pixels_0 = __UXTB16(__ROR(__UHADD8(row_bgbg_1, __PKHBT(row_bgbg_1, row_bgbg_1, 16)), 8));

                        int y0 = ((r_pixels_0 * 38) + (g_pixels_0 * 75) + (b_pixels_0 * 15)) >> 7;

                        Y0[index_e] = y0, Y0[index_e + 1] = y0 >> 16;

                        int u0 = __SSUB16(b_pixels_0 * 64, (r_pixels_0 * 21) + (g_pixels_0 * 43)) >> 7;
                        u0 ^= 0x800080;

                        CB[index_e] = u0, CB[index_e + 1] = u0 >> 16;

                        int v0 = __SSUB16(r_pixels_0 * 64, (g_pixels_0 * 54) + (b_pixels_0 * 10)) >> 7;
                        v0 ^= 0x800080;

                        CR[index_e] = v0, CR[index_e + 1] = v0 >> 16;

                        int row_13 = __UHADD8(row_bgbg_1, row_bgbg_3);
                        int row_2g = __UHADD8(row_grgr_2, __PKHBT(row_grgr_2, row_grgr_2, 16));

                        int r_pixels_1 = __UXTB16(__UHADD8(row_grgr_2, __PKHTB(row_grgr_2, row_grgr_2, 16)));
                        int g_pixels_1 = __UXTB16(__ROR(__UHADD8(row_2g, __PKHBT(row_2g, row_13, 8)), 8));
                        int b_pixels_1 = __UXTB16(__ROR(__UHADD8(row_13, __PKHBT(row_13, row_13, 16)), 8));

                        int y1 = ((r_pixels_1 * 38) + (g_pixels_1 * 75) + (b_pixels_1 * 15)) >> 7;

                        Y0[index_o] = y1, Y0[index_o + 1] = y1 >> 16;

                        int u1 = __SSUB16(b_pixels_1 * 64, (r_pixels_1 * 21) + (g_pixels_1 * 43)) >> 7;
                        u1 ^= 0x800080;

                        CB[index_o] = u1, CB[index_o + 1] = u1 >> 16;

                        int v1 = __SSUB16(r_pixels_1 * 64, (g_pixels_1 * 54) + (b_pixels_1 * 10)) >> 7;
                        v1 ^= 0x800080;

                        CR[index_o] = v1, CR[index_o + 1] = v1 >> 16;
                    }
                }
            } else {
                // If dy is odd this loop will produce 1 extra boundary row in the MCU.
                // This is okay given the boundary checking code below.
                for (int y = y_offset, yy = y + dy, index_e = 0, index_o = MCU_W; y < yy; y += 2) {
                    uint8_t *rowptr_grgr_0, *rowptr_bgbg_1, *rowptr_grgr_2, *rowptr_bgbg_3;

                    // keep row pointers in bounds
                    if (y == 0) {
                        rowptr_bgbg_1 = src->data;
                        rowptr_grgr_2 = rowptr_bgbg_1 + ((src_h >= 2) ? src_w : 0);
                        rowptr_bgbg_3 = rowptr_bgbg_1 + ((src_h >= 3) ? (src_w * 2) : 0);
                        rowptr_grgr_0 = rowptr_grgr_2;
                    } else if (y == h_limit_m_1) {
                        rowptr_grgr_0 = src->data + ((y - 1) * src_w);
                        rowptr_bgbg_1 = rowptr_grgr_0 + src_w;
                        rowptr_grgr_2 = rowptr_bgbg_1 + src_w;
                        rowptr_bgbg_3 = rowptr_bgbg_1;
                    } else if (y >= h_limit) {
                        rowptr_grgr_0 = src->data + ((y - 1) * src_w);
                        rowptr_bgbg_1 = rowptr_grgr_0 + src_w;
                        rowptr_grgr_2 = rowptr_grgr_0;
                        rowptr_bgbg_3 = rowptr_bgbg_1;
                    } else { // get 4 neighboring rows
                        rowptr_grgr_0 = src->data + ((y - 1) * src_w);
                        rowptr_bgbg_1 = rowptr_grgr_0 + src_w;
                        rowptr_grgr_2 = rowptr_bgbg_1 + src_w;
                        rowptr_bgbg_3 = rowptr_grgr_2 + src_w;
                    }

                    // If dx is odd this loop will produce 1 extra boundary column in the MCU.
                    // This is okay given the boundary checking code below.
                    for (int x = x_offset, xx = x + dx; x < xx; x += 2, index_e += 2, index_o += 2) {
                        uint32_t row_grgr_0, row_bgbg_1, row_grgr_2, row_bgbg_3;

                        // keep pixels in bounds
                        if (x == 0) {
                            if (src_w >= 4) {
                                row_grgr_0 = *((uint32_t *) rowptr_grgr_0);
                                row_bgbg_1 = *((uint32_t *) rowptr_bgbg_1);
                                row_grgr_2 = *((uint32_t *) rowptr_grgr_2);
                                row_bgbg_3 = *((uint32_t *) rowptr_bgbg_3);
                            } else if (src_w >= 3) {
                                row_grgr_0 = *((uint16_t *) rowptr_grgr_0) | (*(rowptr_grgr_0 + 2) << 16);
                                row_bgbg_1 = *((uint16_t *) rowptr_bgbg_1) | (*(rowptr_bgbg_1 + 2) << 16);
                                row_grgr_2 = *((uint16_t *) rowptr_grgr_2) | (*(rowptr_grgr_2 + 2) << 16);
                                row_bgbg_3 = *((uint16_t *) rowptr_bgbg_3) | (*(rowptr_bgbg_3 + 2) << 16);
                            } else if (src_w >= 2) {
                                row_grgr_0 = *((uint16_t *) rowptr_grgr_0);
                                row_grgr_0 = (row_grgr_0 << 16) | row_grgr_0;
                                row_bgbg_1 = *((uint16_t *) rowptr_bgbg_1);
                                row_bgbg_1 = (row_bgbg_1 << 16) | row_bgbg_1;
                                row_grgr_2 = *((uint16_t *) rowptr_grgr_2);
                                row_grgr_2 = (row_grgr_2 << 16) | row_grgr_2;
                                row_bgbg_3 = *((uint16_t *) rowptr_bgbg_3);
                                row_bgbg_3 = (row_bgbg_3 << 16) | row_bgbg_3;
                            } else {
                                row_grgr_0 = *(rowptr_grgr_0) * 0x01010101;
                                row_bgbg_1 = *(rowptr_bgbg_1) * 0x01010101;
                                row_grgr_2 = *(rowptr_grgr_2) * 0x01010101;
                                row_bgbg_3 = *(rowptr_bgbg_3) * 0x01010101;
                            }
                            // The starting point needs to be offset by 1. The below patterns are actually
                            // rgrg, gbgb, rgrg, and gbgb. So, shift left and backfill the missing border pixel.
                            row_grgr_0 = (row_grgr_0 << 8) | (__ROR(row_grgr_0, 8) & 0xFF);
                            row_bgbg_1 = (row_bgbg_1 << 8) | (__ROR(row_bgbg_1, 8) & 0xFF);
                            row_grgr_2 = (row_grgr_2 << 8) | (__ROR(row_grgr_2, 8) & 0xFF);
                            row_bgbg_3 = (row_bgbg_3 << 8) | (__ROR(row_bgbg_3, 8) & 0xFF);
                        } else if (x == w_limit_m_1) {
                            row_grgr_0 = *((uint32_t *) (rowptr_grgr_0 + x - 2));
                            row_grgr_0 = (row_grgr_0 >> 8) | ((row_grgr_0 << 8) & 0xff000000);
                            row_bgbg_1 = *((uint32_t *) (rowptr_bgbg_1 + x - 2));
                            row_bgbg_1 = (row_bgbg_1 >> 8) | ((row_bgbg_1 << 8) & 0xff000000);
                            row_grgr_2 = *((uint32_t *) (rowptr_grgr_2 + x - 2));
                            row_grgr_2 = (row_grgr_2 >> 8) | ((row_grgr_2 << 8) & 0xff000000);
                            row_bgbg_3 = *((uint32_t *) (rowptr_bgbg_3 + x - 2));
                            row_bgbg_3 = (row_bgbg_3 >> 8) | ((row_bgbg_1 << 8) & 0xff000000);
                        } else if (x >= w_limit) {
                            row_grgr_0 = *((uint16_t *) (rowptr_grgr_0 + x - 1));
                            row_grgr_0 = (row_grgr_0 << 16) | row_grgr_0;
                            row_bgbg_1 = *((uint16_t *) (rowptr_bgbg_1 + x - 1));
                            row_bgbg_1 = (row_bgbg_1 << 16) | row_bgbg_1;
                            row_grgr_2 = *((uint16_t *) (rowptr_grgr_2 + x - 1));
                            row_grgr_2 = (row_grgr_2 << 16) | row_grgr_2;
                            row_bgbg_3 = *((uint16_t *) (rowptr_bgbg_3 + x - 1));
                            row_bgbg_3 = (row_bgbg_3 << 16) | row_bgbg_3;
                        } else { // get 4 neighboring rows
                            row_grgr_0 = *((uint32_t *) (rowptr_grgr_0 + x - 1));
                            row_bgbg_1 = *((uint32_t *) (rowptr_bgbg_1 + x - 1));
                            row_grgr_2 = *((uint32_t *) (rowptr_grgr_2 + x - 1));
                            row_bgbg_3 = *((uint32_t *) (rowptr_bgbg_3 + x - 1));
                        }
                        int row_01 = __UHADD8(row_grgr_0, row_grgr_2);
                        int row_1g = __UHADD8(row_bgbg_1, __PKHTB(row_bgbg_1, row_bgbg_1, 16));

                        int r_pixels_0 = __UXTB16(__UHADD8(row_01, __PKHTB(row_01, row_01, 16)));
                        int g_pixels_0 = __UXTB16(__UHADD8(row_1g, __PKHTB(row_1g, row_01, 8)));
                        int b_pixels_0 = __UXTB16(__ROR(__UHADD8(row_bgbg_1, __PKHBT(row_bgbg_1, row_bgbg_1, 16)), 8));

                        int y0 = ((r_pixels_0 * 38) + (g_pixels_0 * 75) + (b_pixels_0 * 15)) >> 7;

                        Y0[index_e] = y0, Y0[index_e + 1] = y0 >> 16;

                        int u0 = __SSUB16(b_pixels_0 * 64, (r_pixels_0 * 21) + (g_pixels_0 * 43)) >> 7;
                        u0 ^= 0x800080;

                        CB[index_e] = u0, CB[index_e + 1] = u0 >> 16;

                        int v0 = __SSUB16(r_pixels_0 * 64, (g_pixels_0 * 54) + (b_pixels_0 * 10)) >> 7;
                        v0 ^= 0x800080;

                        CR[index_e] = v0, CR[index_e + 1] = v0 >> 16;

                        int row_13 = __UHADD8(row_bgbg_1, row_bgbg_3);
                        int row_2g = __UHADD8(row_grgr_2, __PKHBT(row_grgr_2, row_grgr_2, 16));

                        int r_pixels_1 = __UXTB16(__UHADD8(row_grgr_2, __PKHTB(row_grgr_2, row_grgr_2, 16)));
                        int g_pixels_1 = __UXTB16(__ROR(__UHADD8(row_2g, __PKHBT(row_2g, row_13, 8)), 8));
                        int b_pixels_1 = __UXTB16(__ROR(__UHADD8(row_13, __PKHBT(row_13, row_13, 16)), 8));

                        int y1 = ((r_pixels_1 * 38) + (g_pixels_1 * 75) + (b_pixels_1 * 15)) >> 7;

                        Y0[index_o] = y1, Y0[index_o + 1] = y1 >> 16;

                        int u1 = __SSUB16(b_pixels_1 * 64, (r_pixels_1 * 21) + (g_pixels_1 * 43)) >> 7;
                        u1 ^= 0x800080;

                        CB[index_o] = u1, CB[index_o + 1] = u1 >> 16;

                        int v1 = __SSUB16(r_pixels_1 * 64, (g_pixels_1 * 54) + (b_pixels_1 * 10)) >> 7;
                        v1 ^= 0x800080;

                        CR[index_o] = v1, CR[index_o + 1] = v1 >> 16;
                    }

                    int inc = (MCU_W * 2) - (((dx + 1) / 2) * 2); // Handle boundary column.
                    index_e += inc;
                    index_o += inc;
                }
            }
            break;
        }
    }
}

#define FB_ALLOC_PADDING            ((__SCB_DCACHE_LINE_SIZE) * 4)
#define OUTPUT_CHUNK_SIZE           (512) // The minimum output buffer size is 2x this - so 1KB.
#define JPEG_INPUT_FIFO_BYTES       (32)
#define JPEG_OUTPUT_FIFO_BYTES      (32)

static int JPEG_out_data_length_max = 0;
static volatile int JPEG_out_data_length = 0;
static volatile bool JPEG_input_paused = false;
static volatile bool JPEG_output_paused = false;
static JPEG_ConfTypeDef JPEG_Config = {};

// JIFF-APP0 header designed to be injected at the start of the JPEG byte stream.
// Contains a variable sized COM header at the end for cache alignment.
static const uint8_t JPEG_APP0[] = {
    0xFF, 0xE0, // JIFF-APP0
    0x00, 0x10, // 16
    0x4A, 0x46, 0x49, 0x46, 0x00, // JIFF
    0x01, 0x01, // V1.01
    0x01, // DPI
    0x00, 0x00, // Xdensity 0
    0x00, 0x00, // Ydensity 0
    0x00, // Xthumbnail 0
    0x00, // Ythumbnail 0
    0xFF, 0xFE // COM
};

void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbDecodedData)
{
    HAL_JPEG_Pause(hjpeg, JPEG_PAUSE_RESUME_INPUT);
    JPEG_input_paused = true;
    RTE_LOGI("Get data");
}

void HAL_JPEG_DataReadyCallback(JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength)
{
    RTE_LOGI("HAL_JPEG_DataReadyCallback");
    // We have received this much data.
    JPEG_out_data_length += OutDataLength;

    if ((JPEG_out_data_length + OUTPUT_CHUNK_SIZE) > JPEG_out_data_length_max) {
        // We will overflow if we receive anymore data.
        HAL_JPEG_Pause(hjpeg, JPEG_PAUSE_RESUME_OUTPUT);
        JPEG_output_paused = true;
    } else {
        uint8_t *new_pDataOut = pDataOut + OutDataLength;

        // DMA will write data to the output buffer in __SCB_DCACHE_LINE_SIZE aligned chunks. At the
        // end of JPEG compression the processor will manually transfer the remaining parts of the
        // image in randomly aligned chunks. We only want to invalidate the cache of the output
        // buffer for the initial DMA chunks. So, this code below will do that and then only
        // invalidate aligned regions when the processor is moving the final parts of the image.
        if (!(((uint32_t) new_pDataOut) % __SCB_DCACHE_LINE_SIZE)) {
            SCB_InvalidateDCache_by_Addr((uint32_t *) new_pDataOut, OUTPUT_CHUNK_SIZE);
        }

        // We are ok to receive more data.
        HAL_JPEG_ConfigOutputBuffer(hjpeg, new_pDataOut, OUTPUT_CHUNK_SIZE);
    }
}

rte_error_t bsp_jpeg_compress(image_t *src, image_t *dst, int quality)
{
    int mcu_size = 0;
    JPEG_ConfTypeDef JPEG_Info;
    JPEG_Info.ImageWidth    = src->w;
    JPEG_Info.ImageHeight   = src->h;
    JPEG_Info.ImageQuality  = quality;

    switch (src->bpp) {
        case IMAGE_BPP_BINARY:
        case IMAGE_BPP_GRAYSCALE:
            mcu_size                    = JPEG_444_GS_MCU_SIZE;
            JPEG_Info.ColorSpace        = JPEG_GRAYSCALE_COLORSPACE;
            JPEG_Info.ChromaSubsampling = JPEG_444_SUBSAMPLING;
            break;
        case IMAGE_BPP_RGB565:
        case IMAGE_BPP_BAYER:
            mcu_size                    = JPEG_444_YCBCR_MCU_SIZE;
            JPEG_Info.ColorSpace        = JPEG_YCBCR_COLORSPACE;
            JPEG_Info.ChromaSubsampling = JPEG_444_SUBSAMPLING;
            break;
    }

    int src_w_mcus = (src->w + MCU_W - 1) / MCU_W;
    int src_w_mcus_bytes = src_w_mcus * mcu_size;
    int src_w_mcus_bytes_2 = src_w_mcus_bytes * 2;

    // If dst->data == NULL then we need to fb_alloc() space for the payload which will be fb_free()'d
    // by the caller. We have to alloc this memory for all cases if we return from the method.
    if (!dst->data) {
        uint32_t avail = memory_sizeof_max(BANK_JPEG);
        uint32_t space = src_w_mcus_bytes_2 + FB_ALLOC_PADDING;

        if (avail < space) {
            RTE_LOGF("alloc for jpeg buffer failed!");
            return RTE_ERR_NO_MEM;
        }

        dst->bpp = avail - space;
        dst->data = memory_alloc_align(BANK_JPEG, __SCB_DCACHE_LINE_SIZE, dst->bpp);
        RTE_ASSERT(dst->data);
    }

    // Compute size of the APP0 header with cache alignment padding.
    int app0_size = sizeof(JPEG_APP0);
    int app0_unalign_size = app0_size % __SCB_DCACHE_LINE_SIZE;
    int app0_padding_size = app0_unalign_size ? (__SCB_DCACHE_LINE_SIZE - app0_unalign_size) : 0;
    int app0_total_size = app0_size + app0_padding_size;

    if (dst->bpp < app0_total_size) {
        RTE_LOGF("app0_total_size overflow! %d %d", dst->bpp, app0_total_size);
        memory_free(BANK_JPEG, dst->data);
        return RTE_ERR_MISMATCH; // overflow
    }

    if (memcmp(&JPEG_Config, &JPEG_Info, sizeof(JPEG_ConfTypeDef))) {
        HAL_JPEG_ConfigEncoding(&hjpeg, &JPEG_Info);
        memcpy(&JPEG_Config, &JPEG_Info, sizeof(JPEG_ConfTypeDef));
    }


    // Adjust JPEG size and address by app0 header size.
    dst->bpp -= app0_total_size;
    uint8_t *dma_buffer = dst->data + app0_total_size;

    // Destination is too small.
    if (dst->bpp < (OUTPUT_CHUNK_SIZE * 2)) {
        RTE_LOGF("Destination is too small!");
        memory_free(BANK_JPEG, dst->data);
        return RTE_ERR_MISMATCH; // overflow
    }

    JPEG_out_data_length_max = dst->bpp;
    JPEG_out_data_length = 0;
    JPEG_input_paused = false;
    JPEG_output_paused = false;

    uint8_t *mcu_row_buffer = memory_alloc_align(BANK_FB, __SCB_DCACHE_LINE_SIZE, src_w_mcus_bytes_2);
    RTE_ASSERT(mcu_row_buffer);
    for (int y_offset = 0; y_offset < src->h; y_offset += MCU_H) {
        uint8_t *mcu_row_buffer_ptr = mcu_row_buffer + (src_w_mcus_bytes * ((y_offset / MCU_H) % 2));

        int dy = src->h - y_offset;
        if (dy > MCU_H) {
            dy = MCU_H;
        }

        for (int x_offset = 0; x_offset < src->w; x_offset += MCU_W) {
            int8_t *Y0 = (int8_t *) (mcu_row_buffer_ptr + (mcu_size * (x_offset / MCU_W)));
            int8_t *CB = Y0 + JPEG_444_GS_MCU_SIZE;
            int8_t *CR = CB + JPEG_444_GS_MCU_SIZE;

            int dx = src->w - x_offset;
            if (dx > MCU_W) {
                dx = MCU_W;
            }

            // Copy 8x8 MCUs.
            jpeg_get_mcu(src, x_offset, y_offset, dx, dy, Y0, CB, CR);
        }

        // Flush the MCU row for DMA...
        SCB_CleanDCache_by_Addr((uint32_t *) mcu_row_buffer_ptr, src_w_mcus_bytes);

        if (!y_offset) {
            // Invalidate the output buffer.
            SCB_InvalidateDCache_by_Addr(dma_buffer, OUTPUT_CHUNK_SIZE);
            // Start the DMA process off on the first row of MCUs.
            HAL_JPEG_Encode_DMA(&hjpeg, mcu_row_buffer_ptr, src_w_mcus_bytes, dma_buffer, OUTPUT_CHUNK_SIZE);
        } else {

            // Wait for the last row MCUs to be processed before starting the next row.
            while (!JPEG_input_paused) {
                if (JPEG_output_paused) {
                    memset(&JPEG_Config, 0, sizeof(JPEG_ConfTypeDef));
                    HAL_JPEG_Abort(&hjpeg);
                    memory_free(BANK_JPEG, mcu_row_buffer); // mcu_row_buffer (after DMA is aborted)
                    memory_free(BANK_JPEG, dst->data);
                    return RTE_ERR_MISMATCH; // overflow
                }
            }

            // Reset the lock.
            JPEG_input_paused = false;

            // Restart the DMA process on the next row of MCUs (that were already prepared).
            HAL_JPEG_ConfigInputBuffer(&hjpeg, mcu_row_buffer_ptr, src_w_mcus_bytes);
            HAL_JPEG_Resume(&hjpeg, JPEG_PAUSE_RESUME_INPUT);
        }
    }

    // After writing the last MCU to the JPEG core it will eventually generate an end-of-conversion
    // interrupt which will finish the JPEG encoding process and clear the busy flag.

    while (HAL_JPEG_GetState(&hjpeg) == HAL_JPEG_STATE_BUSY_ENCODING) {
        if (JPEG_output_paused) {
            memset(&JPEG_Config, 0, sizeof(JPEG_ConfTypeDef));
            HAL_JPEG_Abort(&hjpeg);
            memory_free(BANK_JPEG, mcu_row_buffer); // mcu_row_buffer (after DMA is aborted)
            memory_free(BANK_JPEG, dst->data);
            return RTE_ERR_MISMATCH; // overflow
        }
    }

    memory_free(BANK_JPEG, mcu_row_buffer); // mcu_row_buffer

    // Set output size.
    dst->bpp = JPEG_out_data_length;

    // STM32H7 BUG FIX! The JPEG Encoder will ocassionally trigger the EOCF interrupt before writing
    // a final 0x000000D9 long into the output fifo as the end of the JPEG image. When this occurs
    // the output fifo will have a single 0 value in it after the encoding process finishes.
    if (__HAL_JPEG_GET_FLAG(&hjpeg, JPEG_FLAG_OFNEF) && (!hjpeg.Instance->DOR)) {
        // The encoding output process always aborts before writing OUTPUT_CHUNK_SIZE bytes
        // to the end of the dma_buffer. So, it is always safe to add one extra byte.
        dma_buffer[dst->bpp] = 0xD9;
        dst->bpp += sizeof(uint8_t);
    }

    // Update the JPEG image size by the new APP0 header and it's padding. However, we have to move
    // the SOI header to the front of the image first...
    dst->bpp += app0_total_size;
    memcpy(dst->data, dma_buffer, sizeof(uint16_t)); // move SOI
    memcpy(dst->data + sizeof(uint16_t), JPEG_APP0, sizeof(JPEG_APP0)); // inject APP0

    // Add on a comment header with 0 padding to ensure cache alignment after the APP0 header.
    *((uint16_t *) (dst->data + sizeof(uint16_t) + sizeof(JPEG_APP0))) = __REV16(app0_padding_size); // size
    memset(dst->data + sizeof(uint32_t) + sizeof(JPEG_APP0), 0, app0_padding_size - sizeof(uint16_t)); // data

    // Clean trailing data after 0xFFD9 at the end of the jpeg byte stream.
    dst->bpp = jpeg_clean_trailing_bytes(dst->bpp, dst->data);
    return RTE_SUCCESS;
}
