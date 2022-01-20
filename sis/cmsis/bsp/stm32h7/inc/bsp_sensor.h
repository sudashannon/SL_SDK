#ifndef __BSP_SENSOR_H
#define __BSP_SENSOR_H

#include "stm32h7xx.h"
#include "rte_include.h"
#include "hal_include.h"

#define FRAME_SIZE          FRAMESIZE_QVGA

// Sensor external clock timer frequency.
#define SENSOR_XCLK_FREQUENCY      (24000000)
// Sensor PLL register value.
#define SENSOR_OV7725_PLL_CONFIG   (0x41)  // x4
// Sensor Banding Filter Value
#define SENSOR_OV7725_BANDING      (0x7F)
// Chip I2C's address
#define OV2640_SLV_ADDR     (0x60)
#define OV7725_SLV_ADDR     (0x42)
// Chip ID Registers
#define OV_CHIP_ID          (0x0A)
// Chip ID Values
#define OV2640_ID           (0x26)
#define OV7725_ID           (0x77)

typedef enum {
    PIXFORMAT_INVALID = 0,
    PIXFORMAT_BINARY,    // 1BPP/BINARY
    PIXFORMAT_GRAYSCALE, // 1BPP/GRAYSCALE
    PIXFORMAT_RGB565,    // 2BPP/RGB565
    PIXFORMAT_YUV422,    // 2BPP/YUV422
    PIXFORMAT_BAYER,     // 1BPP/RAW
    PIXFORMAT_JPEG,      // JPEG/COMPRESSED
} pixformat_t;

typedef enum {
    FRAMESIZE_INVALID = 0,
    // C/SIF Resolutions
    FRAMESIZE_QQCIF,    // 88x72
    FRAMESIZE_QCIF,     // 176x144
    FRAMESIZE_CIF,      // 352x288
    FRAMESIZE_QQSIF,    // 88x60
    FRAMESIZE_QSIF,     // 176x120
    FRAMESIZE_SIF,      // 352x240
    // VGA Resolutions
    FRAMESIZE_QQQQVGA,  // 40x30
    FRAMESIZE_QQQVGA,   // 80x60
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QVGA,     // 320x240
    FRAMESIZE_VGA,      // 640x480
    FRAMESIZE_HQQQQVGA, // 30x20
    FRAMESIZE_HQQQVGA,  // 60x40
    FRAMESIZE_HQQVGA,   // 120x80
    FRAMESIZE_HQVGA,    // 240x160
    FRAMESIZE_HVGA,     // 480x320
    // FFT Resolutions
    FRAMESIZE_64X32,    // 64x32
    FRAMESIZE_64X64,    // 64x64
    FRAMESIZE_128X64,   // 128x64
    FRAMESIZE_128X128,  // 128x128
    // Himax Resolutions
    FRAMESIZE_160X160,  // 160x160
    FRAMESIZE_320X320,  // 320x320
    // Other
    FRAMESIZE_LCD,      // 128x160
    FRAMESIZE_QQVGA2,   // 128x160
    FRAMESIZE_WVGA,     // 720x480
    FRAMESIZE_WVGA2,    // 752x480
    FRAMESIZE_SVGA,     // 800x600
    FRAMESIZE_XGA,      // 1024x768
    FRAMESIZE_WXGA,     // 1280x768
    FRAMESIZE_SXGA,     // 1280x1024
    FRAMESIZE_SXGAM,    // 1280x960
    FRAMESIZE_UXGA,     // 1600x1200
    FRAMESIZE_HD,       // 1280x720
    FRAMESIZE_FHD,      // 1920x1080
    FRAMESIZE_QHD,      // 2560x1440
    FRAMESIZE_QXGA,     // 2048x1536
    FRAMESIZE_WQXGA,    // 2560x1600
    FRAMESIZE_WQXGA2,   // 2592x1944
} framesize_t;

typedef enum {
    GAINCEILING_2X,
    GAINCEILING_4X,
    GAINCEILING_8X,
    GAINCEILING_16X,
    GAINCEILING_32X,
    GAINCEILING_64X,
    GAINCEILING_128X,
} gainceiling_t;

typedef enum {
    SDE_NORMAL,
    SDE_NEGATIVE,
} sde_t;

typedef enum {
    ATTR_CONTRAST=0,
    ATTR_BRIGHTNESS,
    ATTR_SATURATION,
    ATTR_GAINCEILING,
} sensor_attr_t;

typedef enum {
    ACTIVE_LOW,
    ACTIVE_HIGH
} polarity_t;

typedef enum {
    IOCTL_SET_READOUT_WINDOW,
    IOCTL_GET_READOUT_WINDOW,
    IOCTL_SET_TRIGGERED_MODE,
    IOCTL_GET_TRIGGERED_MODE,
    IOCTL_TRIGGER_AUTO_FOCUS,
    IOCTL_PAUSE_AUTO_FOCUS,
    IOCTL_RESET_AUTO_FOCUS,
    IOCTL_WAIT_ON_AUTO_FOCUS,
    IOCTL_LEPTON_GET_WIDTH,
    IOCTL_LEPTON_GET_HEIGHT,
    IOCTL_LEPTON_GET_RADIOMETRY,
    IOCTL_LEPTON_GET_REFRESH,
    IOCTL_LEPTON_GET_RESOLUTION,
    IOCTL_LEPTON_RUN_COMMAND,
    IOCTL_LEPTON_SET_ATTRIBUTE,
    IOCTL_LEPTON_GET_ATTRIBUTE,
    IOCTL_LEPTON_GET_FPA_TEMPERATURE,
    IOCTL_LEPTON_GET_AUX_TEMPERATURE,
    IOCTL_LEPTON_SET_MEASUREMENT_MODE,
    IOCTL_LEPTON_GET_MEASUREMENT_MODE,
    IOCTL_LEPTON_SET_MEASUREMENT_RANGE,
    IOCTL_LEPTON_GET_MEASUREMENT_RANGE,
    IOCTL_HIMAX_MD_ENABLE,
    IOCTL_HIMAX_MD_CLEAR,
    IOCTL_HIMAX_MD_WINDOW,
    IOCTL_HIMAX_MD_THRESHOLD,
    IOCTL_HIMAX_OSC_ENABLE,
} ioctl_t;

typedef enum {
    SENSOR_ERROR_NO_ERROR               =  0,
    SENSOR_ERROR_CTL_FAILED             = -1,
    SENSOR_ERROR_CTL_UNSUPPORTED        = -2,
    SENSOR_ERROR_ISC_UNDETECTED         = -3,
    SENSOR_ERROR_ISC_UNSUPPORTED        = -4,
    SENSOR_ERROR_ISC_INIT_FAILED        = -5,
    SENSOR_ERROR_TIM_INIT_FAILED        = -6,
    SENSOR_ERROR_DMA_INIT_FAILED        = -7,
    SENSOR_ERROR_DCMI_INIT_FAILED       = -8,
    SENSOR_ERROR_IO_ERROR               = -9,
    SENSOR_ERROR_CAPTURE_FAILED         = -10,
    SENSOR_ERROR_CAPTURE_TIMEOUT        = -11,
    SENSOR_ERROR_INVALID_FRAMESIZE      = -12,
    SENSOR_ERROR_INVALID_PIXFORMAT      = -13,
    SENSOR_ERROR_INVALID_WINDOW         = -14,
    SENSOR_ERROR_INVALID_FRAMERATE      = -15,
    SENSOR_ERROR_INVALID_ARGUMENT       = -16,
    SENSOR_ERROR_PIXFORMAT_UNSUPPORTED  = -17,
    SENSOR_ERROR_FRAMEBUFFER_ERROR      = -18,
    SENSOR_ERROR_FRAMEBUFFER_OVERFLOW   = -19,
    SENSOR_ERROR_JPEG_OVERFLOW          = -20,
} sensor_error_t;

#define SENSOR_HW_FLAGS_VSYNC           (0) // vertical sync polarity.
#define SENSOR_HW_FLAGS_HSYNC           (1) // horizontal sync polarity.
#define SENSOR_HW_FLAGS_PIXCK           (2) // pixel clock edge.
#define SENSOR_HW_FLAGS_FSYNC           (3) // hardware frame sync.
#define SENSOR_HW_FLAGS_JPEGE           (4) // hardware JPEG encoder.
#define SENSOR_HW_FLAGS_RGB565_REV      (5) // byte reverse rgb565.
#define SENSOR_HW_FLAGS_GET(s, x)       ((s)->hw_flags &  (1<<x))
#define SENSOR_HW_FLAGS_SET(s, x, v)    ((s)->hw_flags |= (v<<x))
#define SENSOR_HW_FLAGS_CLR(s, x)       ((s)->hw_flags &= ~(1<<x))

typedef struct _sensor sensor_t;
typedef struct _sensor {
    union {
    uint8_t  chip_id;           // Sensor ID.
    uint16_t chip_id_w;         // Sensor ID 16 bits.
    };
    uint8_t  slv_addr;          // Sensor I2C slave address.
    uint16_t gs_bpp;            // Grayscale bytes per pixel.
    uint32_t hw_flags;          // Hardware flags (clock polarities/hw capabilities)

    polarity_t pwdn_pol;        // PWDN polarity (TODO move to hw_flags)
    polarity_t reset_pol;       // Reset polarity (TODO move to hw_flags)

    // Sensor state
    sde_t sde;                  // Special digital effects
    pixformat_t pixformat;      // Pixel format
    framesize_t framesize;      // Frame size
    int framerate;              // Frame rate
    gainceiling_t gainceiling;  // AGC gainceiling

    // Resources
    I2C_HandleTypeDef *bus;     // SCCB/I2C bus.
    DCMI_HandleTypeDef *dcmi;
    void *sema;

    // Sensor function pointers
    int  (*reset)               (sensor_t *sensor);
    int  (*sleep)               (sensor_t *sensor, int enable);
    int  (*read_reg)            (sensor_t *sensor, uint16_t reg_addr);
    int  (*write_reg)           (sensor_t *sensor, uint16_t reg_addr, uint16_t reg_data);
    int  (*set_pixformat)       (sensor_t *sensor, pixformat_t pixformat);
    int  (*set_framesize)       (sensor_t *sensor, framesize_t framesize);
    int  (*set_framerate)       (sensor_t *sensor, int framerate);
    int  (*set_contrast)        (sensor_t *sensor, int level);
    int  (*set_brightness)      (sensor_t *sensor, int level);
    int  (*set_saturation)      (sensor_t *sensor, int level);
    int  (*set_gainceiling)     (sensor_t *sensor, gainceiling_t gainceiling);
    int  (*set_quality)         (sensor_t *sensor, int quality);
    int  (*set_colorbar)        (sensor_t *sensor, int enable);
    int  (*set_auto_gain)       (sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling);
    int  (*get_gain_db)         (sensor_t *sensor, float *gain_db);
    int  (*set_auto_exposure)   (sensor_t *sensor, int enable, int exposure_us);
    int  (*get_exposure_us)     (sensor_t *sensor, int *exposure_us);
    int  (*set_auto_whitebal)   (sensor_t *sensor, int enable, float r_gain_db, float g_gain_db, float b_gain_db);
    int  (*get_rgb_gain_db)     (sensor_t *sensor, float *r_gain_db, float *g_gain_db, float *b_gain_db);
    int  (*set_hmirror)         (sensor_t *sensor, int enable);
    int  (*set_vflip)           (sensor_t *sensor, int enable);
    int  (*set_special_effect)  (sensor_t *sensor, sde_t sde);
    int  (*set_lens_correction) (sensor_t *sensor, int enable, int radi, int coef);
    int  (*ioctl)               (sensor_t *sensor, int request, va_list ap);
    int  (*snapshot)            (sensor_t *sensor, uint8_t **pimage, uint32_t flags);
} sensor_t;

extern sensor_t sensor;

// Resolution table
extern const int resolution[][2];

// Initialize the sensor hardware and probe the image sensor.
int sensor_init();

// Configure DCMI hardware interface.
int sensor_dcmi_config(uint32_t pixformat);

// Abort frame capture and disable IRQs, DMA etc..
int sensor_abort();

// Reset the sensor to its default state.
int sensor_reset();

// Return sensor PID.
int sensor_get_id();

// Returns the xclk freq in hz.
uint32_t sensor_get_xclk_frequency();

// Returns the xclk freq in hz.
int sensor_set_xclk_frequency(uint32_t frequency);

// Return true if the sensor was detected and initialized.
bool sensor_is_detected();

// Sleep mode.
int sensor_sleep(int enable);

// Shutdown mode.
int sensor_shutdown(int enable);

// Read a sensor register.
int sensor_read_reg(uint16_t reg_addr);

// Write a sensor register.
int sensor_write_reg(uint16_t reg_addr, uint16_t reg_data);

// Set the sensor pixel format.
int sensor_set_pixformat(pixformat_t pixformat);

// Set the sensor frame size.
int sensor_set_framesize(framesize_t framesize);

// Set the sensor frame rate.
int sensor_set_framerate(int framerate);

// Return the number of bytes per pixel to read from the image sensor.
uint32_t sensor_get_src_bpp();

// Return the number of bytes per pixel to write to memory.
uint32_t sensor_get_dst_bpp();

// Set the sensor contrast level (from -3 to +3).
int sensor_set_contrast(int level);

// Set the sensor brightness level (from -3 to +3).
int sensor_set_brightness(int level);

// Set the sensor saturation level (from -3 to +3).
int sensor_set_saturation(int level);

// Set the sensor AGC gain ceiling.
// Note: This function has no effect when AGC (Automatic Gain Control) is disabled.
int sensor_set_gainceiling(gainceiling_t gainceiling);

// Set the quantization scale factor, controls JPEG quality (quality 0-255).
int sensor_set_quality(int qs);

// Enable/disable the colorbar mode.
int sensor_set_colorbar(int enable);

// Enable auto gain or set value manually.
int sensor_set_auto_gain(int enable, float gain_db, float gain_db_ceiling);

// Get the gain value.
int sensor_get_gain_db(float *gain_db);

// Enable auto exposure or set value manually.
int sensor_set_auto_exposure(int enable, int exposure_us);

// Get the exposure value.
int sensor_get_exposure_us(int *get_exposure_us);

// Enable auto white balance or set value manually.
int sensor_set_auto_whitebal(int enable, float r_gain_db, float g_gain_db, float b_gain_db);

// Get the rgb gain values.
int sensor_get_rgb_gain_db(float *r_gain_db, float *g_gain_db, float *b_gain_db);

// Set special digital effects (SDE).
int sensor_set_special_effect(sde_t sde);

// Set lens shading correction
int sensor_set_lens_correction(int enable, int radi, int coef);

// IOCTL function
int sensor_ioctl(int request, ...);

// Default snapshot function.
int sensor_snapshot(sensor_t *sensor, uint8_t *pimage, uint32_t timeout_ms);

// Convert sensor error codes to strings.
const char *sensor_strerror(int error);

int cambus_readb(I2C_HandleTypeDef *bus, uint8_t slv_addr, uint8_t reg_addr, uint8_t *reg_data);

int cambus_writeb(I2C_HandleTypeDef *bus, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data);

#endif
