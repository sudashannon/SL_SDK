#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"
#include "rte_include.h"

osThreadId_t system_thread_id;

static size_t rte_data_out(uint8_t *data, size_t length)
{
    extern UART_HandleTypeDef huart1;
    HAL_UART_Transmit(&huart1, data, (uint16_t)length, HAL_MAX_DELAY);
    return length;
}

__NO_RETURN void system_thread(void *argument)
{
    (void)argument;
    rte_init();
    log_level_t log_level = LOG_LEVEL_INFO;
    log_control(LOG_CMD_SET_LEVEL, &log_level);
    log_control(LOG_CMD_SET_OUTPUT, rte_data_out);
    RTE_LOGI("Boot at clk: %d", SystemCoreClock);
    for (;;) {
        timer_tick_handle();
        osDelay(1);
    }
}
