#include "cmsis_os2.h"
#include "bsp_sensor.h"
#include "bsp_jpege.h"
#include "lvgl.h"

osThreadId_t sensor_tid;
extern void refresh_disp_image(image_t *cap_image, lv_obj_t *disp_obj);
extern lv_obj_t *gui_get_disp_obj(uint8_t index);

__NO_RETURN void sensor_thread(void *param)
{
    bsp_jpeg_init();
    int result = sensor_init();
    RTE_LOGI("sensor_init result: %d", result);
    result = sensor_reset();
    RTE_LOGI("sensor_reset result: %d", result);
    result = sensor_set_pixformat(PIXFORMAT_GRAYSCALE);
    RTE_LOGI("sensor_set_pixformat result: %d", result);
    result = sensor_set_framesize(FRAME_SIZE);
    RTE_LOGI("sensor_set_framesize result: %d", result);
    image_t sensor_cap_image;
    image_t jpeg_image;
    image_init(&sensor_cap_image, resolution[FRAME_SIZE][0], resolution[FRAME_SIZE][1], IMAGE_BPP_GRAYSCALE);
    image_init(&jpeg_image, resolution[FRAME_SIZE][0], resolution[FRAME_SIZE][1], IMAGE_BPP_JPEG);
    rectangle_t roi = {
        .x = 0,
        .y = 0,
        .w = sensor_cap_image.w,
        .h = sensor_cap_image.h,
    };
    for (;;) {
        int snap_result = sensor_snapshot(&sensor, &sensor_cap_image, 150);
        rte_error_t jpeg_result = bsp_jpeg_compress(&sensor_cap_image, &jpeg_image, 60);
        if (jpeg_result == 0) {
            extern hal_device_t *com_debug;
            hal_device_write_async(com_debug, jpeg_image.data, (uint32_t)jpeg_image.bpp, HAL_MAX_DELAY);
        }
        if (snap_result == 0) {
            // refresh_disp_image(&sensor_cap_image, gui_get_disp_obj(0));
            // edge_simple(&sensor_cap_image, &roi, 15, 200);
            // linked_list_t *line_ll = hough_find_lines(&sensor_cap_image, &roi, 2, 1, 30000, 25, 25);
            // if (line_ll) {
            //     while (list_count(line_ll)) {
            //         find_lines_list_lnk_data_t *lnk_line = list_pop_tail_value(line_ll);
            //         image_draw_line(&sensor_cap_image, lnk_line->line.x1, lnk_line->line.y1, lnk_line->line.x2, lnk_line->line.y2, 255, 2);
            //         data_free(lnk_line);
            //     }
            //     list_destroy(line_ll);
            // }
            // linked_list_t *circle_ll = hough_find_circles(&sensor_cap_image, &roi, 2, 1, 12000, 10, 10, 10, 5, 100, 2);
            // if (circle_ll) {
            //     while (list_count(circle_ll)) {
            //         find_circles_list_lnk_data_t *lnk_circle = list_pop_tail_value(circle_ll);
            //         image_draw_circle(&sensor_cap_image, lnk_circle->p.x, lnk_circle->p.y, lnk_circle->r, 255, 2, true);
            //         data_free(lnk_circle);
            //     }
            //     list_destroy(circle_ll);
            // }
            refresh_disp_image(&sensor_cap_image, gui_get_disp_obj(1));
        } else {
            RTE_LOGF("sensor_snap failed! %d", snap_result);
        }
    }
}
