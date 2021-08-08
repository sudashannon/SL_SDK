#include "cmsis_os2.h"
#include "bsp_sensor.h"
#include "lvgl.h"

static image_t sensor_cap_image;
extern uint8_t *get_disp_data(void);
extern void refresh_disp_image(void);

__NO_RETURN void sensor_thread(void *param)
{
    int result = sensor_init();
    RTE_LOGI("sensor_init result: %d", result);
    result = sensor_reset();
    RTE_LOGI("sensor_reset result: %d", result);
    result = sensor_set_pixformat(PIXFORMAT_RGB565);
    RTE_LOGI("sensor_set_pixformat result: %d", result);
    result = sensor_set_framesize(FRAMESIZE_QVGA);
    RTE_LOGI("sensor_set_framesize result: %d", result);
    image_init(&sensor_cap_image, get_disp_data(), resolution[FRAMESIZE_QVGA][0],
                                resolution[FRAMESIZE_QVGA][1], PIXFORMAT_RGB565);
    for (;;) {
        int snap_result = sensor_snapshot(&sensor, &sensor_cap_image, 1000);
        if (snap_result == 0)
            refresh_disp_image();
        else
            RTE_LOGF("sensor_snap failed! %d", snap_result);
    }
}
