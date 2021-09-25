#include "cmsis_os2.h"
#include "bsp_jpege.h"
#include "bsp_sensor.h"
#include "lvgl.h"

osThreadId_t jpeg_tid;
osSemaphoreId_t jpeg_sema;
extern void refresh_disp_image(image_t *cap_image, lv_obj_t *disp_obj);
extern lv_obj_t *gui_get_disp_obj(uint8_t index);

__NO_RETURN void jpeg_thread(void *param)
{
    bsp_jpeg_init();
    jpeg_sema = osSemaphoreNew(1, 0, NULL);
    image_t jpeg_image;
    image_init(&jpeg_image, resolution[FRAME_SIZE][0], resolution[FRAME_SIZE][1], IMAGE_BPP_JPEG);
    for (;;) {
        extern image_t sensor_cap_image;
        extern osSemaphoreId_t process_sema;
        osSemaphoreAcquire(jpeg_sema, osWaitForever);
        // Refresh the image to GUI, so the gui_thread can display when encode thread
        // gives up the CPU.
        refresh_disp_image(&sensor_cap_image, gui_get_disp_obj(0));
        rte_error_t jpeg_result = bsp_jpeg_compress(&sensor_cap_image, &jpeg_image, 60);
        if (jpeg_result == 0) {
//                extern hal_device_t *com_debug;
//                hal_device_write_async(com_debug, jpeg_image.data, (uint32_t)jpeg_image.bpp, HAL_MAX_DELAY);
        }
        image_reuse(&sensor_cap_image);
        osSemaphoreRelease(process_sema);
    }
}