#include "cmsis_os2.h"
#include "spi.h"
#include "bsp_lcd.h"

static hal_device_t *spi_lcd = NULL;

static void bsp_lcd_write_data(uint8_t data)
{
    gpio_set_low(LCD_CS);
    gpio_set_high(LCD_DC);
    hal_device_write_sync(spi_lcd, &data, 1, HAL_MAX_DELAY);
    gpio_set_high(LCD_CS);
}

static void bsp_lcd_write_ndata(uint8_t *data, uint16_t size)
{
    gpio_set_low(LCD_CS);
    gpio_set_high(LCD_DC);
    hal_device_write_sync(spi_lcd, data, size, HAL_MAX_DELAY);
    gpio_set_high(LCD_CS);
}

static void bsp_lcd_write_command(uint8_t command)
{
    gpio_set_low(LCD_CS);
    gpio_set_low(LCD_DC);
    hal_device_write_sync(spi_lcd, &command, 1, HAL_MAX_DELAY);
    gpio_set_high(LCD_CS);
}
static void bsp_lcd_set_cursor(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2)
{
	bsp_lcd_write_command(0x2A);
    x1 = (x1) << 8 | (x1) >> 8;
    x2 = (x2) << 8 | (x2) >> 8;
    y1 = (y1) << 8 | (y1) >> 8;
    y2 = (y2) << 8 | (y2) >> 8;
    bsp_lcd_write_ndata(&x1, 2);
    bsp_lcd_write_ndata(&x2, 2);

	bsp_lcd_write_command(0x2B);
    bsp_lcd_write_ndata(&y1, 2);
    bsp_lcd_write_ndata(&y2, 2);
}
void bsp_lcd_put_pixel(uint16_t x, uint16_t y, uint16_t color)
{
	bsp_lcd_set_cursor(x, y, x, y);
	bsp_lcd_write_command(0x2C);
	color = (color) << 8 | (color) >> 8;
    gpio_set_low(LCD_CS);
    gpio_set_high(LCD_DC);
    hal_device_write_sync(spi_lcd, (uint8_t *)&color, 2, HAL_MAX_DELAY);
    gpio_set_high(LCD_CS);
}
void bsp_lcd_fill_frame_slow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	bsp_lcd_set_cursor(x0, x1, y0, y1);
	bsp_lcd_write_command(0x2C);
	color = (color) << 8 | (color) >> 8;
    gpio_set_low(LCD_CS);
    gpio_set_high(LCD_DC);
    for (uint32_t i = 0; i < (x1 - x0 + 1) *(y1 - y0 + 1); i++)
        hal_device_write_sync(spi_lcd, (uint8_t *)&color, 2, HAL_MAX_DELAY);
    gpio_set_high(LCD_CS);
}

void bsp_lcd_init(void)
{
    spi_configuration_t spi_lcd_config = {0};
    spi_lcd_config.spi_handle = &hspi2;
    spi_lcd_config.tx_dma_handle = NULL;
    spi_lcd_config.rx_dma_handle = NULL;
    spi_create(SPI_LCD, &spi_lcd_config, &spi_lcd);

    bsp_lcd_write_command(0x01);
    rte_delay_ms(50);

    bsp_lcd_write_command(0x36);//�ڴ����ݷ��ʿ���
    bsp_lcd_write_data(0x00);

    bsp_lcd_write_command(0x3A);//�ӿ����ظ�ʽ
    bsp_lcd_write_data(0x05);

    bsp_lcd_write_command(0xB2);     //��������
    bsp_lcd_write_data(0x0C);
    bsp_lcd_write_data(0x0C);
    bsp_lcd_write_data(0x00);
    bsp_lcd_write_data(0x33);
    bsp_lcd_write_data(0x33);

    bsp_lcd_write_command(0xB7);    //�ſ���
    bsp_lcd_write_data(0x35);

    bsp_lcd_write_command(0xBB);    //VCOM Setting
    bsp_lcd_write_data(0x19);

    bsp_lcd_write_command(0xC0);    //LCM Control
    bsp_lcd_write_data(0x2C);

    bsp_lcd_write_command(0xC2);   //VDV and VRH Command Enable
    bsp_lcd_write_data(0x01);

    bsp_lcd_write_command(0xC3);  //VRH Set
    bsp_lcd_write_data(0x12);

    bsp_lcd_write_command(0xC4);  //VDV Set
    bsp_lcd_write_data(0x20);

    bsp_lcd_write_command(0xC6);   //����ģʽ�µ�֡�ʿ���
    bsp_lcd_write_data(0x0F); //֡Ƶ��60֡

    bsp_lcd_write_command(0xD0);   //Power Control
    bsp_lcd_write_data(0xA4);
    bsp_lcd_write_data(0xA1);

    bsp_lcd_write_command(0xE0);    //����ѹ٤�����
    bsp_lcd_write_data(0xD0);
    bsp_lcd_write_data(0x04);
    bsp_lcd_write_data(0x0D);
    bsp_lcd_write_data(0x11);
    bsp_lcd_write_data(0x13);
    bsp_lcd_write_data(0x2B);
    bsp_lcd_write_data(0x3F);
    bsp_lcd_write_data(0x54);
    bsp_lcd_write_data(0x4C);
    bsp_lcd_write_data(0x18);
    bsp_lcd_write_data(0x0D);
    bsp_lcd_write_data(0x0B);
    bsp_lcd_write_data(0x1F);
    bsp_lcd_write_data(0x23);

    bsp_lcd_write_command(0xE1);   //����ѹ٤�����
    bsp_lcd_write_data(0xD0);
    bsp_lcd_write_data(0x04);
    bsp_lcd_write_data(0x0C);
    bsp_lcd_write_data(0x11);
    bsp_lcd_write_data(0x13);
    bsp_lcd_write_data(0x2C);
    bsp_lcd_write_data(0x3F);
    bsp_lcd_write_data(0x44);
    bsp_lcd_write_data(0x51);
    bsp_lcd_write_data(0x2F);
    bsp_lcd_write_data(0x1F);
    bsp_lcd_write_data(0x1F);
    bsp_lcd_write_data(0x20);
    bsp_lcd_write_data(0x23);

    bsp_lcd_write_command(0x21); //��ʾ��ת��
    bsp_lcd_write_command(0x11); //�˳�����

    rte_delay_ms(100);
	bsp_lcd_write_command(0x29); //����ʾ
    gpio_set_high(LCD_BLK);
    bsp_lcd_fill_frame_slow(0, 0, 120, 100, 0x5241);

}
