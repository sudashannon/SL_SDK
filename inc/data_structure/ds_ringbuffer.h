/**
 * @file ds_ringbuffer.h
 * @author Leon Shan (813475603@qq.com)
 * @brief A general buffer design, support dma to write data in.
 * @version 1.0.0
 * @date 2021-08-03
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __DS_RINGBUFFER_H
#define __DS_RINGBUFFER_H

#include "../middle_layer/rte.h"

/**
 * @brief Create a fix size buffer.
 *
 * @param capacity
 * @param mutex
 * @param handle
 * @return rte_error_t
 */
extern rte_error_t ds_ringbuffer_create(uint32_t capacity, rte_mutex_t *mutex, void **handle);
/**
 * @brief Destroy a created buffer.
 *
 * @param handle
 * @return rte_error_t
 */
extern rte_error_t ds_ringbuffer_destroy(void *handle);
/**
 * @brief Write some data into a created buffer.
 *        NOTE: this function support DMA, user needs to register the
 *              write callback first.
 *
 * @param handle
 * @param data
 * @param size
 * @return rte_error_t
 */
extern rte_error_t ds_ringbuffer_write(void *handle, uint8_t *data, uint32_t size);
/**
 * @brief Read some data from the ds_ringbuffer into the user's buffer.
 *
 * @param handle
 * @param data
 * @param size
 * @return rte_error_t
 */
extern rte_error_t ds_ringbuffer_read(void *handle, uint8_t *data, uint32_t *size);
#endif
