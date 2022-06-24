/**
 * @file ds_vector.c
 * @author Leon Shan
 * @brief A general vector design, used when element is fixed-sized and
 *        the vector is read more often than write.
 *        NOTE:
 *        1. optional ability of expand capacity.
 *        2. optional ability of deep copy.
 *        3. Unlike ringbuffer, the head of the vector won't be bigger
 *           than the capacity of the vector. When it reach the tail of
 *           the vector, it will return to the head of the vector automatically.
 *        4. Only one element can be added or removed at a time.
 *        5. Mutex used for vector must be recursive.
 * @version 1.0.0
 * @date 2021-07-29
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../inc/data_structure/ds_vector.h"
#include "../../inc/middle_layer/rte_memory.h"

typedef struct __ds_vector {
    uint8_t if_deep_copy:1;
    uint8_t if_expand:1;
    uint8_t reserved:6;
    uint32_t capacity;
    uint32_t element_size;
    uint32_t head;  // pointer to first valid data's position
    uint32_t length;
    void *data;
    vector_element_free_cb_f free_cb;
    rte_mutex_t *mutex;              // mutex for the whole vector.
} ds_vector_impl_t;

#define VECTOR_LOCK(vector)           rte_lock(vector->mutex)
#define VECTOR_UNLOCK(vector)         rte_unlock(vector->mutex)

/**
 * @brief Creat a vector with fixed capacity.
 *
 * @param config
 * @param handle
 * @return rte_error_t
 */
rte_error_t ds_vector_create(vector_configuration_t *config, ds_vector_t *handle)
{
    if (rte_unlikely(handle == NULL) ||
        rte_unlikely(config == NULL) ||
        rte_unlikely(config->capacity == 0) ||
        rte_unlikely(config->if_deep_copy && config->element_size == 0)) {
        return RTE_ERR_PARAM;
    }
    ds_vector_impl_t *vector = NULL;

    vector = rte_calloc(sizeof(ds_vector_impl_t));
    if (!vector)
        return RTE_ERR_NO_MEM;

    vector->length = 0;
    vector->capacity = rte_roundup_pow_of_two(config->capacity);
    vector->head = 0;
    vector->if_deep_copy = config->if_deep_copy;
    vector->if_expand = config->if_expand;
    if(vector->if_deep_copy) {
        vector->element_size = config->element_size;
    } else {
        vector->element_size = sizeof(void *);
    }
    vector->data = rte_calloc(vector->element_size * vector->capacity);
    vector->free_cb = config->free_cb;
    vector->mutex = config->mutex;
    if (!vector->data) {
        rte_free(vector);
        return RTE_ERR_NO_MEM;
    }

    *handle = (void *)vector;
    return RTE_SUCCESS;
}
/**
 * @brief Clear a created vector.
 *
 * @param handle
 * @return rte_error_t
 */
rte_error_t ds_vector_clear(ds_vector_t handle)
{
    ds_vector_impl_t *vector = (ds_vector_impl_t *)handle;
    if (rte_unlikely(vector == NULL)) {
        return RTE_ERR_PARAM;
    }
    VECTOR_LOCK(vector);
    for (uint32_t i = 0; i < vector->length; i++) {
        uint32_t index = (vector->head + i) & (vector->capacity - 1);
        if(vector->if_deep_copy)
            memset((uint8_t *)vector->data + index * vector->element_size, 0, vector->element_size);
        else
            ((void **)vector->data)[index] = NULL;
    }

    vector->length = 0;
    vector->head = 0;

    VECTOR_UNLOCK(vector);
    return RTE_SUCCESS;
}
/**
 * @brief Destroy a created vector. If the user has registed the free data cb,
 *        each data will call such function.
 *
 * @param handle
 * @return rte_error_t
 */
rte_error_t ds_vector_destroy(ds_vector_t handle)
{
    ds_vector_impl_t *vector = (ds_vector_impl_t *)handle;
    if (rte_unlikely(vector == NULL)) {
        return RTE_ERR_PARAM;
    }
    VECTOR_LOCK(vector);
    if (vector->free_cb) {
        for (uint32_t i = 0; i < vector->length; i++) {
            uint32_t index = (vector->head + i) & (vector->capacity - 1);
            if(vector->if_deep_copy)
                (*vector->free_cb)((uint8_t *)vector->data + index * vector->element_size, i);
            else
                (*vector->free_cb)(((void **)vector->data)[index], i);
        }
    }

    if (vector->data)
        rte_free(vector->data);
    // Record this mutex cause its owner will be released in the free api.
    rte_mutex_t *vector_mutex = vector->mutex;
    rte_free(vector);
    rte_unlock(vector_mutex);
    return RTE_SUCCESS;
}
/**
 * @brief Expand the vector to a new size.
 *
 * @param handle
 * @param new_size
 * @return rte_error_t
 */
rte_error_t ds_vector_expand(ds_vector_t handle, uint32_t new_size)
{
    ds_vector_impl_t *vector = (ds_vector_impl_t *)handle;
    if (rte_unlikely(vector == NULL)) {
        return RTE_ERR_PARAM;
    }
    VECTOR_LOCK(vector);
    if (rte_unlikely(vector->capacity >= new_size)) {
        VECTOR_UNLOCK(vector);
        return RTE_ERR_PARAM;
    }
    uint32_t oldcapacity = 0;
    oldcapacity = vector->capacity;
    void *tmp = NULL;
    tmp = rte_realloc(vector->data,
                    vector->element_size * new_size);
    if (tmp == NULL) {
        VECTOR_UNLOCK(vector);
        return RTE_ERR_NO_MEM;
    }
    vector->data = tmp;
    vector->capacity = new_size;

    // Judge if the data has been wrapped.
    if ((vector->head + vector->length) > oldcapacity) {
            // Choose the smaller length of the buffer to wrap around
        uint32_t first_section_size = (vector->head + vector->length) & (oldcapacity - 1);
        uint32_t second_section_size = (oldcapacity - vector->head);
        if (vector->head < (oldcapacity >> 1)) {
            //       |h|
            // |x|x|x|x|x|x|x|x|
            //       |h|
            // |-|-|-|x|x|x|x|x|x|x|x|-|-|-|-|-|
            if(vector->if_deep_copy)
                memcpy((uint8_t *)vector->data + oldcapacity * vector->element_size,
                        (uint8_t *)vector->data,
                        vector->element_size * first_section_size);
            else
                memcpy(&((void **)vector->data)[oldcapacity],
                        ((void **)vector->data),
                        vector->element_size * first_section_size);
        } else {
            //           |h|
            // |x|x|x|x|x|x|x|x|
            //                           |h|
            // |x|x|x|x|x|-|-|-|-|-|-|-|-|x|x|x|
            if(vector->if_deep_copy)
                memcpy((uint8_t *)vector->data + (oldcapacity + vector->head) * vector->element_size,
                        (uint8_t *)vector->data + vector->head * vector->element_size,
                        vector->element_size * second_section_size);
            else
                memcpy(&((void **)vector->data)[oldcapacity + vector->head],
                        &((void **)vector->data)[vector->head],
                        vector->element_size * second_section_size);
            // In this case, the head needs to be update.
            vector->head = oldcapacity + vector->head;
        }
    }
    VECTOR_UNLOCK(vector);
    return RTE_SUCCESS;
}
/**
 * @brief Push a data into the selected vector. If the space of vector is
 *        not enough, this api will double the vector's formal capacity.
 *
 * @param handle
 * @param value
 * @return rte_error_t
 */
rte_error_t ds_vector_push(ds_vector_t handle, void *value)
{
    ds_vector_impl_t *vector = (ds_vector_impl_t *)handle;
    if (rte_unlikely(vector == NULL) ||
        rte_unlikely(vector->if_deep_copy == true && value == NULL)) {
        return RTE_ERR_PARAM;
    }
    VECTOR_LOCK(vector);
    uint32_t index = 0;

    if (vector->capacity < vector->length + 1) {
        if(vector->if_expand) {
            rte_error_t result = ds_vector_expand(handle, vector->capacity << 1);
            if (result != RTE_SUCCESS) {
                VECTOR_UNLOCK(vector);
                return result;
            }
        } else {
            VECTOR_UNLOCK(vector);
            return RTE_ERR_NO_RSRC;
        }
    }
    index = (vector->head + vector->length) & (vector->capacity - 1);
    if(vector->if_deep_copy)
        memcpy((uint8_t *)vector->data + index * vector->element_size, value, vector->element_size);
    else
        ((void **)vector->data)[index] = value;
    vector->length++;
    VECTOR_UNLOCK(vector);
    return RTE_SUCCESS;
}
/**
 * @brief Pop the first data in the vector.
 *
 * @param handle
 * @param buffer
 * @return rte_error_t
 */
rte_error_t ds_vector_pop(ds_vector_t handle, void *buffer)
{
    ds_vector_impl_t *vector = (ds_vector_impl_t *)handle;
    if (rte_unlikely(vector == NULL)||
        rte_unlikely(buffer == NULL)) {
        return RTE_ERR_PARAM;
    }
    VECTOR_LOCK(vector);
    if (rte_unlikely(vector->data == NULL) ||
        rte_unlikely(vector->length == 0)) {
        VECTOR_UNLOCK(vector);
        return RTE_ERR_NO_RSRC;
    }

    if(vector->if_deep_copy)
        memcpy(buffer, (uint8_t *)vector->data + vector->head * vector->element_size, vector->element_size);
    else
        *((void **)buffer) = ((void **)vector->data)[vector->head];
    vector->head = (vector->head + 1) & (vector->capacity - 1);
    vector->length--;
    VECTOR_UNLOCK(vector);
    return RTE_SUCCESS;
}
/**
 * @brief Get the vector's element count.
 *        NOTE: this API is not thread-safe, user should lock the vector
 *              by himself.
 *
 * @param handle
 * @return uint32_t
 */
uint32_t ds_vector_length(ds_vector_t handle)
{
    ds_vector_impl_t *vector = (ds_vector_impl_t *)handle;
    return vector->length;
}
/**
 * @brief Get the vector's certain index's element.
 *        NOTE: this API is not thread-safe, user should lock the vector
 *              by himself.
 *
 * @param handle
 * @return uint32_t
 */
void *ds_vector_at(ds_vector_t handle, uint32_t index)
{
    ds_vector_impl_t *vector = (ds_vector_impl_t *)handle;
    if (rte_unlikely(vector == NULL) ||
        rte_unlikely(index > vector->length)) {
        return NULL;
    }
    uint32_t real_index = (vector->head + index) & (vector->capacity - 1);
    if(vector->if_deep_copy)
        return (void *)((uint8_t *)vector->data + real_index * vector->element_size);
    else
        return ((void **)vector->data)[real_index];
}
/**
 * @brief Remove the selected element at the input index.
 *
 * @param handle
 * @param index
 * @return rte_error_t
 */
rte_error_t ds_vector_remove_by_index(ds_vector_t handle, uint32_t index)
{
    ds_vector_impl_t *vector = (ds_vector_impl_t *)handle;
    if (rte_unlikely(vector == NULL)) {
        return RTE_ERR_PARAM;
    }
    VECTOR_LOCK(vector);
    if (rte_unlikely(index >= vector->length)) {
        VECTOR_UNLOCK(vector);
        return RTE_ERR_NO_RSRC;
    }
    // Judge if the data has two section.
    if (vector->head + vector->length > vector->capacity) {
        uint32_t first_section_size = (vector->head + vector->length) & (vector->capacity - 1);
        uint32_t second_section_size = (vector->capacity - vector->head);
        // Judge the element in which section.
        if(vector->head + index >= vector->capacity) {
            // In the first section
            //   |i|
            // |x|x|x|-|-|x|x|x|
            // |x|x|-|-|-|x|x|x|
            if(vector->if_deep_copy) {
                uint32_t real_index = (vector->head + index) & (vector->capacity - 1);
                uint8_t *start_ptr = (uint8_t *)vector->data + real_index * vector->element_size;
                memmove(start_ptr, start_ptr + 1,
                        vector->element_size * (first_section_size - real_index - 1));
            } else {
                uint32_t real_index = (vector->head + index) & (vector->capacity - 1);
                void **start_ptr = &((void **)vector->data)[real_index];
                if(vector->free_cb)
                    (*vector->free_cb)(*start_ptr, index);
                memmove(start_ptr, start_ptr + 1,
                        vector->element_size * (first_section_size - real_index - 1));
            }
        } else {
            // In the second section
            //           |i|
            // |x|x|x|-|-|x|x|x|
            if(vector->if_deep_copy) {
                // Copy the first part in second section.
                // |x|x|x|-|-|x|x|-|
                uint8_t *start_ptr = (uint8_t *)vector->data + (vector->head + index) * vector->element_size;
                memmove(start_ptr, start_ptr + vector->element_size,
                        vector->element_size * (second_section_size - index - 1));
                // Move first element in first section into second section.
                // |-|x|x|-|-|x|x|x|
                memcpy((uint8_t *)vector->data + (vector->capacity - 1) * vector->element_size,
                        (uint8_t *)vector->data,
                        vector->element_size);
                // Move first section.
                // |x|x|-|-|-|x|x|x|
                memmove((uint8_t *)vector->data,
                        (uint8_t *)vector->data + vector->element_size,
                        vector->element_size * (first_section_size - 1));
            } else {
                void **start_ptr = &((void **)vector->data)[vector->head + index];
                if(vector->free_cb)
                    (*vector->free_cb)(*start_ptr, index);
                memmove(start_ptr, start_ptr + 1,
                        vector->element_size * (second_section_size - index - 1));
                ((void **)(vector->data))[vector->capacity - 1] = ((void **)vector->data)[0];
                memmove(&((void **)vector->data)[0],
                        &((void **)vector->data)[1],
                        vector->element_size * (first_section_size - 1));
            }
        }
    } else {
        //           |i|
        // |-|-|-|-|-|x|x|x|
        // |-|-|-|-|-|x|x|-|
        if(vector->if_deep_copy) {
            uint8_t *start_ptr = (uint8_t *)vector->data + (vector->head + index) * vector->element_size;
            memmove(start_ptr, start_ptr + vector->element_size,
                    vector->element_size * (vector->length - index - 1));
        } else {
            void **start_ptr = &((void **)vector->data)[vector->head + index];
            if(vector->free_cb)
                (*vector->free_cb)(*start_ptr, index);
            memmove(start_ptr, start_ptr + 1,
                    vector->element_size * (vector->length - index - 1));
        }
    }
    vector->length--;
    VECTOR_UNLOCK(vector);
    return RTE_SUCCESS;
}
/**
 * @brief Remove the selected element at the input pointer.
 *
 * @param handle
 * @param index
 * @return rte_error_t
 */
rte_error_t ds_vector_remove_by_pointer(ds_vector_t handle, void *pointer)
{
    ds_vector_impl_t *vector = (ds_vector_impl_t *)handle;
    if (rte_unlikely(vector == NULL)) {
        return RTE_ERR_PARAM;
    }
    VECTOR_LOCK(vector);
    uint32_t i = 0;
    rte_error_t result = RTE_ERR_UNDEFINE;
    for (i = 0; i < vector->length; i++) {
        if(ds_vector_at(vector, i) == pointer)
            break;
    }
    if(i < vector->length)
        result = ds_vector_remove_by_index(vector, i);
    else
        result = RTE_ERR_NO_RSRC;
    VECTOR_UNLOCK(vector);
    return result;
}
/**
 * @brief Lock the vector.
 *
 * @param handle
 */
void ds_vector_lock(ds_vector_t handle)
{
    ds_vector_impl_t *vector = (ds_vector_impl_t *)handle;
    VECTOR_LOCK(vector);
}
/**
 * @brief Unlock the vector.
 *
 * @param handle
 */
void ds_vector_unlock(ds_vector_t handle)
{
    ds_vector_impl_t *vector = (ds_vector_impl_t *)handle;
    VECTOR_UNLOCK(vector);
}
