/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <bsp.h>
#include <sysctl.h>
#include "fpioa.h"
#include "gpio.h"
#include "rsis_core.h"
#include "rsis_trap.h"

/*****************************HARDWARE-PIN*********************************/
// 硬件IO口，与原理图对应
#define PIN_LED_0             (0)
#define PIN_LED_1             (17)

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define LED0_GPIONUM          (0)
#define LED1_GPIONUM          (1)

/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_LED0             (FUNC_GPIO0 + LED0_GPIONUM)
#define FUNC_LED1             (FUNC_GPIO0 + LED1_GPIONUM)

typedef struct {
    uint64_t ra;
    uint64_t sp;
    uint64_t t0;
    uint64_t t1;
    uint64_t t2;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
    uint64_t a0;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t a7;
} arch_ctx_t;

arch_ctx_t main_task = {0};
arch_ctx_t next_task = {0};

void arch_switch_to(arch_ctx_t *from, arch_ctx_t *to)
{
    asm volatile("sd ra, 0(a0)");
    asm volatile("sd sp, 8(a0)");
    asm volatile("sd t0, 16(a0)");
    asm volatile("sd t1, 24(a0)");
    asm volatile("sd t2, 32(a0)");
    asm volatile("sd t3, 40(a0)");
    asm volatile("sd t4, 48(a0)");
    asm volatile("sd t5, 56(a0)");
    asm volatile("sd t6, 64(a0)");
    asm volatile("sd a0, 72(a0)");
    asm volatile("sd a1, 80(a0)");
    asm volatile("sd a2, 88(a0)");
    asm volatile("sd a3, 96(a0)");
    asm volatile("sd a4, 104(a0)");
    asm volatile("sd a5, 112(a0)");
    asm volatile("sd a6, 120(a0)");

    asm volatile("ld ra, 0(a1)");
    asm volatile("ld sp, 8(a1)");
    asm volatile("ld t0, 16(a1)");
    asm volatile("ld t1, 24(a1)");
    asm volatile("ld t2, 32(a1)");
    asm volatile("ld t3, 40(a1)");
    asm volatile("ld t4, 48(a1)");
    asm volatile("ld t5, 56(a1)");
    asm volatile("ld t6, 64(a1)");
    asm volatile("ld a0, 72(a1)");
    asm volatile("ld a2, 88(a1)");
    asm volatile("ld a3, 96(a1)");
    asm volatile("ld a4, 104(a1)");
    asm volatile("ld a5, 112(a1)");
    asm volatile("ld a6, 120(a1)");
    asm volatile("ld a1, 80(a1)");
}

int core1_function(void *ctx)
{
    //uint64_t core = current_coreid();
#if DEBUG_ON_QEMU == 0
    //printf("This program is running on %ld core.\r\n", core);
#else
    core = core;
#endif
    while (1) {
#if DEBUG_ON_QEMU == 0
        sleep(1);
        //printf("continue running on %ld...\r\n", core);
#endif
    }
}

uint64_t next_task_stack[1024 / sizeof(uint64_t)] = {0};

void next_task_body(void *arg)
{
    printf("task %p call switch\r\n", &main_task);
    printf("formal ra %016lx sp %016lx\r\n", main_task.ra, main_task.sp);
    uint64_t core = current_coreid();
#if DEBUG_ON_QEMU == 0
    printf("task %p is running on %ld core.\r\n", &next_task, core);
    printf("now ra %016lx sp %016lx\r\n", next_task.ra, next_task.sp);
    gpio_pin_value_t *value = (gpio_pin_value_t *)arg;
#else
    core = core;
#endif
    while (1) {
#if DEBUG_ON_QEMU == 0
        gpio_set_pin(LED0_GPIONUM, *value = !*value);
        sleep(1);
#endif
        arch_switch_to(&next_task, &main_task);
    }
}

int main(void)
{
#if DEBUG_ON_QEMU == 0
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    fpioa_set_function(PIN_LED_0, FUNC_LED0);
    gpio_init();
    gpio_set_drive_mode(LED0_GPIONUM, GPIO_DM_OUTPUT);
    gpio_pin_value_t value = GPIO_PV_HIGH;
#endif
    uint64_t core = current_coreid();
#if DEBUG_ON_QEMU == 0
    printf("This program is running on %ld core.\r\n", core);
#else
    core = core;
#endif
    register_core1(core1_function, NULL);
    next_task.sp = (uint64_t)(&next_task_stack[1024 / sizeof(uint64_t) - 1]);
    next_task.ra = (uint64_t)next_task_body;
    next_task.a0 = (uint64_t)&value;
    while(1) {
#if DEBUG_ON_QEMU == 0
        printf("task %p continue running on %ld...\r\n", &main_task, core);
        // gpio_set_pin(LED0_GPIONUM, value);
        sleep(1);
#endif
        arch_switch_to(&main_task, &next_task);
    }
    return 0;
}
