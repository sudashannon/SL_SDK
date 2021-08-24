#include "cmsis_os2.h"
#include "bsp_sensor.h"
#include "lvgl.h"

osThreadId_t sensor_tid;
extern void refresh_disp_image(image_t *cap_image);

__NO_RETURN void sensor_thread(void *param)
{
    int result = sensor_init();
    RTE_LOGI("sensor_init result: %d", result);
    result = sensor_reset();
    RTE_LOGI("sensor_reset result: %d", result);
    result = sensor_set_pixformat(PIXFORMAT_GRAYSCALE);
    RTE_LOGI("sensor_set_pixformat result: %d", result);
    result = sensor_set_framesize(FRAME_SIZE);
    RTE_LOGI("sensor_set_framesize result: %d", result);
    image_t sensor_cap_image;
    image_init(&sensor_cap_image, resolution[FRAME_SIZE][0], resolution[FRAME_SIZE][1], IMAGE_BPP_GRAYSCALE);
    rectangle_t roi = {
        .x = 0,
        .y = 0,
        .w = sensor_cap_image.w,
        .h = sensor_cap_image.h,
    };
    for (;;) {
        int snap_result = sensor_snapshot(&sensor, &sensor_cap_image, 150);
        if (snap_result == 0) {
            edge_canny(&sensor_cap_image, &roi, 100, 10);
            refresh_disp_image(&sensor_cap_image);
        } else {
            RTE_LOGF("sensor_snap failed! %d", snap_result);
        }
    }
}
