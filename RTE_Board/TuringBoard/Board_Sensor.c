#include "Board_Sensor.h"
#include "Board_OV7725.h"
#include "Board_SCCB.h"
#include "Board_DCMI.h"
sensor_t sensor          = {0};
#define OV_CHIP_ID      (0x0A)
#define ON_CHIP_ID      (0x00)
#define MAX_XFER_SIZE (0xFFFC)
const int resolution[][2] = {
    {0,    0   },
    // C/SIF Resolutions
    {88,   72  },    /* QQCIF     */
    {176,  144 },    /* QCIF      */
    {352,  288 },    /* CIF       */
    {88,   60  },    /* QQSIF     */
    {176,  120 },    /* QSIF      */
    {352,  240 },    /* SIF       */
    // VGA Resolutions
    {40,   30  },    /* QQQQVGA   */
    {80,   60  },    /* QQQVGA    */
    {160,  120 },    /* QQVGA     */
    {320,  240 },    /* QVGA      */
    {640,  480 },    /* VGA       */
    {60,   40  },    /* HQQQVGA   */
    {120,  80  },    /* HQQVGA    */
    {240,  160 },    /* HQVGA     */
    // FFT Resolutions
    {64,   32  },    /* 64x32     */
    {64,   64  },    /* 64x64     */
    {128,  64  },    /* 128x64    */
    {128,  128 },    /* 128x64    */
    // Other
    {128,  160 },    /* LCD       */
    {128,  160 },    /* QQVGA2    */
    {800,  600 },    /* SVGA      */
    {1280, 1024},    /* SXGA      */
    {1600, 1200},    /* UXGA      */
};
void sensor_gpio_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  /* GPIO Ports Clock Enable */
	CAM_RST_GPIO_CLK_ENABLE();
	CAM_STBY_GPIO_CLK_ENABLE();
  /*Configure GPIO pins : PCPin PCPin PCPin */
  GPIO_InitStruct.Pin = CAM_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CAM_RST_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = CAM_STBY_Pin;
  HAL_GPIO_Init(CAM_STBY_GPIO_Port, &GPIO_InitStruct);
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CAM_RST_GPIO_Port, CAM_RST_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(CAM_STBY_GPIO_Port, CAM_STBY_Pin, GPIO_PIN_RESET);
}

int sensor_init(void)
{
		sensor_gpio_init();
    int init_ret = 0;
    /* Do a power cycle */
		HAL_GPIO_WritePin(CAM_STBY_GPIO_Port,CAM_STBY_Pin,GPIO_PIN_SET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(CAM_STBY_GPIO_Port,CAM_STBY_Pin,GPIO_PIN_RESET);
		HAL_Delay(100);
    /* Reset the sesnor state */
    memset(&sensor, 0, sizeof(sensor_t));
    /* Some sensors have different reset polarities, and we can't know which sensor
       is connected before initializing cambus and probing the sensor, which in turn
       requires pulling the sensor out of the reset state. So we try to probe the
       sensor with both polarities to determine line state. */
    sensor.pwdn_pol = ACTIVE_HIGH;
    sensor.reset_pol = ACTIVE_HIGH;
    /* Reset the sensor */
		HAL_GPIO_WritePin(CAM_RST_GPIO_Port,CAM_RST_Pin,GPIO_PIN_SET);
		HAL_Delay(10);
		HAL_GPIO_WritePin(CAM_RST_GPIO_Port,CAM_RST_Pin,GPIO_PIN_RESET);
		HAL_Delay(10);
    /* Probe the sensor */
    sensor.slv_addr = Board_SCCB_Probe();
    if (sensor.slv_addr == 0) {
        /* Sensor has been held in reset,
           so the reset line is active low */
        sensor.reset_pol = ACTIVE_LOW;
        /* Pull the sensor out of the reset state */
				HAL_GPIO_WritePin(CAM_RST_GPIO_Port,CAM_RST_Pin,GPIO_PIN_SET);
				HAL_Delay(100);
        /* Probe again to set the slave addr */
        sensor.slv_addr = Board_SCCB_Probe();
			
        if (sensor.slv_addr == 0) {
            sensor.pwdn_pol = ACTIVE_LOW;
						HAL_GPIO_WritePin(CAM_STBY_GPIO_Port,CAM_STBY_Pin,GPIO_PIN_SET);
						HAL_Delay(100);
            sensor.slv_addr = Board_SCCB_Probe();
            if (sensor.slv_addr == 0) {
                sensor.reset_pol = ACTIVE_HIGH;
								/* Pull the sensor out of the reset state */
								HAL_GPIO_WritePin(CAM_RST_GPIO_Port,CAM_RST_Pin,GPIO_PIN_RESET);
								HAL_Delay(100);
                sensor.slv_addr = Board_SCCB_Probe();
                if (sensor.slv_addr == 0) {
                    return -2;
                }
            }
        }
    }
		RTE_Printf("Sensor Add:%x\r\n",sensor.slv_addr);
    // Clear sensor chip ID.
    sensor.chip_id = 0;
    // Set default snapshot function.
    sensor.snapshot = sensor_snapshot;
    if (sensor.slv_addr == LEPTON_ID) {
        sensor.chip_id = LEPTON_ID;
//        extclk_config(25000000);
//        init_ret = lepton_init(&sensor);
    } else {
        // Read ON semi sensor ID.
        Board_SCCB_Read(sensor.slv_addr, ON_CHIP_ID, &sensor.chip_id);
        if (sensor.chip_id == MT9V034_ID) {
//            // On/Aptina MT requires 13-27MHz clock.
//            extclk_config(27000000);
//            // Only the MT9V034 is currently supported.
//            init_ret = mt9v034_init(&sensor);
        } else { // Read OV sensor ID.
            Board_SCCB_Read(sensor.slv_addr, OV_CHIP_ID, &sensor.chip_id);
            // Initialize sensor struct.
            switch (sensor.chip_id) {
                case OV9650_ID:
//                    init_ret = ov9650_init(&sensor);
                    break;
                case OV2640_ID:
//                    init_ret = ov2640_init(&sensor);
                    break;
                case OV7725_ID:
                    init_ret = ov7725_init(&sensor);
                    break;
                default:
                    // Sensor is not supported.
                    return -3;
            }
        }
    }

    if (init_ret != 0 ) {
        // Sensor init failed.
        return -4;
    }
    // Disable VSYNC EXTI IRQ
    HAL_NVIC_DisableIRQ(DCMI_VSYNC_IRQN);

    /* All good! */
    return 0;
}

int sensor_reset()
{
    // Reset the sesnor state
    sensor.sde         = (sde_t)0;
    sensor.pixformat   = (pixformat_t)0;
    sensor.framesize   = (framesize_t)0;
    sensor.framerate   = (framerate_t)0;
    sensor.gainceiling = (gainceiling_t)0;
    sensor.vsync_gpio  = NULL;

    // Call sensor-specific reset function
    if (sensor.reset(&sensor) != 0) {
        return -1;
    }

    // Just in case there's a running DMA request.
    HAL_DMA_Abort(&BoardDcmiDma);

    // Disable VSYNC EXTI IRQ
    HAL_NVIC_DisableIRQ(DCMI_VSYNC_IRQN);
		
    return 0;
}

int sensor_get_id()
{
    return sensor.chip_id;
}

int sensor_sleep(int enable)
{
    if (sensor.sleep == NULL
        || sensor.sleep(&sensor, enable) != 0) {
        // Operation not supported
        return -1;
    }
    return 0;
}

int sensor_read_reg(uint8_t reg_addr)
{
    if (sensor.read_reg == NULL) {
        // Operation not supported
        return -1;
    }
    return sensor.read_reg(&sensor, reg_addr);
}

int sensor_write_reg(uint8_t reg_addr, uint16_t reg_data)
{
    if (sensor.write_reg == NULL) {
        // Operation not supported
        return -1;
    }
    return sensor.write_reg(&sensor, reg_addr, reg_data);
}

int sensor_set_pixformat(pixformat_t pixformat)
{
    uint32_t jpeg_mode = DCMI_JPEG_DISABLE;

    if (sensor.pixformat == pixformat) {
        // No change
        return 0;
    }

    if (sensor.set_pixformat == NULL
        || sensor.set_pixformat(&sensor, pixformat) != 0) {
        // Operation not supported
        return -1;
    }

    // Set pixel format
    sensor.pixformat = pixformat;

    // Set JPEG mode
    if (pixformat == PIXFORMAT_JPEG) {
        jpeg_mode = DCMI_JPEG_ENABLE;
				BoardDcmi.Init.JPEGMode = jpeg_mode;
				if (HAL_DCMI_Init(&BoardDcmi) != HAL_OK)
				{
					RTE_Assert(__FILE__, __LINE__);
				}
    }

    // Skip the first frame.
    MAIN_FB()->bpp = 0;

    return 0;
}

int sensor_set_framesize(framesize_t framesize)
{
    if (sensor.framesize == framesize) {
        // No change
        return 0;
    }

    // Call the sensor specific function
    if (sensor.set_framesize == NULL
        || sensor.set_framesize(&sensor, framesize) != 0) {
        // Operation not supported
        return -1;
    }

    // Set framebuffer size
    sensor.framesize = framesize;

    // Skip the first frame.
    MAIN_FB()->bpp = 0;

    // Set MAIN FB x, y offset.
    MAIN_FB()->x = 0;
    MAIN_FB()->y = 0;

    // Set MAIN FB width and height.
    MAIN_FB()->w = resolution[framesize][0];
    MAIN_FB()->h = resolution[framesize][1];

    // Set MAIN FB backup width and height.
    MAIN_FB()->u = resolution[framesize][0];
    MAIN_FB()->v = resolution[framesize][1];
    return 0;
}

int sensor_set_framerate(framerate_t framerate)
{
    if (sensor.framerate == framerate) {
       /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_framerate == NULL
        || sensor.set_framerate(&sensor, framerate) != 0) {
        /* operation not supported */
        return -1;
    }

    /* set the frame rate */
    sensor.framerate = framerate;

    return 0;
}

int sensor_set_windowing(int x, int y, int w, int h)
{
    MAIN_FB()->x = x;
    MAIN_FB()->y = y;
    MAIN_FB()->w = MAIN_FB()->u = w;
    MAIN_FB()->h = MAIN_FB()->v = h;
    return 0;
}

int sensor_set_contrast(int level)
{
    if (sensor.set_contrast != NULL) {
        return sensor.set_contrast(&sensor, level);
    }
    return -1;
}

int sensor_set_brightness(int level)
{
    if (sensor.set_brightness != NULL) {
        return sensor.set_brightness(&sensor, level);
    }
    return -1;
}

int sensor_set_saturation(int level)
{
    if (sensor.set_saturation != NULL) {
        return sensor.set_saturation(&sensor, level);
    }
    return -1;
}

int sensor_set_gainceiling(gainceiling_t gainceiling)
{
    if (sensor.gainceiling == gainceiling) {
        /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_gainceiling == NULL
        || sensor.set_gainceiling(&sensor, gainceiling) != 0) {
        /* operation not supported */
        return -1;
    }

    sensor.gainceiling = gainceiling;
    return 0;
}

int sensor_set_quality(int qs)
{
    /* call the sensor specific function */
    if (sensor.set_quality == NULL
        || sensor.set_quality(&sensor, qs) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_colorbar(int enable)
{
    /* call the sensor specific function */
    if (sensor.set_colorbar == NULL
        || sensor.set_colorbar(&sensor, enable) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_auto_gain(int enable, float gain_db, float gain_db_ceiling)
{
    /* call the sensor specific function */
    if (sensor.set_auto_gain == NULL
        || sensor.set_auto_gain(&sensor, enable, gain_db, gain_db_ceiling) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_get_gain_db(float *gain_db)
{
    /* call the sensor specific function */
    if (sensor.get_gain_db == NULL
        || sensor.get_gain_db(&sensor, gain_db) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_auto_exposure(int enable, int exposure_us)
{
    /* call the sensor specific function */
    if (sensor.set_auto_exposure == NULL
        || sensor.set_auto_exposure(&sensor, enable, exposure_us) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_get_exposure_us(int *exposure_us)
{
    /* call the sensor specific function */
    if (sensor.get_exposure_us == NULL
        || sensor.get_exposure_us(&sensor, exposure_us) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_auto_whitebal(int enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
    /* call the sensor specific function */
    if (sensor.set_auto_whitebal == NULL
        || sensor.set_auto_whitebal(&sensor, enable, r_gain_db, g_gain_db, b_gain_db) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_get_rgb_gain_db(float *r_gain_db, float *g_gain_db, float *b_gain_db)
{
    /* call the sensor specific function */
    if (sensor.get_rgb_gain_db == NULL
        || sensor.get_rgb_gain_db(&sensor, r_gain_db, g_gain_db, b_gain_db) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_hmirror(int enable)
{
    /* call the sensor specific function */
    if (sensor.set_hmirror == NULL
        || sensor.set_hmirror(&sensor, enable) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_vflip(int enable)
{
    /* call the sensor specific function */
    if (sensor.set_vflip == NULL
        || sensor.set_vflip(&sensor, enable) != 0) {
        /* operation not supported */
        return -1;
    }
    return 0;
}

int sensor_set_special_effect(sde_t sde)
{
    if (sensor.sde == sde) {
        /* no change */
        return 0;
    }

    /* call the sensor specific function */
    if (sensor.set_special_effect == NULL
        || sensor.set_special_effect(&sensor, sde) != 0) {
        /* operation not supported */
        return -1;
    }

    sensor.sde = sde;
    return 0;
}

int sensor_set_lens_correction(int enable, int radi, int coef)
{
    /* call the sensor specific function */
    if (sensor.set_lens_correction == NULL
        || sensor.set_lens_correction(&sensor, enable, radi, coef) != 0) {
        /* operation not supported */
        return -1;
    }

    return 0;
}

int sensor_set_vsync_output(GPIO_TypeDef *gpio, uint32_t pin)
{
    sensor.vsync_pin  = pin;
    sensor.vsync_gpio = gpio;
    // Enable VSYNC EXTI IRQ
    HAL_NVIC_SetPriority(DCMI_VSYNC_IRQN, 0, 0);
    HAL_NVIC_EnableIRQ(DCMI_VSYNC_IRQN);
    return 0;
}

static void sensor_check_buffsize()
{
    int bpp=0;
    switch (sensor.pixformat) {
        case PIXFORMAT_BAYER:
        case PIXFORMAT_GRAYSCALE:
            bpp = 1;
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_RGB565:
            bpp = 2;
            break;
        default:
            break;
    }
		
    if ((MAIN_FB()->w * MAIN_FB()->h * bpp) > RTE_MEM_GetDataSize(MAIN_FB()->pixels)) {
        if (sensor.pixformat == PIXFORMAT_GRAYSCALE) {
            // Crop higher GS resolutions to QVGA
            sensor_set_windowing(190, 120, 320, 240);
        } else if (sensor.pixformat == PIXFORMAT_RGB565) {
            // Switch to BAYER if the frame is too big to fit in RAM.
            sensor_set_pixformat(PIXFORMAT_BAYER);
        }
    }

}
void DCMI_VsyncExtiCallback()
{
    __HAL_GPIO_EXTI_CLEAR_FLAG(1 << DCMI_VSYNC_IRQ_LINE);
    if (sensor.vsync_gpio != NULL) {
        HAL_GPIO_WritePin(sensor.vsync_gpio, sensor.vsync_pin,
                !HAL_GPIO_ReadPin(DCMI_VSYNC_PORT, DCMI_VSYNC_PIN));
    }
}
// This is the default snapshot function, which can be replaced in sensor_init functions. This function
// uses the DCMI and DMA to capture frames and each line is processed in the DCMI_DMAConvCpltUser function.
int sensor_snapshot(sensor_t *sensor)
{
    uint32_t addr, length;
    // Compress the framebuffer for the IDE preview, only if it's not the first frame,
    // the framebuffer is enabled and the image sensor does not support JPEG encoding.
    // Note: This doesn't run unless the IDE is connected and the framebuffer is enabled.
    //fb_update_jpeg_buffer();
    // Make sure the raw frame fits into the FB. If it doesn't it will be cropped if
    // the format is set to GS, otherwise the pixel format will be swicthed to BAYER.
    sensor_check_buffsize();

    // The user may have changed the MAIN_FB width or height on the last image so we need
    // to restore that here. We don't have to restore bpp because that's taken care of
    // already in the code below. Note that we do the JPEG compression above first to save
    // the FB of whatever the user set it to and now we restore.
    MAIN_FB()->w = MAIN_FB()->u;
    MAIN_FB()->h = MAIN_FB()->v;

    // We use the stored frame size to read the whole frame. Note that cropping is
    // done in the line function using the diemensions stored in MAIN_FB()->x,y,w,h.
    uint32_t w = resolution[sensor->framesize][0];
    uint32_t h = resolution[sensor->framesize][1];

    // Setup the size and address of the transfer
    switch (sensor->pixformat) {
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            // RGB/YUV read 2 bytes per pixel.
            length = (w * h * 2)/4;
            addr = (uint32_t) (MAIN_FB()->pixels);
            break;
        case PIXFORMAT_BAYER:
            // BAYER/RAW: 1 byte per pixel
            length = (w * h * 1)/4;
            addr = (uint32_t) (MAIN_FB()->pixels);
            break;
        case PIXFORMAT_GRAYSCALE:
            // 1/2BPP Grayscale.
            length = (w * h * sensor->gs_bpp)/4;
            addr = (uint32_t) (MAIN_FB()->pixels);
            break;
        case PIXFORMAT_JPEG:
            // Sensor has hardware JPEG set max frame size.
            length = MAX_XFER_SIZE;
            addr = (uint32_t) (MAIN_FB()->pixels);
            break;
        default:
            return -1;
    }

    // Enable DMA IRQ
    HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
		__HAL_DCMI_ENABLE_IT(&BoardDcmi,DCMI_IT_FRAME);
		// Start a regular transfer
		HAL_DCMI_Start_DMA(&BoardDcmi,
						DCMI_MODE_SNAPSHOT, addr, length);
    return 0;
}
void sensor_capture_complete(sensor_t *sensor, image_t *image)
{
    // Abort DMA transfer.
    // Note: In JPEG mode the DMA will still be waiting for data since
    // the max frame size is set, so we need to abort the DMA transfer.
    HAL_DMA_Abort(&BoardDcmiDma);
    // Disable DMA IRQ
    HAL_NVIC_DisableIRQ(DMA1_Stream2_IRQn);
    // Fix the BPP
    switch (sensor->pixformat) {
        case PIXFORMAT_GRAYSCALE:
            MAIN_FB()->bpp = 1;
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_RGB565:
				{
            MAIN_FB()->bpp = 2;
				}break;
        case PIXFORMAT_BAYER:
            MAIN_FB()->bpp = 3;
            break;
        case PIXFORMAT_JPEG:
            // Read the number of data items transferred
            MAIN_FB()->bpp = (MAX_XFER_SIZE - __HAL_DMA_GET_COUNTER(&BoardDcmiDma))*4;
            break;
        default:
            break;
    }
		SCB_CleanInvalidateDCache_by_Addr((uint32_t *)MAIN_FB()->pixels, (MAIN_FB()->w*MAIN_FB()->h*MAIN_FB()->bpp));
		if(sensor->pixformat == PIXFORMAT_GRAYSCALE)
		{
				uint8_t *dst = MAIN_FB()->pixels;
				uint8_t *src = MAIN_FB()->pixels;
				uint32_t length = MAIN_FB()->w * MAIN_FB()->h;
				for(uint32_t addr = 0; addr < length; addr++) {
						*dst = *src;
						dst += 1;
						src += sensor->gs_bpp;
				}
		}
		// Set the user image.
    if (image != NULL) {
        image->w = MAIN_FB()->w;
        image->h = MAIN_FB()->h;
        image->bpp = MAIN_FB()->bpp;
        image->pixels = MAIN_FB()->pixels;
    }
}
void sensor_abort(sensor_t *sensor)
{
	while(DCMI->CR&0X01);               //等待传输完成	
	// Abort DMA transfer.
	// Note: In JPEG mode the DMA will still be waiting for data since
	// the max frame size is set, so we need to abort the DMA transfer.
	HAL_DMA_Abort(&BoardDcmiDma);
	// Disable DMA IRQ
	HAL_NVIC_DisableIRQ(DMA1_Stream2_IRQn);
	// Fix the BPP
	switch (sensor->pixformat) {
			case PIXFORMAT_GRAYSCALE:
					MAIN_FB()->bpp = 1;
					break;
			case PIXFORMAT_YUV422:
			case PIXFORMAT_RGB565:
			{
					MAIN_FB()->bpp = 2;
			}break;
			case PIXFORMAT_BAYER:
					MAIN_FB()->bpp = 3;
					break;
			case PIXFORMAT_JPEG:
					// Read the number of data items transferred
					MAIN_FB()->bpp = (MAX_XFER_SIZE - __HAL_DMA_GET_COUNTER(&BoardDcmiDma))*4;
					break;
			default:
					break;
	}
	SCB_CleanInvalidateDCache_by_Addr((uint32_t *)MAIN_FB()->pixels, (MAIN_FB()->w*MAIN_FB()->h*MAIN_FB()->bpp));
}
