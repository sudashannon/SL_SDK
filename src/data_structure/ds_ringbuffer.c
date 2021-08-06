/**
 * @file ds_ringbuffer.c
 * @author Leon Shan (813475603@qq.com)
 * @brief A general buffer design, used when unfixed size of data needs to be
 *        written and read.
 *        NOTE:
 *        1. The head is the read pointer and the tail is the write pointer, both
 *           of which can be bigger than the capacity to reach the aim of ringbuffer.
 *        2. When read, the size of the user's buffer should be provided and the api
 *           will set the size into the real size of the read data.
 *        3. The user input size of the buffer will be expanded to accelerate the speed
 *           of calculation.
 *        4. Mutex used for vector must be recursive.
 * @version 1.0.0
 * @date 2021-08-03
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../inc/data_structure/ds_ringbuffer.h"

typedef struct _ds_ringbuffer {
    uint8_t *data;
    uint32_t capacity;
    uint32_t head;
    uint32_t tail;
    rte_mutex_t *mutex;
    rte_allocator_t *allocator;
} ds_ringbuffer_impl_t;

#define BUFFER_LOCK(buffer)           RTE_LOCK(buffer->mutex)
#define BUFFER_UNLOCK(buffer)         RTE_UNLOCK(buffer->mutex)

/**
 * @brief Create a fix size buffer.
 *
 * @param capacity
 * @param mutex
 * @param handle
 * @return rte_error_t
 */
rte_error_t ds_ringbuffer_create(uint32_t capacity, rte_mutex_t *mutex, ds_ringbuffer_t *handle)
{
    if (RTE_UNLIKELY(handle == NULL) ||
        RTE_UNLIKELY(capacity == 0)) {
        return RTE_ERR_PARAM;
    }
    rte_allocator_t *allocator = rte_get_general_allocator();
    if (RTE_UNLIKELY(allocator == NULL)) {
        return RTE_ERR_NO_RSRC;
    }
    ds_ringbuffer_impl_t *buffer = NULL;

    buffer = allocator->calloc(sizeof(ds_ringbuffer_impl_t));
    if (!buffer)
        return RTE_ERR_NO_MEM;

    buffer->capacity = rte_roundup_pow_of_two(capacity);
    buffer->head = 0;
    buffer->tail = 0;
    buffer->data = (uint8_t *)allocator->calloc(sizeof(uint8_t) * buffer->capacity);
    buffer->allocator = allocator;
    buffer->mutex = mutex;
    if (!buffer->data) {
        allocator->free(buffer);
        return RTE_ERR_NO_MEM;
    }

    *handle = (void *)buffer;
    return RTE_SUCCESS;
}
/**
 * @brief Destroy a created buffer.
 *
 * @param handle
 * @return rte_error_t
 */
rte_error_t ds_ringbuffer_destroy(ds_ringbuffer_t handle)
{
    ds_ringbuffer_impl_t *buffer = (ds_ringbuffer_impl_t *)handle;
    if (RTE_UNLIKELY(buffer == NULL) ||
        RTE_UNLIKELY(buffer->allocator == NULL)) {
        return RTE_ERR_PARAM;
    }
    BUFFER_LOCK(buffer);
    if (buffer->data)
        buffer->allocator->free(buffer->data);
    // Record this mutex cause its owner will be released in the free api.
    rte_mutex_t *buffer_mutex = buffer->mutex;
    buffer->allocator->free(buffer);
    RTE_UNLOCK(buffer_mutex);
    return RTE_SUCCESS;
}

static inline uint32_t ds_ringbuffer_free_size(ds_ringbuffer_impl_t *buffer)
{
    return buffer->capacity - (buffer->tail - buffer->head);
}

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
rte_error_t ds_ringbuffer_write(ds_ringbuffer_t handle, uint8_t *data, uint32_t size)
{
    ds_ringbuffer_impl_t *buffer = (ds_ringbuffer_impl_t *)handle;
    if (RTE_UNLIKELY(buffer == NULL) ||
        RTE_UNLIKELY(data == NULL)) {
        return RTE_ERR_PARAM;
    }
    BUFFER_LOCK(buffer);
    uint32_t head_offset = buffer->head & (buffer->capacity - 1);
    uint32_t tail_offset = buffer->tail & (buffer->capacity - 1);
    uint32_t new_tail_offset = (buffer->tail + size + sizeof(uint32_t))  & (buffer->capacity - 1);
    bool if_wrap = (buffer->capacity - tail_offset) < (sizeof(uint32_t) + size);
    uint32_t free_size = ds_ringbuffer_free_size(buffer);
    // 1. Check if the left free space is bigger than input size.
    // 2. Check if the coming data will overlap the existed data when wrap.
    if (free_size < sizeof(uint32_t) + size ||
        (if_wrap && new_tail_offset >= head_offset)) {
        BUFFER_UNLOCK(buffer);
        return RTE_ERR_NO_RSRC;
    }
    // Get the write pointer
    uint8_t *write_ptr = (buffer->data + tail_offset);
    if (if_wrap) {
        // Copy the data into the left before the end of the buffer.
        uint32_t first_in_size = buffer->capacity - tail_offset;
        if (first_in_size < sizeof(uint32_t)) {
            memcpy(write_ptr,
                    (uint8_t *)&size,
                    first_in_size);
            memcpy(buffer->data,
                    (uint8_t *)&size + first_in_size,
                    sizeof(uint32_t) - first_in_size);
            memcpy(buffer->data + sizeof(uint32_t) - first_in_size,
                    data,
                    size);
        } else {
            *((uint32_t *)write_ptr) = size;
            first_in_size -= sizeof(uint32_t);
            memcpy(write_ptr + sizeof(uint32_t),
                    data,
                    first_in_size);
            memcpy(buffer->data,
                    data + first_in_size,
                    size - first_in_size);
        }
    } else {
        *((uint32_t *)write_ptr) = size;
        memcpy(write_ptr + sizeof(uint32_t),
                data,
                size);
    }
    // Update tail value.
    buffer->tail += size + sizeof(uint32_t);
    BUFFER_UNLOCK(buffer);
    return RTE_SUCCESS;
}
/**
 * @brief Read some data from the ds_ringbuffer into the user's buffer.
 *
 * @param handle
 * @param data
 * @param size
 * @return rte_error_t
 */
rte_error_t ds_ringbuffer_read(ds_ringbuffer_t handle, uint8_t *data, uint32_t *size)
{
    ds_ringbuffer_impl_t *buffer = (ds_ringbuffer_impl_t *)handle;
    if (RTE_UNLIKELY(buffer == NULL) ||
        RTE_UNLIKELY(data == NULL) ||
        RTE_UNLIKELY(size == NULL)) {
        return RTE_ERR_PARAM;
    }
    BUFFER_LOCK(buffer);
    // Check if the buffer is empty
    if (buffer->head == buffer->tail) {
        *size = 0;
        BUFFER_UNLOCK(buffer);
        return RTE_ERR_NO_RSRC;
    }
    uint32_t head_offset = buffer->head  & (buffer->capacity - 1);
    // Get the read pointer and size.
    uint8_t *read_ptr = (buffer->data + head_offset);
    uint32_t read_size = 0;
    // Check if the size has been wrapped.
    if ((buffer->capacity - head_offset) < sizeof(uint32_t)) {
        memcpy((uint8_t *)&read_size,
                read_ptr,
                buffer->capacity - head_offset);
        memcpy((uint8_t *)&read_size + buffer->capacity - head_offset,
                buffer->data,
                sizeof(uint32_t) - (buffer->capacity - head_offset));
        if(*size < read_size) {
            BUFFER_UNLOCK(buffer);
            return RTE_ERR_PARAM;
        }
        memcpy(data,
                buffer->data + sizeof(uint32_t) - (buffer->capacity - head_offset),
                read_size);
    } else {
        read_size = *((uint32_t *)read_ptr);
        if(*size < read_size) {
            BUFFER_UNLOCK(buffer);
            return RTE_ERR_PARAM;
        }
        // Check if the data has been wrapped.
        if((buffer->capacity - head_offset) < (sizeof(uint32_t) + read_size)) {
            // Copy the data from the left before the end of the buffer.
            uint32_t first_out_size = buffer->capacity - head_offset - sizeof(uint32_t);
            memcpy(data, read_ptr + sizeof(uint32_t), first_out_size);
            memcpy(data + first_out_size, buffer->data, read_size - first_out_size);
        } else {
            memcpy(data, read_ptr + sizeof(uint32_t), read_size);
        }
    }

    *size = read_size;
    buffer->head += read_size + sizeof(uint32_t);
    BUFFER_UNLOCK(buffer);
    return RTE_SUCCESS;
}
