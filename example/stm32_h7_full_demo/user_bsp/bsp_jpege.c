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

static void refresh_mcu_raw_buffer(void)
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
    HAL_JPEG_Pause(hjpeg, JPEG_PAUSE_RESUME_INPUT);
    // Restart the DMA process on the next row of MCUs (that were already prepared).
    if (input_mcu.img_row_count < input_mcu.img->h) {
        input_mcu.img_row_count += MCU_H;
        refresh_mcu_raw_buffer();
        HAL_RAM_CLEAN_PRE_SEND((uint32_t *)input_mcu.buffer, input_mcu.buffer_size);
        HAL_JPEG_ConfigInputBuffer(hjpeg, input_mcu.buffer, input_mcu.buffer_size);
        HAL_JPEG_Resume(hjpeg, JPEG_PAUSE_RESUME_INPUT);
    }
}

void HAL_JPEG_DataReadyCallback(JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength)
{
    HAL_JPEG_Pause(hjpeg, JPEG_PAUSE_RESUME_OUTPUT);
    // We have received this much data.
    output_img.real_size += OutDataLength;
    if ((output_img.real_size + OUTPUT_CHUNK_SIZE) > output_img.buffer_size) {
        // We abort the transfer early when we find overflow for receiving anymore data.
        ATOMIC_SET(&output_img.if_error, true);
        osSemaphoreRelease(output_img.encode_over_sema);
    } else {
        // Check if the transferred chunk is the last chunk
        if (OutDataLength == OUTPUT_CHUNK_SIZE) {
            uint8_t *new_pDataOut = pDataOut + OutDataLength;
            // DMA will write data to the output buffer in __SCB_DCACHE_LINE_SIZE aligned chunks. At the
            // end of JPEG compression the processor will manually transfer the remaining parts of the
            // image in randomly aligned chunks. We only want to invalidate the cache of the output
            // buffer for the initial DMA chunks. So, this code below will do that and then only
            // invalidate aligned regions when the processor is moving the final parts of the image.
            HAL_RAM_CLEAN_AFTER_REC((uint32_t *) new_pDataOut, OUTPUT_CHUNK_SIZE);
            // We are ok to receive more data.
            HAL_JPEG_ConfigOutputBuffer(hjpeg, new_pDataOut, OUTPUT_CHUNK_SIZE);
            HAL_JPEG_Resume(hjpeg, JPEG_PAUSE_RESUME_OUTPUT);
        }
    }
}

void HAL_JPEG_EncodeCpltCallback(JPEG_HandleTypeDef *hjpeg)
{
    // Set output size.
    output_img.img->bpp = output_img.real_size;
    // STM32H7 BUG FIX! The JPEG Encoder will ocassionally trigger the EOCF interrupt before writing
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

    int src_w_mcus = (src->w + MCU_W - 1) / MCU_W;
    int src_w_mcus_bytes = src_w_mcus * mcu_size;
    int src_w_mcus_bytes_2 = src_w_mcus_bytes * 2;
    int src_h_mcus = (src->h + MCU_H - 1) / MCU_H;

    // If dst->data == NULL then we need to fb_alloc() space for the payload which will be fb_free()'d
    // by the caller. We have to alloc this memory for all cases if we return from the method.
    if (dst->data) {
        memory_free(BANK_JPEG, dst->data);
        dst->data = NULL;
    }

    uint32_t avail = memory_sizeof_max(BANK_JPEG);
    uint32_t space = src_w_mcus_bytes_2 + FB_ALLOC_PADDING;

    if (avail < space) {
        RTE_LOGF("alloc for jpeg buffer failed!");
        return RTE_ERR_NO_MEM;
    }

    dst->bpp = avail - space;
    dst->data = memory_alloc_align(BANK_JPEG, __SCB_DCACHE_LINE_SIZE, dst->bpp);
    RTE_ASSERT(dst->data);

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

    HAL_JPEG_ConfigEncoding(&hjpeg, &JPEG_Info);

    // Adjust JPEG size and address by app0 header size.
    dst->bpp -= app0_total_size;

    // Init the output and input handle.
    output_img.img = dst;
    output_img.buffer_size = dst->bpp;
    output_img.real_size = 0;
    output_img.buffer = dst->data + app0_total_size;
    ATOMIC_SET(&output_img.if_error, false);

    input_mcu.buffer = memory_alloc_align(BANK_FB, __SCB_DCACHE_LINE_SIZE, src_w_mcus_bytes_2);
    RTE_ASSERT(input_mcu.buffer);
    input_mcu.img = src;
    input_mcu.input_chunk_size = mcu_size;
    input_mcu.buffer_size = src_w_mcus_bytes;
    input_mcu.img_row_count = 0;
    refresh_mcu_raw_buffer();
    // Start the DMA process off on the first row of MCUs.
    HAL_JPEG_Encode_IT(&hjpeg, input_mcu.buffer, input_mcu.buffer_size, output_img.buffer, OUTPUT_CHUNK_SIZE);
    // After writing the last MCU to the JPEG core it will eventually generate an end-of-conversion
    // interrupt which will finish the JPEG encoding process and clear the busy flag.

    osSemaphoreAcquire(output_img.encode_over_sema, osWaitForever);
    if (ATOMIC_READ(&output_img.if_error) == true) {
        HAL_JPEG_Abort(&hjpeg);
        memory_free(BANK_FB, input_mcu.buffer); // mcu_row_buffer (after DMA is aborted)
        memory_free(BANK_JPEG, dst->data);
        return RTE_ERR_MISMATCH; // overflow
    }
    memory_free(BANK_FB, input_mcu.buffer); // mcu_row_buffer

    // Update the JPEG image size by the new APP0 header and it's padding. However, we have to move
    // the SOI header to the front of the image first...
    dst->bpp += app0_total_size;
    memcpy(dst->data, output_img.buffer, sizeof(uint16_t)); // move SOI
    memcpy(dst->data + sizeof(uint16_t), JPEG_APP0, sizeof(JPEG_APP0)); // inject APP0

    // Add on a comment header with 0 padding to ensure cache alignment after the APP0 header.
    *((uint16_t *) (dst->data + sizeof(uint16_t) + sizeof(JPEG_APP0))) = __REV16(app0_padding_size); // size
    memset(dst->data + sizeof(uint32_t) + sizeof(JPEG_APP0), 0, app0_padding_size - sizeof(uint16_t)); // data

    // Clean trailing data after 0xFFD9 at the end of the jpeg byte stream.
    dst->bpp = jpeg_clean_trailing_bytes(dst->bpp, dst->data);
    return RTE_SUCCESS;
}
