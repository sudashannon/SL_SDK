#include <stdio.h>
#include "rte_include.h"
#include "SDL.h"

MEM_ALIGN_NBYTES (static uint8_t os_buffer[1024 * 1024], MEM_BLOCK_ALIGN) = {0};

typedef struct sugar_queue_rbt_element {
    uint8_t priority;
    ds_vector_t tcb_array;
} test_sugar_queue_rbt_element_t;

static int walk_fun(rbt_t *rbt, void *key, size_t ksize, void *value, void *priv)
{
    test_sugar_queue_rbt_element_t *element_node = (test_sugar_queue_rbt_element_t *)value;
    OS_LOGI("node %p 's vector is %p, tcb_array size is %d", element_node, element_node->tcb_array, ds_vector_length(element_node->tcb_array));
    sugar_tcb_t *element = NULL;
    uint32_t index = 0;
    VECTOR_FOR_EACH_SAFELY(index, element, element_node->tcb_array) {
        OS_LOGI("node %p %d tcb is %p", element_node, index, element);
    }
    return 1;
}

void test_sugar_queue(void)
{
    memory_pool(BANK_OS, NULL, os_buffer, sizeof(os_buffer));
    sugar_queue_impl_t *queue = sugar_tcb_queue_init();
    sugar_tcb_t *first_tcb = memory_calloc(BANK_OS, sizeof(sugar_tcb_t));
    first_tcb->priority = SUGAR_IDLE_THREAD_PRIORITY;
    sugar_tcb_ready_queue_push_priority(queue, first_tcb);
    rbt_walk(queue->tcb_rbt, walk_fun, NULL);
    sugar_tcb_t *second_tcb = memory_calloc(BANK_OS, sizeof(sugar_tcb_t));
    second_tcb->priority = 10;
    sugar_tcb_ready_queue_push_priority(queue, second_tcb);
    rbt_walk(queue->tcb_rbt, walk_fun, NULL);
    sugar_tcb_t *ready_one = sugar_tcb_ready_queue_pop_next(queue);
    OS_LOGI("ready tcb is %p", ready_one);
    rbt_walk(queue->tcb_rbt, walk_fun, NULL);
}
