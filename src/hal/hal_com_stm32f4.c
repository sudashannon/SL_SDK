/**
 * @file hal_com.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-07-28
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "../../inc/hal/hal_com.h"

typedef struct {
    const uint8_t af;
    const uint8_t source;
    const uint16_t pin;
    const uint32_t clock;
    GPIO_TypeDef *port;
} bsp_com_pin_t;

typedef struct {
    uint16_t capacity;
    const uint32_t clock;
    uint8_t *buffer;
	DMA_Stream_TypeDef *dma_stream;
	uint32_t dma_channel;
} bsp_com_dma_t;

typedef struct {
    // Configuration section.
    com_name_t name;
    const uint8_t interrupt;
    const uint8_t priority;
    USART_TypeDef *uart;
    const uint32_t clock;
    bsp_com_pin_t tx_pin;
    bsp_com_pin_t rx_pin;
    bsp_com_dma_t tx_dma;
    bsp_com_dma_t rx_dma;
    // Resource section.
    union {
        void *ringbuffer;
        void *vector;
    } buffer;
    rte_mutex_t mutex;
    osSemaphoreId_t sema;
} bsp_com_handle_t;

static bsp_com_handle_t com_control_handle[COM_N] = {
    {
		.name = COM_1,
        .interrupt = USART1_IRQn,
        .priority = 0x09,
        .clock = RCC_APB2Periph_USART1,
        .uart = USART1,
        .tx_pin = {
            .af = GPIO_AF_USART1,
            .source = GPIO_PinSource9,
            .port = GPIOA,
            .pin = GPIO_Pin_9,
            .clock = RCC_AHB1Periph_GPIOA,
        },
        .rx_pin = {
            .af = GPIO_AF_USART1,
            .source = GPIO_PinSource10,
            .port = GPIOA,
            .pin = GPIO_Pin_10,
            .clock = RCC_AHB1Periph_GPIOA,
        },
        .rx_dma = {
            .clock = RCC_AHB1Periph_DMA2,
            .dma_stream = DMA2_Stream2,
            .dma_channel = DMA_Channel_4,
            .buffer = NULL,
            .capacity = 128,
        },
	},
};


rte_error_t com_init(com_name_t com_name, com_configuration_t *config)
{
    if (RTE_UNLIKELY(config == NULL))
        return RTE_ERR_PARAM;
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};

    // Config for com peripheral
    // 1. clock
    if ((com_control_handle[com_name].uart == USART1) ||
        (com_control_handle[com_name].uart == USART6)) {
        RCC_APB2PeriphClockCmd(com_control_handle[com_name].clock, ENABLE);
    } else {
        RCC_APB1PeriphClockCmd(com_control_handle[com_name].clock, ENABLE);
    }
    // 2. GPIO configuration
    if (com_control_handle[com_name].tx_pin.port != NULL) {
        RCC_AHB1PeriphClockCmd(com_control_handle[com_name].tx_pin.clock, ENABLE);
        GPIO_PinAFConfig(com_control_handle[com_name].tx_pin.port,
                        com_control_handle[com_name].tx_pin.source,
                        com_control_handle[com_name].tx_pin.af);
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Pin = com_control_handle[com_name].tx_pin.pin;
        GPIO_Init(com_control_handle[com_name].tx_pin.port, &GPIO_InitStructure);
    }
    if (com_control_handle[com_name].rx_pin.port != NULL) {
        RCC_AHB1PeriphClockCmd(com_control_handle[com_name].rx_pin.clock, ENABLE);
        GPIO_PinAFConfig(com_control_handle[com_name].rx_pin.port,
                        com_control_handle[com_name].rx_pin.source,
                        com_control_handle[com_name].rx_pin.af);
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Pin =  com_control_handle[com_name].rx_pin.pin;
        GPIO_Init(com_control_handle[com_name].rx_pin.port, &GPIO_InitStructure);
    }
    // 3. basic configuration
    USART_OverSampling8Cmd(com_control_handle[com_name].uart, ENABLE);
    USART_InitStructure.USART_BaudRate = config->boundrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(com_control_handle[com_name].uart, &USART_InitStructure);
    USART_ClearFlag(com_control_handle[com_name].uart, USART_FLAG_TC);
    // Create mutex
    com_control_handle[com_name].mutex.mutex = osMutexNew(NULL);
    com_control_handle[com_name].mutex.lock = rte_mutex_lock;
    com_control_handle[com_name].mutex.unlock = rte_mutex_unlock;
    com_control_handle[com_name].mutex.trylock = NULL;
    // Check if use dma
    if (com_control_handle[com_name].rx_dma.dma_stream != NULL) {
        com_control_handle[com_name].rx_dma.buffer = memory_calloc(BANK_DMA, com_control_handle[com_name].rx_dma.capacity);
        com_control_handle[com_name].sema = osSemaphoreNew(com_control_handle[com_name].rx_dma.capacity, 0, NULL);
        // Use ringbuffer as recv buffer when enable dma.
        rte_error_t result = ds_ringbuffer_create(com_control_handle[com_name].rx_dma.capacity,
                                                &com_control_handle[com_name].mutex,
                                                &com_control_handle[com_name].buffer.ringbuffer);
        RTE_ASSERT(result == RTE_SUCCESS);
        RCC_AHB1PeriphClockCmd(com_control_handle[com_name].rx_dma.clock, ENABLE);
        DMA_DeInit(com_control_handle[com_name].rx_dma.dma_stream);
        DMA_InitStructure.DMA_Channel = com_control_handle[com_name].rx_dma.dma_channel;
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(com_control_handle[com_name].uart->DR);
        DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)com_control_handle[com_name].rx_dma.buffer;
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
        DMA_InitStructure.DMA_BufferSize = com_control_handle[com_name].rx_dma.capacity;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
        DMA_Init(com_control_handle[com_name].rx_dma.dma_stream, &DMA_InitStructure);
        USART_ITConfig(com_control_handle[com_name].uart, USART_IT_IDLE, ENABLE);
    } else {
        // Use vector as recv buffer when no dma is used.
        vector_configuration_t config = {0};
        config.capacity = com_control_handle[com_name].rx_dma.capacity;
        config.element_size = sizeof(uint8_t);
        config.free_cb = NULL;
        config.mutex = &com_control_handle[com_name].mutex;
        config.if_deep_copy = true;
        config.if_expand = false;
        rte_error_t result = ds_vector_create(&config, &com_control_handle[com_name].buffer.vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        USART_ITConfig(com_control_handle[com_name].uart, USART_IT_RXNE, ENABLE);
    }
    // Nvic configuration
    NVIC_InitStructure.NVIC_IRQChannel = com_control_handle[com_name].interrupt;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = com_control_handle[com_name].priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // UART enable
    USART_Cmd(com_control_handle[com_name].uart, ENABLE);
    // DMA enable
    if (com_control_handle[com_name].rx_dma.dma_stream != NULL) {
        DMA_Cmd(com_control_handle[com_name].rx_dma.dma_stream, ENABLE);
        USART_DMACmd(com_control_handle[com_name].uart, USART_DMAReq_Rx, ENABLE);
    }
    return RTE_SUCCESS;
}

rte_error_t com_send_sync(com_name_t com_name, uint8_t *data, uint16_t size)
{
    if (RTE_UNLIKELY(data == NULL))
        return RTE_ERR_PARAM;
    if (size == 0)
        return RTE_SUCCESS;
    for (uint16_t i = 0; i < size; i++) {
        while (USART_GetFlagStatus(
                com_control_handle[com_name].uart,
                USART_FLAG_TXE) == RESET);
        USART_SendData(com_control_handle[com_name].uart, data[i]);
    }
    return RTE_SUCCESS;
}

rte_error_t com_recv_async(com_name_t com_name, uint8_t *buffer, uint16_t *size, uint32_t timeout_ms)
{
    if(com_control_handle[com_name].rx_dma.dma_stream != NULL) {
        osSemaphoreAcquire(com_control_handle[com_name].sema, timeout_ms);
        return ds_ringbuffer_read(com_control_handle[com_name].buffer.ringbuffer, buffer, size);
    } else {
        osDelay(timeout_ms);
        if(*size == 1) {
            return ds_vector_pop(com_control_handle[com_name].buffer.vector, buffer);
        }
        for(uint16_t i = 0; i < *size; i++) {
            if(ds_vector_pop(com_control_handle[com_name].buffer.vector, buffer + i) == RTE_SUCCESS) {
                continue;
            } else {
                *size = i;
                return (*size > 0) ? RTE_SUCCESS : RTE_ERR_UNDEFINE;
            }
        }
    }
    *size = 0;
    return RTE_ERR_UNDEFINE;
}

static uint32_t com_get_dma_transfer(com_name_t com_name)
{
	if(com_control_handle[com_name].rx_dma.dma_stream == DMA2_Stream0||com_control_handle[com_name].rx_dma.dma_stream == DMA1_Stream0)
		return DMA_FLAG_TCIF0;
	else if(com_control_handle[com_name].rx_dma.dma_stream == DMA2_Stream1||com_control_handle[com_name].rx_dma.dma_stream == DMA1_Stream1)
		return DMA_FLAG_TCIF1;
	else if(com_control_handle[com_name].rx_dma.dma_stream == DMA2_Stream2||com_control_handle[com_name].rx_dma.dma_stream == DMA1_Stream2)
		return DMA_FLAG_TCIF2;
	else if(com_control_handle[com_name].rx_dma.dma_stream == DMA2_Stream3||com_control_handle[com_name].rx_dma.dma_stream == DMA1_Stream3)
		return DMA_FLAG_TCIF3;
	else if(com_control_handle[com_name].rx_dma.dma_stream == DMA2_Stream4||com_control_handle[com_name].rx_dma.dma_stream == DMA1_Stream4)
		return DMA_FLAG_TCIF4;
	else if(com_control_handle[com_name].rx_dma.dma_stream == DMA2_Stream5||com_control_handle[com_name].rx_dma.dma_stream == DMA1_Stream5)
		return DMA_FLAG_TCIF5;
	else if(com_control_handle[com_name].rx_dma.dma_stream == DMA2_Stream6||com_control_handle[com_name].rx_dma.dma_stream == DMA1_Stream6)
		return DMA_FLAG_TCIF6;
	else if(com_control_handle[com_name].rx_dma.dma_stream == DMA2_Stream7||com_control_handle[com_name].rx_dma.dma_stream == DMA1_Stream7)
		return DMA_FLAG_TCIF7;
	return 0;
}

static void com_recv_callback(com_name_t com_name)
{
    if (com_control_handle[com_name].rx_dma.dma_stream != NULL) {
        if(USART_GetITStatus(com_control_handle[com_name].uart, USART_IT_IDLE) != RESET) {
            uint16_t recv_length = com_control_handle[com_name].rx_dma.capacity -
                                    DMA_GetCurrDataCounter(com_control_handle[com_name].rx_dma.dma_stream);
            ds_ringbuffer_write(com_control_handle[com_name].buffer.ringbuffer,
                                com_control_handle[com_name].rx_dma.buffer,
                                recv_length);
            DMA_Cmd(com_control_handle[com_name].rx_dma.dma_stream, DISABLE);
            com_control_handle[com_name].rx_dma.dma_stream->NDTR = com_control_handle[com_name].rx_dma.capacity;
            DMA_ClearFlag(com_control_handle[com_name].rx_dma.dma_stream, com_get_dma_transfer(com_name));
            DMA_SetCurrDataCounter(com_control_handle[com_name].rx_dma.dma_stream, com_control_handle[com_name].rx_dma.capacity);
            DMA_Cmd(com_control_handle[com_name].rx_dma.dma_stream, ENABLE);
            com_control_handle[com_name].uart->SR;
            com_control_handle[com_name].uart->DR;
            osSemaphoreRelease(com_control_handle[com_name].sema);
            USART_ClearITPendingBit(com_control_handle[com_name].uart, USART_IT_IDLE);
        }
    } else {
        if(USART_GetITStatus(com_control_handle[com_name].uart, USART_IT_RXNE) != RESET) {
            uint8_t data = 0;
            com_control_handle[com_name].uart->SR;
            data = (uint8_t)com_control_handle[com_name].uart->DR;
            ds_vector_push(com_control_handle[com_name].buffer.vector, &data);
            USART_ClearITPendingBit(com_control_handle[com_name].uart, USART_IT_RXNE);
        }
    }
}

void USART1_IRQHandler(void)
{
    com_recv_callback(COM_1);
}
