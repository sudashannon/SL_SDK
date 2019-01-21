/**
 * \file            esp_ll_stm32.c
 * \brief           Generic STM32 driver, included in various STM32 driver variants
 */

/*
 * Copyright (c) 2018 Tilen Majerle
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of ESP-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 */

/*
 * How it works
 *
 * On first call to \ref esp_ll_init, new thread is created and processed in usart_ll_thread function.
 * USART is configured in RX DMA mode and any incoming bytes are processed inside thread function.
 * DMA and USART implement interrupt handlers to notify main thread about new data ready to send to upper layer.
 *
 * More about UART + RX DMA: https://github.com/MaJerle/STM32_USART_DMA_RX
 *
 * \ref ESP_CFG_INPUT_USE_PROCESS must be enabled in `esp_config.h` to use this driver.
 */
#include "esp/esp.h"
#include "esp/esp_mem.h"
#include "esp/esp_input.h"
#include "system/esp_ll.h"
#include "BSP_Com.h"
#include "Board_ESP8266.h"
#if !__DOXYGEN__

#if !ESP_CFG_INPUT_USE_PROCESS
#error "ESP_CFG_INPUT_USE_PROCESS must be enabled in `esp_config.h` to use this driver."
#endif /* ESP_CFG_INPUT_USE_PROCESS */

#if !defined(ESP_USART_DMA_RX_BUFF_SIZE)
#define ESP_USART_DMA_RX_BUFF_SIZE      0x1000
#endif /* !defined(ESP_USART_DMA_RX_BUFF_SIZE) */

#if !defined(ESP_MEM_SIZE)
#define ESP_MEM_SIZE                    0x1000
#endif /* !defined(ESP_MEM_SIZE) */

/* USART memory */
static uint8_t      is_running, initialized;
/* USART thread */
static uint64_t usart_ll_thread_stk[1024 / 8] = {0};
static osThreadId_t usart_ll_thread_id;
const osThreadAttr_t UsartLLThreadControl = {
	.stack_mem = &usart_ll_thread_stk[0],
  .stack_size =  sizeof(usart_ll_thread_stk) , 
	.priority = osPriorityNormal,
};
/**
 * \brief           USART data processing
 */
static void
usart_ll_thread(void *arg) {
		RTE_MessageQuene_t *WIFIQuene;
		WIFIQuene = &(BSP_COM_ReturnQue(COM_WIFI)->ComQuene);
		uint16_t BufferLenth = 0;
		uint8_t *usart_mem = (uint8_t *)RTE_MEM_Alloc0(MEM_RTE,ESP_USART_DMA_RX_BUFF_SIZE);
    while (1) {
			
			if(is_running)
			{
				if(RTE_MessageQuene_Out(WIFIQuene,(uint8_t *)usart_mem,&BufferLenth) == MSG_NO_ERR)
				{
					/* Wait for the Thread Flag from DMA or USART */
					esp_input_process(usart_mem, BufferLenth);
				}
				memset(usart_mem,0,ESP_USART_DMA_RX_BUFF_SIZE);
				BufferLenth = 0;
			}
			osDelay(50);
    }
}

/**
 * \brief           Send data to ESP device
 * \param[in]       data: Pointer to data to send
 * \param[in]       len: Number of bytes to send
 * \return          Number of bytes sent
 */
static size_t
send_data(const void* data, size_t len) {
		BSP_COM_SendArray(COM_WIFI,(uint8_t *)data,len);
    return len;
}

/**
 * \brief           Callback function called from initialization process
 * \note            This function may be called multiple times if AT baudrate is changed from application
 * \param[in,out]   ll: Pointer to \ref esp_ll_t structure to fill data for communication functions
 * \param[in]       baudrate: Baudrate to use on AT port
 * \return          Member of \ref espr_t enumeration
 */
espr_t
esp_ll_init(esp_ll_t* ll) {
    if (!initialized) {
				Board_ESP8266_Init();
				uint8_t *memory = (uint8_t *)RTE_MEM_Alloc0(MEM_RTE,ESP_MEM_SIZE);
				esp_mem_region_t mem_regions[] = {
        { memory, ESP_MEM_SIZE }
				};
        ll->send_fn = send_data;                /* Set callback function to send data */

        esp_mem_assignmemory(mem_regions, ESP_ARRAYSIZE(mem_regions));  /* Assign memory for allocations */
				
				BSP_COM_Init(COM_WIFI,ll->uart.baudrate);          /* Initialize UART for communication */
				
				if (usart_ll_thread_id == NULL) {
					usart_ll_thread_id = osThreadNew(usart_ll_thread, NULL, &UsartLLThreadControl); 
				}
				Board_ESP8266_HardReset();
				is_running = 1;
		}
    initialized = 1;
    return espOK;
}

/**
 * \brief           Callback function to de-init low-level communication part
 * \param[in,out]   ll: Pointer to \ref esp_ll_t structure to fill data for communication functions
 * \return          \ref espOK on success, member of \ref espr_t enumeration otherwise
 */
espr_t
esp_ll_deinit(esp_ll_t* ll) {
    if (usart_ll_thread_id != NULL) {
        osThreadId_t tmp = usart_ll_thread_id;
        usart_ll_thread_id = NULL;
        osThreadTerminate(tmp);
    }
    initialized = 0;
    return espOK;
}


#endif /* !__DOXYGEN__ */
