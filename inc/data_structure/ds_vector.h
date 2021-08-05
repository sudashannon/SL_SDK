/**
 * @file ds_vector.h
 * @author Leon Shan (813475603@qq.com)
 * @brief A general vector design, used when element is fixed-sized.
 * @version 1.0.0
 * @date 2021-07-29
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __DS_VECTOR_H
#define __DS_VECTOR_H

#include "../middle_layer/rte.h"

typedef void (*vector_element_free_cb)(void *element, uint8_t index);

typedef struct {
    uint8_t if_deep_copy:1;
    uint8_t if_expand:1;
    uint8_t reserved:6;
    uint32_t element_size;
    uint32_t capacity;
    rte_mutex_t *mutex;
    vector_element_free_cb free_cb;
} vector_configuration_t;

#define VECTOR_INITIALIZER {                                \
    .capacity = 0,                                          \
    .element_size = 0,                                      \
    .free_cb = NULL,                                        \
    .if_deep_copy = false,                                  \
    .if_expand = false,                                     \
    .mutex = NULL,                                          \
}

/**
 * @brief Creat a queue with fixed capacity.
 *
 * @param config
 * @param handle
 * @return rte_error_t
 */
extern rte_error_t ds_vector_create(vector_configuration_t *config, void **handle);
/**
 * @brief Destroy a created queue. If the user has registed the free data cb,
 *        each data will call such function.
 *
 * @param handle
 * @return rte_error_t
 */
extern rte_error_t ds_vector_destroy(void *handle);
/**
 * @brief Push a data into the selected queue. If the space of queue is
 *        not enough, this api will double the queue's formal capacity.
 *
 * @param handle
 * @param value
 * @return rte_error_t
 */
extern rte_error_t ds_vector_push(void *handle, void *value);
/**
 * @brief Pop the first data in the queue.
 *
 * @param handle
 * @param buffer
 * @return rte_error_t
 */
extern rte_error_t ds_vector_pop(void *handle, void *buffer);
/**
 * @brief Get the vector's element count.
 *        NOTE: this API is not thread-safe, user should lock the vector
 *              by himself.
 *
 * @param handle
 * @return uint32_t
 */
extern uint32_t ds_vector_length(void *handle);
/**
 * @brief Get the vector's certain index's element.
 *        NOTE: this API is not thread-safe, user should lock the vector
 *              by himself.
 *
 * @param handle
 * @return uint32_t
 */
extern void *ds_vector_at(void *handle, uint32_t index);
/**
 * @brief Remove the selected element at the input index.
 *
 * @param handle
 * @param index
 * @return rte_error_t
 */
extern rte_error_t ds_vector_remove_by_index(void *handle, uint32_t index);
/**
 * @brief Remove the selected element at the input pointer.
 *
 * @param handle
 * @param index
 * @return rte_error_t
 */
extern rte_error_t ds_vector_remove_by_pointer(void *handle, void *pointer);
#endif