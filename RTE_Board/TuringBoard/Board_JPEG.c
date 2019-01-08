#include "Board_JPEG.h"
#define MCU_W                       (8)
#define MCU_H                       (8)
#define JPEG_444_GS_MCU_SIZE        (64)
#define JPEG_444_YCBCR_MCU_SIZE     (192)
#define JPEG_422_YCBCR_MCU_SIZE     (256)
#define JPEG_420_YCBCR_MCU_SIZE     (384)
typedef struct _jpeg_enc {
    int img_w;
    int img_h;
    int img_bpp;
    int mcu_row;
    int mcu_size;
    int out_size;
    int x_offset;
    int y_offset;
    bool overflow;
    image_t *img;
    union {
        uint8_t  *pixels8;
        uint16_t *pixels16;
    };
} jpeg_enc_t;
static uint8_t mcubuf[512];
static jpeg_enc_t jpeg_enc;
static uint8_t *get_mcu()
{
    uint8_t *Y0 = mcubuf;
    uint8_t *CB = mcubuf + 64;
    uint8_t *CR = mcubuf + 128;

    // Copy 8x8 MCUs
    switch (jpeg_enc.img_bpp) {
        case 0:
            for (int y=jpeg_enc.y_offset; y<(jpeg_enc.y_offset + MCU_H); y++) {
                for (int x=jpeg_enc.x_offset; x<(jpeg_enc.x_offset + MCU_W); x++) {
                    *Y0++ = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL(jpeg_enc.img, x, y)) - 128;
                }
            }
            break;
        case 1:
            for (int y=jpeg_enc.y_offset; y<(jpeg_enc.y_offset + MCU_H); y++) {
                for (int x=jpeg_enc.x_offset; x<(jpeg_enc.x_offset + MCU_W); x++) {
                    *Y0++ = jpeg_enc.pixels8[y * jpeg_enc.img_w + x];
                }
            }
            break;
        case 2: {
            for (int y=jpeg_enc.y_offset, idx=0; y<(jpeg_enc.y_offset + MCU_H); y++) {
                for (int x=jpeg_enc.x_offset; x<(jpeg_enc.x_offset + MCU_W); x++, idx++) {
                    int ofs = y * jpeg_enc.img_w + x;
                    Y0[idx] = yuv_table[jpeg_enc.pixels16[ofs] * 3 + 0] - 128;
                    CB[idx] = yuv_table[jpeg_enc.pixels16[ofs] * 3 + 1] - 128;
                    CR[idx] = yuv_table[jpeg_enc.pixels16[ofs] * 3 + 2] - 128;
                }
            }
            break;
        }
        case 3: {
            uint16_t rgbbuf[64];
            imlib_bayer_to_rgb565(jpeg_enc.img, 8, 8, jpeg_enc.x_offset, jpeg_enc.y_offset, rgbbuf); 
            for (int y=0, idx=0; y<8; y++) {
                for (int x=0; x<8; x++, idx++) {
                    Y0[idx] = yuv_table[rgbbuf[idx] * 3 + 0] - 128;
                    CB[idx] = yuv_table[rgbbuf[idx] * 3 + 1] - 128;
                    CR[idx] = yuv_table[rgbbuf[idx] * 3 + 2] - 128;
                }
            }
            break;
        }
    }

    jpeg_enc.x_offset += MCU_W;
    if (jpeg_enc.x_offset == (jpeg_enc.mcu_row * MCU_W)) {
        jpeg_enc.x_offset = 0;
        jpeg_enc.y_offset += MCU_H;
    }
    return mcubuf;
}
void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbDecodedData)
{
    HAL_JPEG_Pause(hjpeg, JPEG_PAUSE_RESUME_INPUT);
    if ((hjpeg->JpegOutCount+1024) > hjpeg->OutDataLength) {
        jpeg_enc.overflow = true;
        HAL_JPEG_Abort(hjpeg);
        HAL_JPEG_ConfigInputBuffer(hjpeg, NULL, 0);
    } else if (jpeg_enc.y_offset == jpeg_enc.img_h) {
        HAL_JPEG_ConfigInputBuffer(hjpeg, NULL, 0);
    } else {
        HAL_JPEG_ConfigInputBuffer(hjpeg, get_mcu(), jpeg_enc.mcu_size);
    }
    HAL_JPEG_Resume(hjpeg, JPEG_PAUSE_RESUME_INPUT);
}

void HAL_JPEG_DataReadyCallback (JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength)
{
    jpeg_enc.out_size = OutDataLength;
}

void HAL_JPEG_ErrorCallback(JPEG_HandleTypeDef *hjpeg)
{
    RTE_Printf("JPEG decode/encode error\n");
}
void Board_JPEG_Init(void)
{ 
  /* Init The JPEG Color Look Up Tables used for YCbCr to RGB conversion   */ 
//  JPEG_InitColorTables();
}
bool Board_JPEG_Encode(image_t *src, image_t *dst, int quality)
{
	JPEG_HandleTypeDef BoardJPEG = {0};
  BoardJPEG.Instance = JPEG;
  HAL_JPEG_Init(&BoardJPEG);  
	jpeg_enc.img      = src;
	jpeg_enc.img_w    = src->w;
	jpeg_enc.img_h    = src->h;
	jpeg_enc.img_bpp  = src->bpp;
	jpeg_enc.mcu_row  = src->w / MCU_W;
	jpeg_enc.out_size = 0;
	jpeg_enc.x_offset = 0;
	jpeg_enc.y_offset = 0;
	jpeg_enc.overflow = false;
	jpeg_enc.pixels8  = (uint8_t *) src->pixels;
	jpeg_enc.pixels16 = (uint16_t*) src->pixels;
	JPEG_ConfTypeDef JPEG_Info;
	JPEG_Info.ImageWidth    = src->w;
	JPEG_Info.ImageHeight   = src->h;
	JPEG_Info.ImageQuality  = quality;
	switch (src->bpp) {
		case 0:
		case 1:
		{
      jpeg_enc.mcu_size           = JPEG_444_GS_MCU_SIZE;
			JPEG_Info.ColorSpace        = JPEG_GRAYSCALE_COLORSPACE;
			JPEG_Info.ChromaSubsampling = JPEG_444_SUBSAMPLING;
		}break;
		case 2:
		case 3:
		{
			jpeg_enc.mcu_size           = JPEG_444_YCBCR_MCU_SIZE;
			JPEG_Info.ColorSpace        = JPEG_YCBCR_COLORSPACE;
			JPEG_Info.ChromaSubsampling = JPEG_444_SUBSAMPLING;
		}break;
	}
  /* Fill Encoding Params */
  HAL_JPEG_ConfigEncoding(&BoardJPEG, &JPEG_Info);
  /* Start JPEG encoding with DMA method */
	if (HAL_JPEG_Encode(&BoardJPEG, get_mcu(), jpeg_enc.mcu_size, dst->pixels, dst->bpp, 100) != HAL_OK) {
		return true;
	}
	// Set output size
	dst->bpp = jpeg_enc.out_size;
	HAL_JPEG_DeInit(&BoardJPEG);
	return jpeg_enc.overflow;
}
void HAL_JPEG_MspInit(JPEG_HandleTypeDef* hjpeg)
{
  if(hjpeg->Instance==JPEG)
  {
    __HAL_RCC_JPEG_CLK_ENABLE();
  }

}
void HAL_JPEG_MspDeInit(JPEG_HandleTypeDef* hjpeg)
{
  if(hjpeg->Instance==JPEG)
  {
    __HAL_RCC_JPEG_CLK_DISABLE();
  }
}
