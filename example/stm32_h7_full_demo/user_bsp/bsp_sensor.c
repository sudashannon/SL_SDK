/**
 * @file bsp_sensor.c
 * @author Leon Shan (813475603@qq.com)
 * @brief United bsp framework for sensor, migrate from openmv.
 * @version 1.0.0
 * @date 2021-08-08
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "bsp_sensor.h"
#include "i2c.h"

const uint16_t rainbow_table[256] = {
    0x063F, 0x063F, 0x065F, 0x067F, 0x069F, 0x06BF, 0x06BF, 0x06DF,
    0x06FF, 0x071F, 0x073F, 0x073F, 0x075F, 0x077F, 0x079F, 0x07BF,
    0x07BF, 0x07DF, 0x07FF, 0x07FF, 0x07FE, 0x07FE, 0x07FD, 0x07FD,
    0x07FD, 0x07FC, 0x07FC, 0x07FB, 0x07FB, 0x07FB, 0x07FA, 0x07FA,
    0x07FA, 0x07F9, 0x07F9, 0x07F8, 0x07F8, 0x07F8, 0x07F7, 0x07F7,
    0x07F6, 0x07F6, 0x07F6, 0x07F5, 0x07F5, 0x07F4, 0x07F4, 0x07F4,
    0x07F3, 0x07F3, 0x07F2, 0x07F2, 0x07F2, 0x07F1, 0x07F1, 0x07F1,
    0x07F0, 0x07F0, 0x07EF, 0x07EF, 0x07EF, 0x07EE, 0x07EE, 0x07ED,
    0x07ED, 0x07ED, 0x07EC, 0x07EC, 0x07EB, 0x07EB, 0x07EB, 0x07EA,
    0x07EA, 0x07EA, 0x07E9, 0x07E9, 0x07E8, 0x07E8, 0x07E8, 0x07E7,
    0x07E7, 0x07E6, 0x07E6, 0x07E6, 0x07E5, 0x07E5, 0x07E4, 0x07E4,
    0x07E4, 0x07E3, 0x07E3, 0x07E2, 0x07E2, 0x07E2, 0x07E1, 0x07E1,
    0x07E1, 0x07E0, 0x07E0, 0x0FE0, 0x0FE0, 0x0FE0, 0x17E0, 0x17E0,
    0x1FE0, 0x1FE0, 0x1FE0, 0x27E0, 0x27E0, 0x2FE0, 0x2FE0, 0x2FE0,
    0x37E0, 0x37E0, 0x3FE0, 0x3FE0, 0x3FE0, 0x47E0, 0x47E0, 0x47E0,
    0x4FE0, 0x4FE0, 0x57E0, 0x57E0, 0x57E0, 0x5FE0, 0x5FE0, 0x67E0,
    0x67E0, 0x67E0, 0x6FE0, 0x6FE0, 0x77E0, 0x77E0, 0x77E0, 0x7FE0,
    0x7FE0, 0x87E0, 0x87E0, 0x87E0, 0x8FE0, 0x8FE0, 0x8FE0, 0x97E0,
    0x97E0, 0x9FE0, 0x9FE0, 0x9FE0, 0xA7E0, 0xA7E0, 0xAFE0, 0xAFE0,
    0xAFE0, 0xB7E0, 0xB7E0, 0xBFE0, 0xBFE0, 0xBFE0, 0xC7E0, 0xC7E0,
    0xC7E0, 0xCFE0, 0xCFE0, 0xD7E0, 0xD7E0, 0xD7E0, 0xDFE0, 0xDFE0,
    0xE7E0, 0xE7E0, 0xE7E0, 0xEFE0, 0xEFE0, 0xF7E0, 0xF7E0, 0xF7E0,
    0xFFE0, 0xFFE0, 0xFFC0, 0xFFA0, 0xFF80, 0xFF80, 0xFF60, 0xFF40,
    0xFF20, 0xFF00, 0xFF00, 0xFEE0, 0xFEC0, 0xFEA0, 0xFE80, 0xFE80,
    0xFE60, 0xFE40, 0xFE20, 0xFE00, 0xFE00, 0xFDE0, 0xFDC0, 0xFDA0,
    0xFD80, 0xFD80, 0xFD60, 0xFD40, 0xFD20, 0xFD00, 0xFD00, 0xFCE0,
    0xFCC0, 0xFCA0, 0xFCA0, 0xFC80, 0xFC60, 0xFC40, 0xFC20, 0xFC20,
    0xFC00, 0xFBE0, 0xFBC0, 0xFBA0, 0xFBA0, 0xFB80, 0xFB60, 0xFB40,
    0xFB20, 0xFB20, 0xFB00, 0xFAE0, 0xFAC0, 0xFAA0, 0xFAA0, 0xFA80,
    0xFA60, 0xFA40, 0xFA20, 0xFA20, 0xFA00, 0xF9E0, 0xF9C0, 0xF9A0,
    0xF9A0, 0xF980, 0xF960, 0xF940, 0xF940, 0xF920, 0xF900, 0xF8E0,
    0xF8C0, 0xF8C0, 0xF8A0, 0xF880, 0xF860, 0xF840, 0xF840, 0xF820
};
// Sensor frame size/resolution table.
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
    {30,   20  },    /* HQQQQVGA  */
    {60,   40  },    /* HQQQVGA   */
    {120,  80  },    /* HQQVGA    */
    {240,  160 },    /* HQVGA     */
    {480,  320 },    /* HVGA      */
    // FFT Resolutions
    {64,   32  },    /* 64x32     */
    {64,   64  },    /* 64x64     */
    {128,  64  },    /* 128x64    */
    {128,  128 },    /* 128x128   */
    // Himax Resolutions
    {160,  160 },    /* 160x160   */
    {320,  320 },    /* 320x320   */
    // Other
    {128,  160 },    /* LCD       */
    {128,  160 },    /* QQVGA2    */
    {720,  480 },    /* WVGA      */
    {752,  480 },    /* WVGA2     */
    {800,  600 },    /* SVGA      */
    {1024, 768 },    /* XGA       */
    {1280, 768 },    /* WXGA      */
    {1280, 1024},    /* SXGA      */
    {1280, 960 },    /* SXGAM     */
    {1600, 1200},    /* UXGA      */
    {1280, 720 },    /* HD        */
    {1920, 1080},    /* FHD       */
    {2560, 1440},    /* QHD       */
    {2048, 1536},    /* QXGA      */
    {2560, 1600},    /* WQXGA     */
    {2592, 1944},    /* WQXGA2    */
};

sensor_t sensor = {};

#define I2C_TIMEOUT             (1000)
#define I2C_SCAN_TIMEOUT        (100)

int cambus_readb(I2C_HandleTypeDef *bus, uint8_t slv_addr, uint8_t reg_addr, uint8_t *reg_data)
{
    int ret = 0;

    if((HAL_I2C_Master_Transmit(bus, slv_addr, &reg_addr, 1, I2C_TIMEOUT) != HAL_OK)
    || (HAL_I2C_Master_Receive (bus, slv_addr, reg_data, 1, I2C_TIMEOUT) != HAL_OK)) {
        ret = -1;
    }
    return ret;
}

int cambus_writeb(I2C_HandleTypeDef *bus, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data)
{
    int ret=0;
    uint8_t buf[] = {reg_addr, reg_data};

    if(HAL_I2C_Master_Transmit(bus, slv_addr, buf, 2, 100) != HAL_OK) {
        ret = -1;
    }
    return ret;
}

static int cambus_scan(I2C_HandleTypeDef *bus)
{
    for (uint8_t addr=0x09; addr<=0x77; addr++) {
        if (HAL_I2C_IsDeviceReady(bus, addr << 1, 10, I2C_SCAN_TIMEOUT) == HAL_OK) {
            return (addr << 1);
        }
    }
#if defined(STM32H7)
    // After a failed scan the bus can get stuck. Re-initializing the bus fixes
    // it, but it seems disabling and re-enabling the bus is all that's needed.
    __HAL_I2C_DISABLE(bus);
    rte_delay_ms(10);
    __HAL_I2C_ENABLE(bus);
#endif
    return 0;
}

int sensor_init()
{
    // Reset the sesnor state
    memset(&sensor, 0, sizeof(sensor_t));
    sensor.bus = &hi2c2;
    sensor.pframebuffer = framebuffer_init(SENSOR_FB_SIZE);
    return SENSOR_ERROR_CTL_UNSUPPORTED;
}

int sensor_abort()
{
    return SENSOR_ERROR_CTL_UNSUPPORTED;
}

int sensor_reset()
{
    // Disable any ongoing frame capture.
    sensor_abort();

    // Reset the sensor state
    sensor.sde                  = 0;
    sensor.pixformat            = 0;
    sensor.framesize            = 0;
    sensor.framerate            = 0;
    sensor.last_frame_ms        = 0;
    sensor.last_frame_ms_valid  = false;
    sensor.gainceiling          = 0;
    sensor.hmirror              = false;
    sensor.vflip                = false;
    sensor.transpose            = false;
    sensor.auto_rotation        = false;
    sensor.vsync_callback       = NULL;
    sensor.frame_callback       = NULL;

    // Reset default color palette.
    sensor.color_palette        = rainbow_table;

    sensor.disable_full_flush   = false;

    // Restore shutdown state on reset.
    sensor_shutdown(false);

    // Disable the bus before reset.
    __HAL_I2C_DISABLE(sensor.bus);

    // Hard-reset the sensor
    if (sensor.reset_pol == ACTIVE_HIGH) {
        gpio_set_high(CAM_RST);
        rte_delay_ms(10);
        gpio_set_low(CAM_RST);
    } else {
        gpio_set_low(CAM_RST);
        rte_delay_ms(10);
        gpio_set_high(CAM_RST);
    }

    rte_delay_ms(20);

    // Re-enable the bus.
    __HAL_I2C_ENABLE(sensor.bus);

    // Check if the control is supported.
    if (sensor.reset == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call sensor-specific reset function
    if (sensor.reset(&sensor) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    // Reset framebuffers
    framebuffer_reset_buffers(sensor.pframebuffer);

    return 0;
}

int sensor_probe_init(void)
{
    int init_ret = 0;

    // Do a power cycle
    gpio_set_high(CAM_PWR);
    rte_delay_ms(10);

    gpio_set_low(CAM_PWR);
    rte_delay_ms(10);

    /* Some sensors have different reset polarities, and we can't know which sensor
       is connected before initializing cambus and probing the sensor, which in turn
       requires pulling the sensor out of the reset state. So we try to probe the
       sensor with both polarities to determine line state. */
    sensor.pwdn_pol = ACTIVE_HIGH;
    sensor.reset_pol = ACTIVE_HIGH;

    // Reset the sensor
    gpio_set_high(CAM_RST);
    rte_delay_ms(10);

    gpio_set_low(CAM_RST);
    rte_delay_ms(10);

    // The camera bus has already been initialized by CubeMX.
    // Probe the sensor
    sensor.slv_addr = cambus_scan(sensor.bus);
    if (sensor.slv_addr == 0) {
        /* Sensor has been held in reset,
           so the reset line is active low */
        sensor.reset_pol = ACTIVE_LOW;

        // Pull the sensor out of the reset state.
        gpio_set_high(CAM_RST);
        rte_delay_ms(10);

        // Probe again to set the slave addr.
        sensor.slv_addr = cambus_scan(sensor.bus);
        if (sensor.slv_addr == 0) {
            sensor.pwdn_pol = ACTIVE_LOW;

            gpio_set_high(CAM_PWR);
            rte_delay_ms(10);

            sensor.slv_addr = cambus_scan(sensor.bus);
            if (sensor.slv_addr == 0) {
                sensor.reset_pol = ACTIVE_HIGH;

                gpio_set_low(CAM_RST);
                rte_delay_ms(10);

                sensor.slv_addr = cambus_scan(sensor.bus);
            }
        }
    }

    switch (sensor.slv_addr) {
        case OV2640_SLV_ADDR: // Or OV9650.
            cambus_readb(sensor.bus, sensor.slv_addr, OV_CHIP_ID, &sensor.chip_id);
            break;
        case OV7725_SLV_ADDR: // Or OV7690 or OV7670.
            cambus_readb(sensor.bus, sensor.slv_addr, OV_CHIP_ID, &sensor.chip_id);
            break;
        default:
            return SENSOR_ERROR_ISC_UNSUPPORTED;
            break;
    }

    switch (sensor.chip_id_w) {
        case OV2640_ID:
            init_ret = ov2640_init(&sensor);
            break;
        case OV7725_ID:
            init_ret = ov7725_init(&sensor);
            break;
        default:
            return SENSOR_ERROR_ISC_UNSUPPORTED;
            break;
    }

    if (init_ret != 0 ) {
        // Sensor init failed.
        return SENSOR_ERROR_ISC_INIT_FAILED;
    }

    return 0;
}

int sensor_get_id()
{
    return sensor.chip_id_w;
}

uint32_t sensor_get_xclk_frequency()
{
    return SENSOR_ERROR_CTL_UNSUPPORTED;
}

int sensor_set_xclk_frequency(uint32_t frequency)
{
    return SENSOR_ERROR_CTL_UNSUPPORTED;
}

bool sensor_is_detected()
{
    return sensor.detected;
}

int sensor_sleep(int enable)
{
    // Disable any ongoing frame capture.
    sensor_abort();

    // Check if the control is supported.
    if (sensor.sleep == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.sleep(&sensor, enable) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_shutdown(int enable)
{
    int ret = 0;

    // Disable any ongoing frame capture.
    sensor_abort();

    if (enable) {
        if (sensor.pwdn_pol == ACTIVE_HIGH) {
            gpio_set_high(CAM_PWR);
        } else {
            gpio_set_low(CAM_PWR);
        }
    } else {
        if (sensor.pwdn_pol == ACTIVE_HIGH) {
            gpio_set_low(CAM_PWR);
        } else {
            gpio_set_high(CAM_PWR);
        }
    }

    rte_delay_ms(10);

    return ret;
}

int sensor_read_reg(uint16_t reg_addr)
{
    // Check if the control is supported.
    if (sensor.read_reg == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.read_reg(&sensor, reg_addr) == -1) {
        return SENSOR_ERROR_IO_ERROR;
    }

    return 0;
}

int sensor_write_reg(uint16_t reg_addr, uint16_t reg_data)
{
    // Check if the control is supported.
    if (sensor.write_reg == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.write_reg(&sensor, reg_addr, reg_data) == -1) {
        return SENSOR_ERROR_IO_ERROR;
    }

    return 0;
}

int sensor_dcmi_config(uint32_t pixformat)
{
    // VSYNC clock polarity
//    DCMIHandle.Init.VSPolarity  = SENSOR_HW_FLAGS_GET(&sensor, SENSOR_HW_FLAGS_VSYNC) ?
//                                    DCMI_VSPOLARITY_HIGH : DCMI_VSPOLARITY_LOW;
//    // HSYNC clock polarity
//    DCMIHandle.Init.HSPolarity  = SENSOR_HW_FLAGS_GET(&sensor, SENSOR_HW_FLAGS_HSYNC) ?
//                                    DCMI_HSPOLARITY_HIGH : DCMI_HSPOLARITY_LOW;
//    // PXCLK clock polarity
//    DCMIHandle.Init.PCKPolarity = SENSOR_HW_FLAGS_GET(&sensor, SENSOR_HW_FLAGS_PIXCK) ?
//                                    DCMI_PCKPOLARITY_RISING : DCMI_PCKPOLARITY_FALLING;
//    // Setup capture parameters.
//    DCMIHandle.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;    // Enable Hardware synchronization
//    DCMIHandle.Init.CaptureRate = DCMI_CR_ALL_FRAME;        // Capture rate all frames
//    DCMIHandle.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B; // Capture 8 bits on every pixel clock
//    // Set JPEG Mode
//    DCMIHandle.Init.JPEGMode = (pixformat == PIXFORMAT_JPEG) ?
//                                    DCMI_JPEG_ENABLE : DCMI_JPEG_DISABLE;
//    #if defined(MCU_SERIES_F7) || defined(MCU_SERIES_H7)
//    DCMIHandle.Init.ByteSelectMode  = DCMI_BSM_ALL;         // Capture all received bytes
//    DCMIHandle.Init.ByteSelectStart = DCMI_OEBS_ODD;        // Ignored
//    DCMIHandle.Init.LineSelectMode  = DCMI_LSM_ALL;         // Capture all received lines
//    DCMIHandle.Init.LineSelectStart = DCMI_OELS_ODD;        // Ignored
//    #endif

//    // Associate the DMA handle to the DCMI handle
//    __HAL_LINKDMA(&DCMIHandle, DMA_Handle, DMAHandle);

//    // Initialize the DCMI
//    HAL_DCMI_DeInit(&DCMIHandle);
//    if (HAL_DCMI_Init(&DCMIHandle) != HAL_OK) {
//        // Initialization Error
//        return -1;
//    }

//    // Configure and enable DCMI IRQ Channel
//    NVIC_SetPriority(DCMI_IRQn, IRQ_PRI_DCMI);
//    HAL_NVIC_EnableIRQ(DCMI_IRQn);
    return 0;
}

bool sensor_get_cropped()
{
    if (sensor.framesize != FRAMESIZE_INVALID) {
        return (((framebuffer_t *)sensor.pframebuffer)->x != 0)                                  // should be zero if not cropped.
            || (((framebuffer_t *)sensor.pframebuffer)->y != 0)                                  // should be zero if not cropped.
            || (((framebuffer_t *)sensor.pframebuffer)->u != resolution[sensor.framesize][0])    // should be equal to the resolution if not cropped.
            || (((framebuffer_t *)sensor.pframebuffer)->v != resolution[sensor.framesize][1]);   // should be equal to the resolution if not cropped.
    }
    return false;
}

int sensor_set_pixformat(pixformat_t pixformat)
{
    // Check if the value has changed.
    if (sensor.pixformat == pixformat) {
        return 0;
    }

    // Some sensor drivers automatically switch to BAYER to reduce the frame size if it does not fit in RAM.
    // If the current format is BAYER (1BPP), and the target format is RGB-565 (2BPP) and the frame does not
    // fit in RAM it will just be switched back again to BAYER, so we keep the current format unchanged.
    uint32_t size = framebuffer_get_buffer_size(sensor.pframebuffer);
    if ((sensor.pixformat == PIXFORMAT_BAYER)
            && (pixformat == PIXFORMAT_RGB565)
            && (((framebuffer_t *)sensor.pframebuffer)->u * ((framebuffer_t *)sensor.pframebuffer)->v * 2 > size)
            && (((framebuffer_t *)sensor.pframebuffer)->u * ((framebuffer_t *)sensor.pframebuffer)->v * 1 <= size)) {
        return 0;
    }

    // Cropping and transposing (and thus auto rotation) don't work in JPEG mode.
    if ((pixformat == PIXFORMAT_JPEG)
            && (sensor_get_cropped() || sensor.transpose || sensor.auto_rotation)) {
        return SENSOR_ERROR_PIXFORMAT_UNSUPPORTED;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    // Check if the control is supported.
    if (sensor.set_pixformat == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_pixformat(&sensor, pixformat) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    rte_delay_ms(100); // wait for the camera to settle

    // Set pixel format
    sensor.pixformat = pixformat;

    // Skip the first frame.
    ((framebuffer_t *)sensor.pframebuffer)->bpp = -1;

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers(sensor.pframebuffer);

    // Reconfigure the DCMI if needed.
    return sensor_dcmi_config(pixformat);
}

int sensor_set_framesize(framesize_t framesize)
{
    if (sensor.framesize == framesize) {
        // No change
        return 0;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    // Call the sensor specific function
    if (sensor.set_framesize == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    if (sensor.set_framesize(&sensor, framesize) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    rte_delay_ms(100); // wait for the camera to settle

    // Set framebuffer size
    sensor.framesize = framesize;

    // Skip the first frame.
    ((framebuffer_t *)sensor.pframebuffer)->bpp = -1;

    // Set MAIN FB x offset, y offset, width, height, backup width, and backup height.
    ((framebuffer_t *)sensor.pframebuffer)->x = 0;
    ((framebuffer_t *)sensor.pframebuffer)->y = 0;
    ((framebuffer_t *)sensor.pframebuffer)->w = ((framebuffer_t *)sensor.pframebuffer)->u = resolution[framesize][0];
    ((framebuffer_t *)sensor.pframebuffer)->h = ((framebuffer_t *)sensor.pframebuffer)->v = resolution[framesize][1];

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers(sensor.pframebuffer);

    return 0;
}

int sensor_set_framerate(int framerate)
{
    if (sensor.framerate == framerate) {
        // No change
        return 0;
    }

    if (framerate < 0) {
        return SENSOR_ERROR_INVALID_ARGUMENT;
    }

    // Call the sensor specific function (does not fail if function is not set)
    if (sensor.set_framerate != NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    if (sensor.set_framerate(&sensor, framerate) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    // Set framerate
    sensor.framerate = framerate;
    return 0;
}


uint32_t sensor_get_src_bpp()
{
    switch (sensor.pixformat) {
        case PIXFORMAT_GRAYSCALE:
            return sensor.gs_bpp;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            return 2;
        case PIXFORMAT_BAYER:
        case PIXFORMAT_JPEG:
            return 1;
        default:
            return 0;
    }
}

uint32_t sensor_get_dst_bpp()
{
    switch (sensor.pixformat) {
        case PIXFORMAT_GRAYSCALE:
        case PIXFORMAT_BAYER:
            return 1;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            return 2;
        default:
            return 0;
    }
}

int sensor_set_windowing(int x, int y, int w, int h)
{
    // Check if the value has changed.
    if ((((framebuffer_t *)sensor.pframebuffer)->x == x) && (((framebuffer_t *)sensor.pframebuffer)->y == y) &&
            (((framebuffer_t *)sensor.pframebuffer)->u == w) && (((framebuffer_t *)sensor.pframebuffer)->v == h)) {
        return 0;
    }

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return SENSOR_ERROR_PIXFORMAT_UNSUPPORTED;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    // Skip the first frame.
    ((framebuffer_t *)sensor.pframebuffer)->bpp = -1;

    ((framebuffer_t *)sensor.pframebuffer)->x = x;
    ((framebuffer_t *)sensor.pframebuffer)->y = y;
    ((framebuffer_t *)sensor.pframebuffer)->w = ((framebuffer_t *)sensor.pframebuffer)->u = w;
    ((framebuffer_t *)sensor.pframebuffer)->h = ((framebuffer_t *)sensor.pframebuffer)->v = h;

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers(sensor.pframebuffer);

    return 0;
}

int sensor_set_contrast(int level)
{
    // Check if the control is supported.
    if (sensor.set_contrast == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_contrast(&sensor, level) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_set_brightness(int level)
{
    // Check if the control is supported.
    if (sensor.set_brightness == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_brightness(&sensor, level) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_set_saturation(int level)
{
    // Check if the control is supported.
    if (sensor.set_saturation == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_saturation(&sensor, level) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_set_gainceiling(gainceiling_t gainceiling)
{
    // Check if the value has changed.
    if (sensor.gainceiling == gainceiling) {
        return 0;
    }

    // Check if the control is supported.
    if (sensor.set_gainceiling == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_gainceiling(&sensor, gainceiling) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    // Set the new control value.
    sensor.gainceiling = gainceiling;

    return 0;
}

int sensor_set_quality(int qs)
{
    // Check if the control is supported.
    if (sensor.set_quality == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_quality(&sensor, qs) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_set_colorbar(int enable)
{
    // Check if the control is supported.
    if (sensor.set_colorbar == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_colorbar(&sensor, enable) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_set_auto_gain(int enable, float gain_db, float gain_db_ceiling)
{
    // Check if the control is supported.
    if (sensor.set_auto_gain == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_auto_gain(&sensor, enable, gain_db, gain_db_ceiling) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_get_gain_db(float *gain_db)
{
    // Check if the control is supported.
    if (sensor.get_gain_db == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.get_gain_db(&sensor, gain_db) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_set_auto_exposure(int enable, int exposure_us)
{
    // Check if the control is supported.
    if (sensor.set_auto_exposure == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_auto_exposure(&sensor, enable, exposure_us) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_get_exposure_us(int *exposure_us)
{
    // Check if the control is supported.
    if (sensor.get_exposure_us == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.get_exposure_us(&sensor, exposure_us) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_set_auto_whitebal(int enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
    // Check if the control is supported.
    if (sensor.set_auto_whitebal == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_auto_whitebal(&sensor, enable, r_gain_db, g_gain_db, b_gain_db) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_get_rgb_gain_db(float *r_gain_db, float *g_gain_db, float *b_gain_db)
{
    // Check if the control is supported.
    if (sensor.get_rgb_gain_db == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.get_rgb_gain_db(&sensor, r_gain_db, g_gain_db, b_gain_db) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_set_hmirror(int enable)
{
    // Check if the value has changed.
    if (sensor.hmirror == ((bool) enable)) {
        return 0;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    // Check if the control is supported.
    if (sensor.set_hmirror == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_hmirror(&sensor, enable) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    // Set the new control value.
    sensor.hmirror = enable;

    // Wait for the camera to settle
    rte_delay_ms(100);

    return 0;
}

bool sensor_get_hmirror()
{
    return sensor.hmirror;
}

int sensor_set_vflip(int enable)
{
    // Check if the value has changed.
    if (sensor.vflip == ((bool) enable)) {
        return 0;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    // Check if the control is supported.
    if (sensor.set_vflip == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_vflip(&sensor, enable) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    // Set the new control value.
    sensor.vflip = enable;

    // Wait for the camera to settle
    rte_delay_ms(100);

    return 0;
}

bool sensor_get_vflip()
{
    return sensor.vflip;
}

int sensor_set_transpose(bool enable)
{
    // Check if the value has changed.
    if (sensor.transpose == enable) {
        return 0;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return SENSOR_ERROR_PIXFORMAT_UNSUPPORTED;
    }

    // Set the new control value.
    sensor.transpose = enable;

    return 0;
}

bool sensor_get_transpose()
{
    return sensor.transpose;
}

int sensor_set_auto_rotation(bool enable)
{
    // Check if the value has changed.
    if (sensor.auto_rotation == enable) {
        return 0;
    }

    // Disable any ongoing frame capture.
    sensor_abort();

    // Operation not supported on JPEG images.
    if (sensor.pixformat == PIXFORMAT_JPEG) {
        return SENSOR_ERROR_PIXFORMAT_UNSUPPORTED;
    }

    // Set the new control value.
    sensor.auto_rotation = enable;
    return 0;
}

bool sensor_get_auto_rotation()
{
    return sensor.auto_rotation;
}

int sensor_set_framebuffers(int count)
{
    // Disable any ongoing frame capture.
    sensor_abort();

    return framebuffer_set_buffers(sensor.pframebuffer, count);
}

int sensor_set_special_effect(sde_t sde)
{
    // Check if the value has changed.
    if (sensor.sde == sde) {
        return 0;
    }

    // Check if the control is supported.
    if (sensor.set_special_effect == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_special_effect(&sensor, sde) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    // Set the new control value.
    sensor.sde = sde;

    return 0;
}

int sensor_set_lens_correction(int enable, int radi, int coef)
{
    // Check if the control is supported.
    if (sensor.set_lens_correction == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (sensor.set_lens_correction(&sensor, enable, radi, coef) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_ioctl(int request, ... /* arg */)
{
    // Disable any ongoing frame capture.
    sensor_abort();

    // Check if the control is supported.
    if (sensor.ioctl == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    va_list ap;
    va_start(ap, request);
    // Call the sensor specific function.
    int ret = sensor.ioctl(&sensor, request, ap);
    va_end(ap);

    return ((ret != 0) ? SENSOR_ERROR_CTL_FAILED : 0);
}

int sensor_set_vsync_callback(vsync_cb_t vsync_cb)
{
    sensor.vsync_callback = vsync_cb;
    return 0;
}

int sensor_set_frame_callback(frame_cb_t vsync_cb)
{
    sensor.frame_callback = vsync_cb;
    return 0;
}

int sensor_set_color_palette(const uint16_t *color_palette)
{
    sensor.color_palette = color_palette;
    return 0;
}

const uint16_t *sensor_get_color_palette()
{
    return sensor.color_palette;
}

int sensor_check_framebuffer_size()
{
    uint32_t bpp = sensor_get_dst_bpp();
    uint32_t size = framebuffer_get_buffer_size(sensor.pframebuffer);
    return (((((framebuffer_t *)sensor.pframebuffer)->u * ((framebuffer_t *)sensor.pframebuffer)->v * bpp) <= size) ? 0 : -1);
}

int sensor_auto_crop_framebuffer()
{
    uint32_t bpp = sensor_get_dst_bpp();
    uint32_t size = framebuffer_get_buffer_size(sensor.pframebuffer);

    // If the pixformat is NULL/JPEG there we can't do anything to check if it fits before hand.
    if (!bpp) {
        return 0;
    }

    // ((framebuffer_t *)sensor.pframebuffer) fits, we are done.
    if ((((framebuffer_t *)sensor.pframebuffer)->u * ((framebuffer_t *)sensor.pframebuffer)->v * bpp) <= size) {
        return 0;
    }

    if (sensor.pixformat == PIXFORMAT_RGB565) {
        // Switch to bayer for the quick 2x savings.
        sensor_set_pixformat(PIXFORMAT_BAYER);
        bpp = 1;

        // ((framebuffer_t *)sensor.pframebuffer) fits, we are done (bpp is 1).
        if ((((framebuffer_t *)sensor.pframebuffer)->u * ((framebuffer_t *)sensor.pframebuffer)->v) <= size) {
            return 0;
        }
    }

    int window_w = ((framebuffer_t *)sensor.pframebuffer)->u;
    int window_h = ((framebuffer_t *)sensor.pframebuffer)->v;

    // We need to shrink the frame buffer. We can do this by cropping. So, we will subtract columns
    // and rows from the frame buffer until it fits within the frame buffer.
    int max = RTE_MAX(window_w, window_h);
    int min = RTE_MIN(window_w, window_h);
    float aspect_ratio = max / ((float) min);
    float r = aspect_ratio, best_r = r;
    int c = 1, best_c = c;
    float best_err = FLT_MAX;

    // Find the width/height ratio that's within 1% of the aspect ratio with a loop limit.
    for (int i = 100; i; i--) {
        float err = fabsf(r - roundf(r));

        if (err <= best_err) {
            best_err = err;
            best_r = r;
            best_c = c;
        }

        if (best_err <= 0.01f) {
            break;
        }

        r += aspect_ratio;
        c += 1;
    }

    // Select the larger geometry to map the aspect ratio to.
    int u_sub, v_sub;

    if (window_w > window_h) {
        u_sub = roundf(best_r);
        v_sub = best_c;
    } else {
        u_sub = best_c;
        v_sub = roundf(best_r);
    }

    // Crop the frame buffer while keeping the aspect ratio and keeping the width/height even.
    while (((((framebuffer_t *)sensor.pframebuffer)->u * ((framebuffer_t *)sensor.pframebuffer)->v * bpp) > size) || (((framebuffer_t *)sensor.pframebuffer)->u % 2)  || (((framebuffer_t *)sensor.pframebuffer)->v % 2)) {
        ((framebuffer_t *)sensor.pframebuffer)->u -= u_sub;
        ((framebuffer_t *)sensor.pframebuffer)->v -= v_sub;
    }

    // Center the new window using the previous offset and keep the offset even.
    ((framebuffer_t *)sensor.pframebuffer)->x += (window_w - ((framebuffer_t *)sensor.pframebuffer)->u) / 2;
    ((framebuffer_t *)sensor.pframebuffer)->y += (window_h - ((framebuffer_t *)sensor.pframebuffer)->v) / 2;

    if (((framebuffer_t *)sensor.pframebuffer)->x % 2) {
        ((framebuffer_t *)sensor.pframebuffer)->x -= 1;
    }
    if (((framebuffer_t *)sensor.pframebuffer)->y % 2) {
        ((framebuffer_t *)sensor.pframebuffer)->y -= 1;
    }

    // Pickout a good buffer count for the user.
    framebuffer_auto_adjust_buffers(sensor.pframebuffer);
    return 0;
}

const char *sensor_strerror(int error)
{
    static const char * sensor_errors[] = {
        "No error.",
        "Sensor control failed.",
        "The requested operation is not supported by the image sensor.",
        "Failed to detect the image sensor or image sensor is detached.",
        "The detected image sensor is not supported.",
        "Failed to initialize the image sensor.",
        "Failed to initialize the image sensor clock.",
        "Failed to initialize the image sensor DMA.",
        "Failed to initialize the image sensor DCMI.",
        "An low level I/O error has occurred.",
        "Frame capture has failed.",
        "Frame capture has timed out.",
        "Frame size is not supported or is not set.",
        "Pixel format is not supported or is not set.",
        "Window is not supported or is not set.",
        "An invalid argument is used.",
        "The requested operation is not supported on the current pixel format.",
        "Frame buffer error.",
        "Frame buffer overflow, try reducing the frame size.",
        "JPEG frame buffer overflow.",
    };

    // Sensor errors are negative.
    error = ((error < 0) ? (error * -1) : error);

    if (error > (sizeof(sensor_errors) / sizeof(sensor_errors[0]))) {
        return "Unknown error.";
    } else {
        return sensor_errors[error];
    }
}

int sensor_snapshot(sensor_t *sensor, image_t *image, uint32_t flags)
{
    return -1;
}
