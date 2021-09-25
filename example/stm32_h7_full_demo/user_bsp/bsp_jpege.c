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
#define FB_ALLOC_PADDING            ((__SCB_DCACHE_LINE_SIZE) * 4)
#define OUTPUT_CHUNK_SIZE           (512) // The minimum output buffer size is 2x this - so 1KB.
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

static rte_error_t jpeg_get_mcu(image_t *src,
                                uint32_t x_offset, uint32_t y_offset,
                                uint32_t dx, uint32_t dy,
                                int8_t *Y0, int8_t *CB, int8_t *CR)
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
            return RTE_SUCCESS;
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
            return RTE_SUCCESS;
        }
        case IMAGE_BPP_RGB565:
        case IMAGE_BPP_BAYER: {
            break;
        }
    }
    return RTE_ERR_UNDEFINE;
}

typedef struct {
    uint8_t *buffer;
    image_t *img;
    uint32_t img_row_count;
    uint32_t input_chunk_size;
    uint32_t buffer_size;
} input_mcu_t;

typedef struct {
    uint8_t *buffer;
    image_t *img;
    uint32_t real_size;
    uint32_t buffer_size;
    bool if_error;
    osSemaphoreId_t encode_over_sema;
} output_img_t;

static input_mcu_t input_mcu = {0};
static output_img_t output_img = {0};

static void refresh_input_mcu_buffer(void)
{
    uint32_t dy = input_mcu.img->h - input_mcu.img_row_count;
    if (dy > MCU_H) {
        dy = MCU_H;
    }

    for (uint32_t x_offset = 0; x_offset < input_mcu.img->w; x_offset += MCU_W) {
        int8_t *Y0 = (int8_t *) (input_mcu.buffer + (input_mcu.input_chunk_size * (x_offset / MCU_W)));
        int8_t *CB = Y0 + input_mcu.input_chunk_size;
        int8_t *CR = CB + input_mcu.input_chunk_size;

        uint32_t dx = input_mcu.img->w - x_offset;
        if (dx > MCU_W) {
            dx = MCU_W;
        }
        // Copy 8x8 MCUs.
        jpeg_get_mcu(input_mcu.img, x_offset, input_mcu.img_row_count, dx, dy, Y0, CB, CR);
    }
}

void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbDecodedData)
{
    if (NbDecodedData == input_mcu.buffer_size) {
        // Restart the DMA process on the next row of MCUs (that were already prepared).
        if (input_mcu.img_row_count < input_mcu.img->h) {
            input_mcu.img_row_count += MCU_H;
            refresh_input_mcu_buffer();
            HAL_RAM_CLEAN_PRE_SEND((uint32_t *)input_mcu.buffer, input_mcu.buffer_size);
            HAL_JPEG_ConfigInputBuffer(hjpeg, input_mcu.buffer, input_mcu.buffer_size);
        }
    } else {
        HAL_JPEG_ConfigInputBuffer(hjpeg, input_mcu.buffer + NbDecodedData, input_mcu.buffer_size - NbDecodedData);
    }
}

void HAL_JPEG_DataReadyCallback(JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength)
{
    // We have received this much data.
    output_img.real_size += OutDataLength;
    if ((output_img.real_size + OUTPUT_CHUNK_SIZE) > output_img.buffer_size) {
        // We abort the transfer early when we find overflow for receiving anymore data.
        output_img.if_error = true;
        osSemaphoreRelease(output_img.encode_over_sema);
    } else {
        uint8_t *new_output_data = pDataOut + OutDataLength;
        // We are ok to receive more data.
        HAL_JPEG_ConfigOutputBuffer(hjpeg, new_output_data, OUTPUT_CHUNK_SIZE);
    }
}

void HAL_JPEG_EncodeCpltCallback(JPEG_HandleTypeDef *hjpeg)
{
    // Set output size.
    output_img.img->bpp = output_img.real_size;
    // STM32H7 BUG FIX! The JPEG Encoder will occasionally trigger the EOCF interrupt before writing
    // a final 0x000000D9 long into the output fifo as the end of the JPEG image. When this occurs
    // the output fifo will have a single 0 value in it after the encoding process finishes.
    if (__HAL_JPEG_GET_FLAG(hjpeg, JPEG_FLAG_OFNEF) && (!hjpeg->Instance->DOR)) {
        // The encoding output process always aborts before writing OUTPUT_CHUNK_SIZE bytes
        // to the end of the output_img.buffer. So, it is always safe to add one extra byte.
        output_img.buffer[output_img.img->bpp] = 0xD9;
        output_img.img->bpp += sizeof(uint8_t);
    }
    osSemaphoreRelease(output_img.encode_over_sema);
}

void bsp_jpeg_init(void)
{
    output_img.encode_over_sema = osSemaphoreNew(8, 0U, NULL);
}

rte_error_t bsp_jpeg_compress(image_t *src, image_t *dst, int quality)
{
    uint32_t mcu_size = 0;
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

    uint32_t src_w_mcus = (src->w + MCU_W - 1) / MCU_W;
    uint32_t src_w_mcus_bytes = src_w_mcus * mcu_size;

    // If dst->data == NULL then we need to fb_alloc() space for the payload which will be fb_free()'d
    // by the caller. We have to alloc this memory for all cases if we return from the method.
    if (dst->data) {
        memory_free(BANK_JPEG, dst->data);
        dst->data = NULL;
    }

    uint32_t avail = memory_sizeof_max(BANK_JPEG);
    dst->bpp = avail - (__SCB_DCACHE_LINE_SIZE << 8);
    dst->data = memory_alloc_align(BANK_JPEG, __SCB_DCACHE_LINE_SIZE, dst->bpp);
    RTE_ASSERT(dst->data);
    HAL_JPEG_ConfigEncoding(&hjpeg, &JPEG_Info);

    // Init the output and input handle.
    output_img.img = dst;
    output_img.buffer_size = dst->bpp;
    output_img.real_size = 0;
    output_img.buffer = dst->data;
    output_img.if_error = false;

    input_mcu.buffer = memory_alloc_align(BANK_FB, __SCB_DCACHE_LINE_SIZE, src_w_mcus_bytes);
    RTE_ASSERT(input_mcu.buffer);
    input_mcu.img = src;
    input_mcu.input_chunk_size = mcu_size;
    input_mcu.buffer_size = src_w_mcus_bytes;
    input_mcu.img_row_count = 0;
    refresh_input_mcu_buffer();
    HAL_RAM_CLEAN_PRE_SEND((uint32_t *)input_mcu.buffer, input_mcu.buffer_size);
    // Start the DMA process off on the first row of MCUs.
    HAL_JPEG_Encode_DMA(&hjpeg, input_mcu.buffer, input_mcu.buffer_size, output_img.buffer, OUTPUT_CHUNK_SIZE);
    // Wait encode over.
    osSemaphoreAcquire(output_img.encode_over_sema, osWaitForever);
    // Check if error happens.
    if (output_img.if_error == true) {
        HAL_JPEG_Abort(&hjpeg);
        memory_free(BANK_FB, input_mcu.buffer); // mcu_row_buffer (after DMA is aborted)
        memory_free(BANK_JPEG, dst->data);
        return RTE_ERR_MISMATCH; // overflow
    }
    memory_free(BANK_FB, input_mcu.buffer); // mcu_row_buffer

    // Clean trailing data after 0xFFD9 at the end of the jpeg byte stream.
    dst->bpp = jpeg_clean_trailing_bytes(dst->bpp, dst->data);
    return RTE_SUCCESS;
}
