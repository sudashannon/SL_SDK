#include "stm32h7xx.h"
#include "cmsis_os2.h"
#include "rte_include.h"
#include "hal_include.h"
#include "bsp_sensor.h"
#include "usart.h"

osThreadId_t system_thread_id;

int shell_getc(char *ch)
{
    uint32_t read_size = 1;
    if (hal_device_read_async("com_0", (uint8_t *)ch, &read_size, 100) != RTE_SUCCESS) {
        return 0;
    }
    return 1;
}

size_t rte_data_out(uint8_t *data, size_t length)
{
    HAL_UART_Transmit(&huart1, data, length, HAL_MAX_DELAY);
    return length;
}

static void running_timer(void *arg)
{
    gpio_toggle(GPIO_RUN);
}

__NO_RETURN void system_thread(void *argument)
{
    (void)argument;
    /* Init the rte */
    rte_init();
    log_level_t log_level = LOG_LEVEL_INFO;
    log_control(LOG_CMD_SET_LEVEL, &log_level);
    log_control(LOG_CMD_SET_OUTPUT, rte_data_out);
    /* Init all hal devices */
    hal_init();

    RTE_LOGI("Boot at clk: %d", SystemCoreClock);
    timer_configuration_t config = TIMER_CONFIG_INITIALIZER;
    timer_id_t running_timer_id = 0;
    config.repeat_period_ms = 100;
    config.timer_callback = running_timer;
    timer_create_new(rte_get_main_timergroup(), &config, &running_timer_id);
    osThreadAttr_t shell_tconfig = {
        .name = "shell_task",
        .stack_size = 10240,
    };
    osThreadId_t shell_tid = osThreadNew(shell_task, NULL, &shell_tconfig);
    extern __NO_RETURN void gui_thread(void *param);
    extern osThreadId_t gui_tid;
    gui_tid = osThreadNew(gui_thread, NULL, NULL);
    for (;;) {
        timer_tick_handle(10);
        osDelay(10);
    }
}
