#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"
#include "shell.h"
#include "rte_include.h"
#include "hal_include.h"

osThreadId_t system_thread_id;
/**
 * @brief Used for letter-shell.
 *
 */
static Shell shell;
static char shellBuffer[512];

static size_t rte_data_out(uint8_t *data, size_t length)
{
    com_send_sync(COM_1, data, (uint16_t)length);
    return length;
}

/**
 * @brief 用户shell写
 *
 * @param data 数据
 */
unsigned short userShellWrite(char *data, unsigned short len)
{
    com_send_sync(COM_1, (uint8_t *)data, (uint16_t)len);
    return len;
}
/**
 * @brief 用户shell读
 *
 * @param data 数据
 * @return char 状态
 */
unsigned short userShellRead(char *data, unsigned short len)
{
    rte_error_t result = com_recv_async(COM_1, data, &len, 50);
    if(result == RTE_SUCCESS) {
        return len;
    }
    return 0;
}

static void running_timer(void *arg)
{
    gpio_toggle(GPIO_RUN);
}

__NO_RETURN void system_thread(void *argument)
{
    (void)argument;
    rte_init();
    com_configuration_t com_config = {0};
    extern UART_HandleTypeDef huart1;
    extern DMA_HandleTypeDef hdma_usart1_rx;
    com_config.user_arg1 = &huart1;
    com_config.user_arg2 = &hdma_usart1_rx;
    com_init(COM_1, &com_config);
    log_level_t log_level = LOG_LEVEL_INFO;
    log_control(LOG_CMD_SET_LEVEL, &log_level);
    log_control(LOG_CMD_SET_OUTPUT, rte_data_out);
    timer_configuration_t config = TIMER_CONFIG_INITIALIZER;
    timer_id_t running_timer_id = 0;
    config.repeat_period_tick = 500;
    config.timer_callback = running_timer;
    timer_create_new(rte_get_main_timergroup(), &config, &running_timer_id);
    shell.write = userShellWrite;
    shell.read = userShellRead;
    shellInit(&shell, shellBuffer, 512);
    osThreadNew(shellTask, &shell, NULL);
    RTE_LOGI("Boot at clk: %d", SystemCoreClock);
    for (;;) {
        timer_tick_handle();
        osDelay(1);
    }
}
