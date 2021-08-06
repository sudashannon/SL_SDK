#include "SDL.h"
#include "monitor.h"
#include "lvgl.h"
#include "rte_include.h"

int gui_thread(void *param)
{
    /*Initialize LittlevGL*/
    lv_log_register_print_cb(log_output);
    lv_init();
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    monitor_init();
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
    static lv_color_t buf_2_1[MONITOR_HOR_RES * 10];                        /*A buffer for 10 rows*/
    static lv_color_t buf_2_2[MONITOR_HOR_RES * 10];                        /*An other buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, MONITOR_HOR_RES * 10);   /*Initialize the display buffer*/

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
    disp_drv.hor_res = MONITOR_HOR_RES;
    disp_drv.ver_res = MONITOR_VER_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = monitor_flush;

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
    /*Create a Label on the currently active screen*/
    lv_obj_t * label1 =  lv_label_create(scr);
    /*Modify the Label's text*/
    lv_label_set_text(label1, "Hello world!");
    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
    for (;;) {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_task_handler();
        lv_refr_now(disp);
        SDL_Delay(5);
    }
    return 0;
}
