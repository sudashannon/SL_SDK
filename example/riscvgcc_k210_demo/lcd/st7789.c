/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "st7789.h"
#include "gpiohs.h"
#include "spi.h"
#include "pin_config.h"

/* 初始化RS引脚GPIO为输出模式 */
static void init_rs_gpio(void)
{
    gpiohs_set_drive_mode(LCD_RS_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_pin(LCD_RS_GPIONUM, GPIO_PV_HIGH);
}

/* 开始传输命令 */
static void set_start_cmd(void)
{
    gpiohs_set_pin(LCD_RS_GPIONUM, GPIO_PV_LOW);
}

/* 开始传输数据 */
static void set_start_data(void)
{
    gpiohs_set_pin(LCD_RS_GPIONUM, GPIO_PV_HIGH);
}

/* 初始化LCD复位引脚GPIO */
static void init_rst(void)
{
    gpiohs_set_drive_mode(LCD_RST_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_pin(LCD_RST_GPIONUM, GPIO_PV_HIGH);
}

/* 设置LCD-RST电平 */
static void set_rst(uint8_t val)
{
    gpiohs_set_pin(LCD_RST_GPIONUM, val);
}

/* ST7789底层初始化 */
void tft_hard_init(void)
{
    init_rs_gpio();
    init_rst();
    set_rst(0);
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_set_clk_rate(SPI_CHANNEL, 10000000);
    set_rst(1);
}

/* SPI写命令 */
void tft_write_command(uint8_t cmd)
{
    set_start_cmd();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_init_non_standard(SPI_CHANNEL, 8 /*instrction length*/, 0 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_send_data_normal_dma(DMAC_CHANNEL0, SPI_CHANNEL, SPI_SLAVE_SELECT, (uint8_t *)(&cmd), 1, SPI_TRANS_CHAR);
}

/* SPI写数据（uint8_t类型） */
void tft_write_byte(uint8_t *data_buf, uint32_t length)
{
    set_start_data();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_init_non_standard(SPI_CHANNEL, 8 /*instrction length*/, 0 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_send_data_normal_dma(DMAC_CHANNEL0, SPI_CHANNEL, SPI_SLAVE_SELECT, data_buf, length, SPI_TRANS_CHAR);
}

/* SPI写数据（uint16_t类型） */
void tft_write_half(uint16_t *data_buf, uint32_t length)
{
    set_start_data();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 16, 0);
    spi_init_non_standard(SPI_CHANNEL, 16 /*instrction length*/, 0 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_send_data_normal_dma(DMAC_CHANNEL0, SPI_CHANNEL, SPI_SLAVE_SELECT, data_buf, length, SPI_TRANS_SHORT);
}

/* SPI写数据（uint32_t类型） */
void tft_write_word(uint32_t *data_buf, uint32_t length, uint32_t flag)
{
    set_start_data();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 32, 0);

    spi_init_non_standard(SPI_CHANNEL, 0 /*instrction length*/, 32 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_send_data_normal_dma(DMAC_CHANNEL0, SPI_CHANNEL, SPI_SLAVE_SELECT, data_buf, length, SPI_TRANS_INT);
}

/* 以相同的数据填充 */
void tft_fill_data(uint32_t *data_buf, uint32_t length)
{
    set_start_data();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 32, 0);
    spi_init_non_standard(SPI_CHANNEL, 0 /*instrction length*/, 32 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_fill_data_dma(DMAC_CHANNEL0, SPI_CHANNEL, SPI_SLAVE_SELECT, data_buf, length);
}
