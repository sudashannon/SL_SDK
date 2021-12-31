/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        摄像头显示当前画面
* @details
* @par History  见如下说明
*
* version:	V1.0: 摄像头采集当前画面，然后在LCD上显示。
*/
#include <stdio.h>
#include "unistd.h"
#include "gpiohs.h"
#include "lcd.h"
#include "st7789.h"
#include "sysctl.h"
#include "uarths.h"
#include "iomem.h"
#include "bsp_sensor.h"
#include "pin_config.h"

extern const unsigned char gImage_logo[153608];
uint32_t *display_buf = NULL;
#define DISPLAY_WIDTH       160
#define DISPLAY_HEIGHT      120
/**
* Function       io_set_power
* @author        Gengyue
* @date          2020.05.27
* @brief         设置摄像头和显示器电源域电压值
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void io_set_power(void)
{
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         硬件初始化，绑定GPIO口
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void hardware_init(void)
{
    /* lcd */
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);

    /* DVP camera */
    fpioa_set_function(PIN_DVP_RST,   FUNC_CMOS_RST);
    fpioa_set_function(PIN_DVP_PWDN,  FUNC_CMOS_PWDN);
    fpioa_set_function(PIN_DVP_XCLK,  FUNC_CMOS_XCLK);
    fpioa_set_function(PIN_DVP_VSYNC, FUNC_CMOS_VSYNC);
    fpioa_set_function(PIN_DVP_HSYNC, FUNC_CMOS_HREF);
    fpioa_set_function(PIN_DVP_PCLK,  FUNC_CMOS_PCLK);
    fpioa_set_function(PIN_DVP_SCL,   FUNC_SCCB_SCLK);
    fpioa_set_function(PIN_DVP_SDA,   FUNC_SCCB_SDA);

    /* 使能SPI0和DVP */
    sysctl_set_spi0_dvp_data(1);
}

void refresh_disp_image(image_t *cap_image, void *disp_data)
{
    uint32_t w_ratio = ((uint32_t)cap_image->w << 16) / DISPLAY_WIDTH + 1;
    uint32_t h_ratio = ((uint32_t)cap_image->h << 16) / DISPLAY_HEIGHT + 1;
    uint16_t *dest_data = (uint16_t *)disp_data;
    uint32_t srcy = 0;
    for (uint16_t y = 0; y < DISPLAY_HEIGHT; y++) {
        uint16_t *src_data = (uint16_t *)(cap_image->data + cap_image->w * cap_image->bpp * (srcy >> 16));
        uint32_t srcx = 0;
        for (uint16_t x = 0; x < DISPLAY_WIDTH; x++) {
            // if (cap_image->bpp == IMAGE_BPP_GRAYSCALE) {
            //     dest_data[x] = COLOR_GRAYSCALE_TO_RGB565(((uint8_t *)src_data)[srcx >> 16]);
            // } else {
                dest_data[x] =  src_data[srcx >> 16];
            // }
            srcx += w_ratio;
        }
        srcy += h_ratio;
        dest_data += DISPLAY_WIDTH;
    }
}

/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         主函数，程序的入口
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
int main(void)
{
    /* 硬件引脚初始化 */
    hardware_init();

    /* 设置IO口电压 */
    io_set_power();

    /* 设置系统时钟和DVP时钟 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 300000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

    /* 初始化LCD */
    lcd_init();

    /* LCD显示图片 */
    uint16_t *img = (uint16_t *)&gImage_logo;
    lcd_draw_picture_half(0, 0, 320, 240, img);
    lcd_draw_string(16, 40, "Hello Yahboom!", RED);
    lcd_draw_string(16, 60, "Nice to meet you!", BLUE);
    sleep(1);
    printf("unused memory %d\r\n", iomem_unused());
    /* 初始化摄像头 */
    int result = sensor_init();
    printf("sensor_init result: %d\r\n", result);
    result = sensor_reset();
    printf("sensor_reset result: %d\r\n", result);
    result = sensor_set_pixformat(PIXFORMAT_RGB565);
    printf("sensor_set_pixformat result: %d\r\n", result);
    result = sensor_set_framesize(FRAME_SIZE);
    printf("sensor_set_framesize result: %d\r\n", result);
    image_t sensor_cap_image = {
        .w = resolution[FRAME_SIZE][0],
        .h = resolution[FRAME_SIZE][1],
        .bpp = IMAGE_BPP_RGB565,
    };
    display_buf =(uint32_t*)iomem_malloc(DISPLAY_WIDTH * DISPLAY_HEIGHT * 2);
    while (1) {
        sensor_snapshot(&sensor, &sensor_cap_image, 150);
        refresh_disp_image(&sensor_cap_image, (void *)display_buf);
        /* 显示画面 */
        lcd_draw_picture(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, display_buf);
    }

    return 0;
}
