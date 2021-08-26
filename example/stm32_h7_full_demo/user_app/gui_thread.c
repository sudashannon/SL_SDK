#include "lvgl.h"
#include "rte_include.h"
#include "cmsis_os2.h"
#include "bsp_lcd.h"
#include "bsp_sensor.h"

static lv_obj_t *img_obj[2] = {NULL};
osThreadId_t gui_tid;

static void lvgl_log_output(const char *data)
{
    log_output(data, strlen(data));
}

lv_obj_t *gui_get_disp_obj(uint8_t index)
{
    return img_obj[index];
}

void refresh_disp_image(image_t *cap_image, lv_obj_t *disp_obj)
{
    lv_img_dsc_t *disp_image = lv_img_get_src(disp_obj);
    if(disp_image) {
        uint32_t w_ratio = ((uint32_t)cap_image->w << 16)/disp_image->header.w + 1;
        uint32_t h_ratio = ((uint32_t)cap_image->h << 16)/disp_image->header.h + 1;
        uint16_t *dest_data = disp_image->data;
        uint32_t srcy = 0;
        for (uint16_t y = 0; y < disp_image->header.h; y++) {
            uint16_t *src_data = cap_image->data + cap_image->w * cap_image->bpp * (srcy >> 16);
            uint32_t srcx = 0;
            for (uint16_t x = 0; x < disp_image->header.w; x++) {
                if (cap_image->bpp == IMAGE_BPP_GRAYSCALE) {
                    dest_data[x] = __REV16(COLOR_GRAYSCALE_TO_RGB565(((uint8_t *)src_data)[srcx >> 16]));
                } else {
                    dest_data[x] =  __REV16((src_data)[srcx >> 16]);
                }
                srcx += w_ratio;
            }
            srcy += h_ratio;
            dest_data += disp_image->header.w;
        }
        lv_obj_invalidate(disp_obj);
    }
}


/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    bsp_lcd_fill_frame(area->x1, area->y1, area->x2, area->y2, color_p);
    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}


__NO_RETURN void gui_thread(void *param)
{
    /* Initialize LittlevGL */
    lv_log_register_print_cb(lvgl_log_output);
    lv_init();
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    bsp_lcd_init();
    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */

    /* Example for 1) */
    // static lv_disp_draw_buf_t draw_buf_dsc_1;
    // static lv_color_t buf_1[MONITOR_HOR_RES * 10];                          /*A buffer for 10 rows*/
    // lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MONITOR_HOR_RES * 10);   /*Initialize the display buffer*/

    /* Example for 2) */
    static lv_disp_draw_buf_t draw_buf_dsc_2;
    lv_color_t *buf_2_1 = (lv_color_t *)memory_calloc(BANK_DEFAULT, 320 * 10 * sizeof(lv_color_t));
    lv_color_t *buf_2_2 = (lv_color_t *)memory_calloc(BANK_DEFAULT, 320 * 10 * sizeof(lv_color_t));
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, 320 * 10);   /*Initialize the display buffer*/

    /* Example for 3) also set disp_drv.full_refresh = 1 below*/
    // static lv_disp_draw_buf_t draw_buf_dsc_3;
    // static lv_color_t buf_3_1[MONITOR_HOR_RES * MONITOR_VER_RES];            /*A screen sized buffer*/
    // static lv_color_t buf_3_2[MONITOR_HOR_RES * MONITOR_VER_RES];            /*An other screen sized buffer*/
    // lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2, MONITOR_VER_RES * MONITOR_HOR_RES);   /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_2;

    /*Required for Example 3)*/
    //disp_drv.full_refresh = 1;

    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.*/
    //disp_drv.gpu_fill_cb = gpu_fill;

    /*Finally register the driver*/
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    lv_obj_t * scr = lv_disp_get_scr_act(NULL);     /*Get the current screen*/
    lv_disp_set_default(disp);
    /*Create a Label on the currently active screen*/
    lv_obj_t * label1 =  lv_label_create(scr);
    /*Modify the Label's text*/
    lv_label_set_text(label1, "Hello world!");
    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    img_obj[0] = lv_img_create(scr);
    lv_img_dsc_t sensor_disp_image;
	sensor_disp_image.header.cf = LV_IMG_CF_TRUE_COLOR;
	sensor_disp_image.header.always_zero = 0;
	sensor_disp_image.header.w = 100;
	sensor_disp_image.header.h = sensor_disp_image.header.w * resolution[FRAME_SIZE][1] / resolution[FRAME_SIZE][0];
    sensor_disp_image.data = memory_alloc(BANK_DEFAULT, sensor_disp_image.header.w * sensor_disp_image.header.h * 2);
	sensor_disp_image.data_size = sensor_disp_image.header.w * sensor_disp_image.header.h * sizeof(lv_color_t);
	lv_img_set_src(img_obj[0], &sensor_disp_image);
    lv_obj_align(img_obj[0], LV_ALIGN_TOP_RIGHT, 0, 0);

    img_obj[1] = lv_img_create(scr);
    lv_img_dsc_t dealt_image;
	dealt_image.header.cf = LV_IMG_CF_TRUE_COLOR;
	dealt_image.header.always_zero = 0;
	dealt_image.header.w = 200;
	dealt_image.header.h = dealt_image.header.w * resolution[FRAME_SIZE][1] / resolution[FRAME_SIZE][0];
    dealt_image.data = memory_alloc(BANK_DEFAULT, dealt_image.header.w * dealt_image.header.h * 2);
	dealt_image.data_size = dealt_image.header.w * dealt_image.header.h * sizeof(lv_color_t);
	lv_img_set_src(img_obj[1], &dealt_image);
    lv_obj_align(img_obj[1], LV_ALIGN_BOTTOM_LEFT, 0, 0);

    for (;;) {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_task_handler();
        lv_refr_now(disp);
        osDelay(10);
    }
}