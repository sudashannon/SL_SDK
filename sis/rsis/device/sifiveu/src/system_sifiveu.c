#include "../inc/sifiveuxx.h"
#include "rte_include.h"
#include "cmsis_os2.h"

// static int hart_id = 0;
// static bool if_rte_initialized = false;

// int shell_getc(char *ch)
// {
//     uint32_t read_size = 1;
//     if (hal_device_read_sync("com_0", (uint8_t *)ch, &read_size, 100) != RTE_SUCCESS) {
//         return 0;
//     }
//     return 1;
// }

// size_t rte_data_out(uint8_t *data, size_t length)
// {
//     if (hal_device_write_sync("com_0", data, length, TIME_MAX_DELAY_MS) == RTE_SUCCESS) {
//         return length;
//     }
//     return 0;
// }

// #if WITH_OS == 0
// static void systimer_trap_handle(rv_csr_t mcause, rv_csr_t sp, void *arg)
// {
//     rv_csr_t cmp_value = MSYS_CLOCK_HZ_DEF / MTICK_HZ_DEF + systime_get_tick();
//     systimer_set_cmpvalue(hart_id, cmp_value);
//     if(if_rte_initialized)
//         timer_tick_handle(10);
// }
// #endif

/**
 * \brief early init function before main
 * \details
 * This function is executed right before main function.
 * For RISC-V gnu toolchain, _init function might not be called
 * by __libc_init_array function, so we defined a new function
 * to do initialization
 */
void _premain_init(void)
{
//     hart_id = csr_read(CSR_MHARTID_BASE);
//     extern void exc_entry(void);
//     /* Regist the mtvec handle */
//     __set_mirq_handle(IRQ_DIRECT_MODE, exc_entry);
// #if WITH_OS == 0
//     /* Start system timer */
//     rv_csr_t cmp_value = MSYS_CLOCK_HZ_DEF / MTICK_HZ_DEF + systime_get_tick();
//     systimer_set_cmpvalue(hart_id, cmp_value);
//     core_trap_regist(TRAP_INTERRUPT_TYPE, INT_NUM_SYSTIMER, systimer_trap_handle, NULL);
//     /* Enable system timer interrupt */
//     systimer_start();
// #endif
//     /* enable global interrupt */
//     __enable_mirq();
// #if WITH_OS == 1
//     os_kernel_initialize();
// #endif
//     /* Init the rte */
//     rte_init();
//     /* Init all hal devices */
//     hal_init();
//     /* Config log */
//     log_level_t log_level = LOG_LEVEL_INFO;
//     log_control(LOG_CMD_SET_LEVEL, &log_level);
//     log_control(LOG_CMD_SET_OUTPUT, rte_data_out);
//     if_rte_initialized = true;
}

/**
 * \brief finish function after main
 * \param [in]  status     status code return from main
 * \details
 * This function is executed right after main function.
 * For RISC-V gnu toolchain, _fini function might not be called
 * by __libc_fini_array function, so we defined a new function
 * to do initialization
 */
void _postmain_fini(int status)
{
    // /* TODO: Add your own finishing code here, called after main */
    // if_rte_initialized = false;
    // hal_deinit();
    // rte_deinit();
}

/**
 * \brief _init function called in __libc_init_array()
 * \details
 * This `__libc_init_array()` function is called during startup code,
 * user need to implement this function, otherwise when link it will
 * error init.c:(.text.__libc_init_array+0x26): undefined reference to `_init'
 * \note
 * Please use \ref _premain_init function now
 */
void _init(void)
{
    /* Don't put any code here, please use _premain_init now */
}

/**
 * \brief _fini function called in __libc_fini_array()
 * \details
 * This `__libc_fini_array()` function is called when exit main.
 * user need to implement this function, otherwise when link it will
 * error fini.c:(.text.__libc_fini_array+0x28): undefined reference to `_fini'
 * \note
 * Please use \ref _postmain_fini function now
 */
void _fini(void)
{
    /* Don't put any code here, please use _postmain_fini now */
}
