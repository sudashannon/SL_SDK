#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"
#include "rte_include.h"
#include "hal_include.h"

osThreadId_t system_thread_id;

static size_t rte_data_out(uint8_t *data, size_t length)
{
    extern UART_HandleTypeDef huart1;
    HAL_UART_Transmit(&huart1, data, (uint16_t)length, HAL_MAX_DELAY);
    return length;
}

static void running_timer(void *arg)
{
    gpio_toggle(GPIO_RUN);
}

__NO_RETURN void system_thread(void *argument)
{
    (void)argument;
    rte_init();
    log_level_t log_level = LOG_LEVEL_INFO;
    log_control(LOG_CMD_SET_LEVEL, &log_level);
    log_control(LOG_CMD_SET_OUTPUT, rte_data_out);
    timer_configuration_t config = TIMER_CONFIG_INITIALIZER;
    timer_id_t running_timer_id = 0;
    config.repeat_period_tick = 500;
    config.timer_callback = running_timer;
    timer_create_new(rte_get_main_timergroup(), &config, &running_timer_id);
    RTE_LOGI("Boot at clk: %d", SystemCoreClock);
    for (;;) {
        timer_tick_handle();
        osDelay(1);
    }
}
