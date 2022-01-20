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
#include <stdio.h>
#include "bsp_sensor.h"
#include "ov2640.h"
#include "dvp.h"
#include "plic.h"
#include "sysctl.h"
#include "iomem.h"

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

static int cambus_readb(sensor_t *sensor, uint8_t slv_addr, uint8_t reg_addr, uint8_t *reg_data)
{
    RTE_UNUSED(sensor);
    *reg_data = dvp_sccb_receive_data(slv_addr, reg_addr);
    return 0;
}

static int cambus_writeb(sensor_t *sensor, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data)
{
    RTE_UNUSED(sensor);
    dvp_sccb_send_data(slv_addr, reg_addr, reg_data);
    return 0;
}

uint32_t *sensor_buf = NULL;
volatile uint8_t g_dvp_finish_flag;

/* dvp中断回调函数 */
static int on_dvp_irq_cb(void *ctx)
{
    /* 读取DVP中断状态，如果完成则刷新显示地址的数据，并清除中断标志，否则读取摄像头数据*/
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        dvp_set_display_addr((uint32_t)sensor_buf);
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    } else {
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }
    return 0;
}

int sensor_init(void)
{
    // Reset the sesnor state
    memset(&sensor, 0, sizeof(sensor_t));
    // Set default snapshot function.
    // Some sensors need to call snapshot from init.
    sensor.snapshot = sensor_snapshot;
    sensor.read_byte = cambus_readb;
    sensor.write_byte = cambus_writeb;
    /* DVP初始化，设置sccb的寄存器长度为8bit */
    dvp_init(8);
    /* 设置输入时钟为24000000*/
    dvp_set_xclk_rate(24000000);
    /* 使能突发传输模式 */
    dvp_enable_burst();
    /* 关闭AI输出模式，使能显示模式 */
    dvp_set_output_enable(DVP_OUTPUT_AI, 0);
    dvp_set_output_enable(DVP_OUTPUT_DISPLAY, 1);
    /* 设置输出格式为RGB */
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    /* 设置输出像素大小为320*240 */
    dvp_set_image_size(resolution[FRAME_SIZE][0], resolution[FRAME_SIZE][1]);
    /* 设置DVP的显示地址参数和中断 */
    sensor_buf = (uint32_t*)iomem_malloc(resolution[FRAME_SIZE][0] * resolution[FRAME_SIZE][1] * 2);
    dvp_set_display_addr((uint32_t)sensor_buf);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
    /* DVP 中断配置：中断优先级，中断回调，使能DVP中断 */
    printf("DVP interrupt config\r\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, on_dvp_irq_cb, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    /* 清除DVP中断位 */
    g_dvp_finish_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
    sensor.slv_addr = OV2640_SLV_ADDR;
    cambus_readb(&sensor, sensor.slv_addr, OV_CHIP_ID, &sensor.chip_id);
    printf("sensor chip id is %x\r\n", sensor.chip_id);
    return ov2640_init(&sensor);

}

int sensor_reset(void)
{
    // Reset the sensor state
    sensor.sde                  = 0;
    sensor.pixformat            = 0;
    sensor.framesize            = 0;
    sensor.framerate            = 0;
    sensor.gainceiling          = 0;

    // Check if the control is supported.
    if (sensor.reset == NULL) {
        return SENSOR_ERROR_CTL_UNSUPPORTED;
    }

    // Call sensor-specific reset function
    if (sensor.reset(&sensor) != 0) {
        return SENSOR_ERROR_CTL_FAILED;
    }

    return 0;
}

int sensor_get_id()
{
    return sensor.chip_id_w;
}

uint32_t sensor_get_xclk_frequency()
{
    return SENSOR_XCLK_FREQUENCY;
}



int sensor_sleep(int enable)
{
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

int sensor_set_pixformat(pixformat_t pixformat)
{
    // Check if the value has changed.
    if (sensor.pixformat == pixformat) {
        return 0;
    }

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
    // Reconfigure the DCMI if needed.
    return 0;
}

int sensor_set_framesize(framesize_t framesize)
{
    if (sensor.framesize == framesize) {
        // No change
        return 0;
    }

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

int sensor_snapshot(sensor_t *sensor, uint8_t **simage, uint32_t timeout_ms)
{
    g_dvp_finish_flag = 0;
    dvp_start_convert();
    while (g_dvp_finish_flag == 0);
    *simage = (uint8_t *)sensor_buf;
	// 	if (sensor->pixformat == PIXFORMAT_GRAYSCALE) {
    //         uint8_t *dst = image->data;
    //         uint8_t *src = image->data;
    //         for(uint32_t i = 0; i < pixel_count; i++) {
    //             *dst = *src;
    //             dst ++;
    //             src += sensor->gs_bpp;
    //         }
	// 	}
    return 0;
}
