#include <stdio.h>
#include "rte_include.h"
#include "SDL.h"

void test_ringbuffer(void)
{
    RTE_LOGI("test_buffer start!");
    void *buffer = NULL;
    uint8_t data[10] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    rte_error_t retval = RTE_ERR_UNDEFINE;
    uint8_t output[256] = {0};
    retval = ds_ringbuffer_create(18, NULL, &buffer);
    RTE_LOGI("test_buffer create result %d is at: 0x%p", retval, buffer);
    typedef struct {
        uint8_t *data;
        uint32_t capacity;
        uint32_t head;
        uint32_t tail;
        rte_mutex_t *mutex;
    } test_buffer_t;
    test_buffer_t *test_buffer = (test_buffer_t *)buffer;

    retval = ds_ringbuffer_write(buffer, data, 10);
    RTE_LOGI("test_buffer write result %d", retval);
    RTE_LOGI("test_buffer head %d tail %d", test_buffer->head, test_buffer->tail);
    uint32_t size = 32;
    uint8_t read_buf[32] = {0};
    retval = ds_ringbuffer_read(buffer, read_buf, &size);
    RTE_LOGI("test_buffer read result %d size %d", retval, size);

    memset(output, 0, 256);
    for (uint32_t i = 0; i < size; i++) {
        sprintf(output, "%s0x%x ", output, read_buf[i]);
    }
    RTE_LOGI("test_buffer read data %s",output);
    RTE_LOGI("test_buffer head %d tail %d", test_buffer->head, test_buffer->tail);

    retval = ds_ringbuffer_write(buffer, data, 10);
    RTE_LOGI("test_buffer write result %d", retval);
    RTE_LOGI("test_buffer %p head %d tail %d", buffer, test_buffer->head, test_buffer->tail);
    memset(output, 0, 256);
    for (uint32_t i = 0; i < test_buffer->capacity; i++) {
        sprintf(output, "%s0x%x ", output, test_buffer->data[i]);
    }
    RTE_LOGI("test_buffer %p real data %s", buffer, output);

    memset(output, 0, 256);
    memset(read_buf, 0, 32);
    size = 32;
    retval = ds_ringbuffer_read(buffer, read_buf, &size);
    RTE_LOGI("test_buffer read result %d size %d", retval, size);
    for (uint32_t i = 0; i < size; i++) {
        sprintf(output, "%s0x%x ", output, read_buf[i]);
    }
    RTE_LOGI("test_buffer read data %s",output);
    RTE_LOGI("test_buffer head %d tail %d", test_buffer->head, test_buffer->tail);
    retval = ds_ringbuffer_destroy(buffer);
    RTE_LOGI("test_buffer destroy result %d",retval);
}
