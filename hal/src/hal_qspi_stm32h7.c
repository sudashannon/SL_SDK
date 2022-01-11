/**
 * @file hal_qspi_stm32h7.c
 * @author Leon Shan (813475603@qq.qspi)
 * @brief
 * @version 1.0.0
 * @date 2021-08-07
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../inc/hal_qspi.h"
#include "cmsis_os2.h"

typedef struct {
    MDMA_HandleTypeDef *mdma_handle;
    hal_device_t device;
} qspi_device_t;

static qspi_device_t qspi_control_handle[qspi_N];

static rte_error_t qspi_send(hal_device_t *device, uint8_t *data, uint32_t size, uint32_t timeout_ms)
{
    RTE_UNUSED(size);
    HAL_StatusTypeDef result = HAL_ERROR;
    if (size == 0)
        return RTE_SUCCESS;
    result = HAL_QSPI_Transmit(
                qspi_control_handle[device->device_id].device.fd,
                data, timeout_ms);
    if (result == HAL_OK)
        return RTE_SUCCESS;
    else if(result == HAL_TIMEOUT)
        return RTE_ERR_TIMEOUT;
    else
        return RTE_ERR_UNDEFINE;
}

static rte_error_t qspi_recv(hal_device_t *device, uint8_t *buffer, uint32_t *size, uint32_t timeout_ms)
{
    RTE_UNUSED(size);
    HAL_StatusTypeDef result = HAL_ERROR;
    qspi_name_t qspi_name = device->device_id;
    result = HAL_QSPI_Receive(
                qspi_control_handle[qspi_name].device.fd, buffer, timeout_ms);
    if (result == HAL_OK) {
        return RTE_SUCCESS;
    } else if(result == HAL_TIMEOUT) {
        *size = 0;
        return RTE_ERR_TIMEOUT;
    } else {
        *size = 0;
        return RTE_ERR_UNDEFINE;
    }
}

static rte_error_t qspi_send_async(hal_device_t *device, uint8_t *data, uint32_t size, uint32_t timeout_ms)
{
    RTE_UNUSED(size);
    qspi_name_t qspi_name = device->device_id;
    if (qspi_control_handle[qspi_name].mdma_handle != NULL) {
        // uint8_t *tx_buffer = memory_alloc(BANK_DMA, size);
        // if (tx_buffer == NULL)
        //     return RTE_ERR_NO_RSRC;
        // memcpy(tx_buffer, data, size);
        // HAL_RAM_CLEAN_PRE_SEND(tx_buffer, size);
        // HAL_UART_Transmit_DMA(qspi_control_handle[qspi_name].device.fd,
        //                     tx_buffer,
        //                     size);
        // if (osSemaphoreAcquire(qspi_control_handle[qspi_name].tx_sema, timeout_ms) == osOK) {
        //     memory_free(BANK_DMA, tx_buffer);
        //     return RTE_SUCCESS;
        // }
        // HAL_UART_AbortTransmit_IT(qspi_control_handle[qspi_name].device.fd);
        // memory_free(BANK_DMA, tx_buffer);
        return RTE_ERR_TIMEOUT;
    } else {
        HAL_QSPI_Transmit_IT(qspi_control_handle[qspi_name].device.fd, data);
        if (osSemaphoreAcquire(qspi_control_handle[qspi_name].device.tx_sema, timeout_ms) == osOK) {
            return RTE_SUCCESS;
        }
        HAL_QSPI_Abort_IT(qspi_control_handle[qspi_name].device.fd);
        return RTE_ERR_TIMEOUT;
    }
}

static rte_error_t qspi_recv_async(hal_device_t *device, uint8_t *buffer, uint32_t *size, uint32_t timeout_ms)
{
    RTE_UNUSED(size);
    qspi_name_t qspi_name = device->device_id;
    if (qspi_control_handle[qspi_name].mdma_handle != NULL) {
        // HAL_UARTEx_ReceiveToIdle_DMA(qspi_control_handle[qspi_name].device.fd,
        //                     qspi_control_handle[qspi_name].buffer,
        //                     qspi_control_handle[qspi_name].capacity);
        // if (osSemaphoreAcquire(qspi_control_handle[qspi_name].rx_sema, timeout_ms) == osOK) {
        //     HAL_RAM_CLEAN_AFTER_REC(qspi_control_handle[qspi_name].buffer, qspi_control_handle[qspi_name].recv_length);
        //     *size = RTE_MIN(qspi_control_handle[qspi_name].recv_length, *size);
        //     memcpy(buffer, qspi_control_handle[qspi_name].buffer, *size);
        //     return RTE_SUCCESS;
        // }
        // *size = 0;
        // HAL_UART_AbortReceive_IT(qspi_control_handle[qspi_name].device.fd);
        return RTE_ERR_TIMEOUT;
    } else {
        HAL_QSPI_Receive_IT(qspi_control_handle[qspi_name].device.fd, buffer);
        if (osSemaphoreAcquire(qspi_control_handle[qspi_name].device.rx_sema, timeout_ms) == osOK) {
            return RTE_SUCCESS;
        }
        *size = 0;
        HAL_QSPI_Abort_IT(qspi_control_handle[qspi_name].device.fd);
        return RTE_ERR_TIMEOUT;
    }
}

void w25_flash_map(void)
{
    QSPI_CommandTypeDef s_command = {0};
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};

    /* 基本配置 */
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;      /* 1线方式发送指令 */
    s_command.AddressSize = QSPI_ADDRESS_32_BITS;             /* 32位地址 */
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字节 */
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;                /* W25Q256JV不支持DDR */
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;   /* DDR模式，数据输出延迟 */
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;            /* 每次传输都发指令 */

    /* 全部采用4线 */
    s_command.Instruction = 0xEC; /* 快速读取命令 */
    s_command.AddressMode = QSPI_ADDRESS_4_LINES;                 /* 4个地址线 */
    s_command.DataMode = QSPI_DATA_4_LINES;                       /* 4个数据线 */
    s_command.DummyCycles = 6;                                    /* 空周期 */

    /* 关闭溢出计数 */
    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod = 0;

    HAL_QSPI_MemoryMapped(qspi_control_handle[qspi_1].device.fd, &s_command, &s_mem_mapped_cfg);
}

rte_error_t qspi_create(qspi_name_t qspi_name, qspi_configuration_t *config, hal_device_t **device)
{
    hal_device_initialize(qspi, qspi_name,
                        qspi_recv, qspi_send, qspi_recv_async, qspi_send_async,
                        device);
    return RTE_SUCCESS;
}

void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    HAL_DEVICE_OP_COMPLET_HANDLE(qspi, tx, hqspi);
}

void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    HAL_DEVICE_OP_COMPLET_HANDLE(qspi, rx, hqspi);
}

