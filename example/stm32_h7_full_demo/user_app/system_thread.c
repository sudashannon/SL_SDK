#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"
#include "shell.h"
#include "rte_include.h"
#include "hal_include.h"
#include "usart.h"
#include "bsp_sensor.h"
#include "i2c.h"
#include "lvgl.h"

osThreadId_t system_thread_id;
hal_device_t *com_debug = NULL;
/**
 * @brief Used for letter-shell.
 *
 */
static Shell shell;
static char shellBuffer[512];

static size_t rte_data_out(uint8_t *data, size_t length)
{
    hal_device_write_sync(com_debug, data, (uint32_t)length, HAL_MAX_DELAY);
    return length;
}

/**
 * @brief 用户shell写
 *
 * @param data 数据
 */
short userShellWrite(char *data, unsigned short len)
{
    hal_device_write_sync(com_debug, (uint8_t *)data, (uint32_t)len, HAL_MAX_DELAY);
    return len;
}
/**
 * @brief 用户shell读
 *
 * @param data 数据
 * @return char 状态
 */
short userShellRead(char *data, unsigned short len)
{
    rte_error_t result = hal_device_read_async(com_debug, (uint8_t *)data, (uint32_t *)&len, 50);
    if(result == RTE_SUCCESS) {
        return len;
    }
    return 0;
}

static int memory(int argc, char *argv[])
{
    if (argc < 2) {
        return -1;
    }
    if (!strcmp(argv[1], "list")) {
        RTE_LOGI("System has handled %d banks", BANK_CNT);
    } else if (!strcmp(argv[1], "demon")) {
        mem_bank_t bank = atoi(argv[2]);
        memory_demon(bank);
    } else {
        return -1;
    }
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), mem, memory, memory tool to demon all handled bank);

static void running_timer(void *arg)
{
    gpio_toggle(GPIO_RUN);
}

const osThreadAttr_t sensor_tconfig = {
    .stack_size = 10240
};

__NO_RETURN void system_thread(void *argument)
{
    (void)argument;
    rte_init();
    // Init all hardware.
    com_configuration_t com_config = {0};
    com_config.huart = &huart1;
    com_config.hrx_dma = &hdma_usart1_rx;
    com_config.htx_dma = &hdma_usart1_tx;
    com_create(com_1, &com_config, &com_debug);

    log_level_t log_level = LOG_LEVEL_INFO;
    log_control(LOG_CMD_SET_LEVEL, &log_level);
    log_control(LOG_CMD_SET_OUTPUT, rte_data_out);

    timer_configuration_t config = TIMER_CONFIG_INITIALIZER;
    timer_id_t running_timer_id = 0;
    config.repeat_period_tick = 100;
    config.timer_callback = running_timer;
    timer_create_new(rte_get_main_timergroup(), &config, &running_timer_id);

    RTE_LOGI("Boot at clk: %d", SystemCoreClock);
    shell.write = userShellWrite;
    shell.read = userShellRead;
    shellInit(&shell, shellBuffer, 512);
    osThreadNew(shellTask, &shell, NULL);
    extern __NO_RETURN void gui_thread(void *param);
    extern osThreadId_t gui_tid;
    gui_tid = osThreadNew(gui_thread, NULL, NULL);
    osThreadSetPriority(gui_tid, osPriorityBelowNormal);
    extern __NO_RETURN void jpeg_thread(void *param);
    extern osThreadId_t jpeg_tid;
    jpeg_tid = osThreadNew(jpeg_thread, NULL, NULL);
    osThreadSetPriority(jpeg_tid, osPriorityHigh);
    extern __NO_RETURN void sensor_thread(void *param);
    extern osThreadId_t sensor_tid;
    sensor_tid = osThreadNew(sensor_thread, NULL, &sensor_tconfig);
    osThreadSetPriority(sensor_tid, osPriorityHigh);
    for (;;) {
        timer_tick_handle(10);
        osDelay(10);
    }
}
