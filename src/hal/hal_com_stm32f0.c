/**
 * @file bsp_com.c
 * @author Leon Shan (813475603@qq.com)
 * @brief 
 * @version 1.0.0
 * @date 2020-10-08
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "../../inc/bsp_com.h"

typedef struct {
	USART_TypeDef* usart;
	GPIO_TypeDef* port;
    uint8_t af;
	uint8_t tx_pin_source;
	uint8_t rx_pin_source;
    uint16_t tx_pin;
    uint16_t rx_pin;
	uint32_t gpio_clock;
	uint16_t mode;
	DMA_Channel_TypeDef* tx_channel;
	DMA_Channel_TypeDef* rx_channel;
    bool if_enable_dma;
	uint8_t *buffer_ptr;
	uint16_t buffer_capability;
    queue_t *queue;
} com_handle_t;

typedef struct {
    uint16_t length;
    uint8_t *data;
} com_data_t;

#define USART_TXEMPTY(USARTx)               ((USARTx->ISR & USART_FLAG_TXE))
#define USART_WAIT(USARTx)                  while (!USART_TXEMPTY(USARTx))
#define USART_WRITE_DATA(USARTx, data)      ((USARTx)->TDR = (data))

static com_handle_t com_handle_instance[COM_N]=
{
    // Name  ,
    {
        USART1, GPIOA, GPIO_AF_1,
        GPIO_PinSource9 , GPIO_PinSource10 ,
        GPIO_Pin_9 , GPIO_Pin_10 , RCC_AHBPeriph_GPIOA,
        USART_Mode_Rx | USART_Mode_Tx,
        DMA1_Channel2, DMA1_Channel3,
        .buffer_capability = 128,
        .if_enable_dma = true,
    }
};

static void com_ip_init(com_name_t com_name,uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    if(com_handle_instance[com_name].usart == USART1)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
    else if(com_handle_instance[com_name].usart == USART2)
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
    USART_DeInit(com_handle_instance[com_name].usart);

    RCC_AHBPeriphClockCmd(com_handle_instance[com_name].gpio_clock, ENABLE);
    GPIO_PinAFConfig(com_handle_instance[com_name].port, com_handle_instance[com_name].tx_pin_source, com_handle_instance[com_name].af); 
	GPIO_PinAFConfig(com_handle_instance[com_name].port, com_handle_instance[com_name].rx_pin_source, com_handle_instance[com_name].af); 
    GPIO_InitStructure.GPIO_Pin = com_handle_instance[com_name].tx_pin | com_handle_instance[com_name].rx_pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(com_handle_instance[com_name].port, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = com_handle_instance[com_name].mode;
    USART_Init(com_handle_instance[com_name].usart, &USART_InitStructure);
}

static void com_interupt_init(com_name_t com_name)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    if(com_handle_instance[com_name].if_enable_dma)
        USART_ITConfig(com_handle_instance[com_name].usart, USART_IT_IDLE, ENABLE);
    else
        USART_ITConfig(com_handle_instance[com_name].usart, USART_IT_RXNE, ENABLE);
    if (com_handle_instance[com_name].usart == USART1) {
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
    } else if(com_handle_instance[com_name].usart == USART2) {
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
    }
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void com_dma_init(com_name_t com_name)
{
    DMA_InitTypeDef DMA_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    DMA_DeInit(com_handle_instance[com_name].rx_channel);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&com_handle_instance[com_name].usart->RDR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)com_handle_instance[com_name].buffer_ptr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = com_handle_instance[com_name].buffer_capability;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(com_handle_instance[com_name].rx_channel, &DMA_InitStructure);

	DMA_RemapConfig(DMA1, DMA1_CH3_USART1_RX);
    DMA_Cmd(com_handle_instance[com_name].rx_channel, ENABLE);
    USART_DMACmd(com_handle_instance[com_name].usart, USART_DMAReq_Rx, ENABLE);
}

int8_t com_init(com_name_t com_name, uint32_t baudrate) {
    int8_t retval = -1;
    if(com_handle_instance[com_name].if_enable_dma) {
        com_handle_instance[com_name].buffer_ptr = memory_calloc(BANK_DEFAULT, com_handle_instance[com_name].buffer_capability);
        LOG_ASSERT(LOG_STR(com_name), com_handle_instance[com_name].buffer_ptr);
        retval = queue_create(&com_handle_instance[com_name].queue, 16, sizeof(com_data_t));
    } else {
        retval = queue_create(&com_handle_instance[com_name].queue, com_handle_instance[com_name].buffer_capability, sizeof(uint8_t));
    }
    if(retval != 0) {
        memory_free(BANK_DEFAULT, com_handle_instance[com_name].buffer_ptr);
        LOG_ERROR(LOG_STR(com_name), "queue init error!");
        return retval;
    }
    com_ip_init(com_name,baudrate);
    com_interupt_init(com_name);
    if(com_handle_instance[com_name].if_enable_dma)
        com_dma_init(com_name);
    USART_ClearFlag(com_handle_instance[com_name].usart, USART_FLAG_TC);
    USART_ClearFlag(com_handle_instance[com_name].usart, USART_FLAG_IDLE);
    USART_Cmd(com_handle_instance[com_name].usart, ENABLE);
    return retval;
}

int8_t com_deinit(com_name_t com_name)
{
    int8_t retval = -1;
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_Cmd(com_handle_instance[com_name].usart, DISABLE);
    if(com_handle_instance[com_name].if_enable_dma) {
        DMA_Cmd(com_handle_instance[com_name].rx_channel, DISABLE);
        USART_DMACmd(com_handle_instance[com_name].usart, USART_DMAReq_Rx, DISABLE);
    }
    if(com_handle_instance[com_name].if_enable_dma)
        USART_ITConfig(com_handle_instance[com_name].usart, USART_IT_IDLE, DISABLE);
    else
        USART_ITConfig(com_handle_instance[com_name].usart, USART_IT_RXNE, DISABLE);
    if (com_handle_instance[com_name].usart == USART1) {
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
    } else if(com_handle_instance[com_name].usart == USART2) {
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
    }
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
    if(com_handle_instance[com_name].usart == USART1)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,DISABLE);
    else if(com_handle_instance[com_name].usart == USART2)
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,DISABLE);
    USART_DeInit(com_handle_instance[com_name].usart);
    if(com_handle_instance[com_name].if_enable_dma) {
        retval = queue_destory(&com_handle_instance[com_name].queue);
        memory_free(BANK_DEFAULT, com_handle_instance[com_name].buffer_ptr);
    } else {
        retval = queue_destory(&com_handle_instance[com_name].queue);
    }
    return retval;
}

int8_t com_send(com_name_t com_name, uint8_t *data, uint16_t size)
{
    /* Go through entire data  */
    while (size--) {
        /* Wait to be ready, buffer empty */
        USART_WAIT(com_handle_instance[com_name].usart);
        /* Send data */
        USART_WRITE_DATA(com_handle_instance[com_name].usart, (uint16_t)(*data++));
        /* Wait to be ready, buffer empty */
        USART_WAIT(com_handle_instance[com_name].usart);
    }
    return 0;
}

int8_t com_recv(com_name_t com_name, uint8_t *buffer, uint16_t *size)
{
    if(com_handle_instance[com_name].if_enable_dma) {
        com_data_t data;
        if(queue_pop(com_handle_instance[com_name].queue, &data) == 0) {
            if(data.length > *size) {
                LOG_WARN(LOG_STR(com_name), "recv buffer is too small!");
            } else {
                *size = data.length;
            }
            memcpy(buffer, data.data, *size);
            memory_free(BANK_DEFAULT, data.data);
            return 0;
        }
    } else {
        if(*size == 1)
            return queue_pop(com_handle_instance[com_name].queue, buffer);
        uint8_t i = 0;
        for(i = 0; i < *size; i++) {
            if(queue_pop(com_handle_instance[com_name].queue, buffer + i) == 0) {
                continue;
            } else {
                *size = i;
                return (*size > 0) ? 0 : -1;
            }
        }
    }
    *size = 0;
    return -1;
}

static void com_dma_reset(com_name_t com_name, uint16_t data_length)
{
    com_data_t data;
    data.data = memory_alloc(BANK_DEFAULT, data_length);
    if(!data.data)
        goto next;
    data.length = data_length;
    memcpy(data.data, com_handle_instance[com_name].buffer_ptr, data_length);
    int8_t result = queue_push(com_handle_instance[com_name].queue, &data);
next:
    DMA_Cmd(com_handle_instance[com_name].rx_channel, DISABLE);
    com_handle_instance[com_name].rx_channel->CMAR = (uint32_t)com_handle_instance[com_name].buffer_ptr;
    com_handle_instance[com_name].rx_channel->CNDTR = com_handle_instance[com_name].buffer_capability;
    DMA_Cmd(com_handle_instance[com_name].rx_channel, ENABLE);
}

static void com_recv_callback(com_name_t com_name)
{
    if(com_handle_instance[com_name].if_enable_dma) {
        com_handle_instance[com_name].usart->ISR;
        com_handle_instance[com_name].usart->RDR;
        uint16_t recv_length = com_handle_instance[com_name].buffer_capability - com_handle_instance[com_name].rx_channel->CNDTR;
        com_dma_reset(com_name, recv_length);
        USART_ClearITPendingBit(com_handle_instance[com_name].usart, USART_IT_IDLE);
    } else {
        uint8_t data = 0;
        com_handle_instance[com_name].usart->ISR;
        data = com_handle_instance[com_name].usart->RDR;
        queue_push(com_handle_instance[com_name].queue, &data);
        USART_ClearITPendingBit(com_handle_instance[com_name].usart, USART_IT_RXNE);
    }
}

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET ||
        USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        com_recv_callback(COM_1);
    }
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET ||
        USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        com_recv_callback(COM_2);
    }
}