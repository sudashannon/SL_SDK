#include <stdio.h>
#include "rte_include.h"
#include "SDL.h"

static uint8_t *store_ptr[8] = {NULL};
static void element_free(void *element, uint32_t index) {
    RTE_LOGI("element 0x%p store 0x%p index %d", element, store_ptr[index], index);
    RTE_ASSERT((uint8_t *)element == store_ptr[index]);
    rte_free(element);
}

void test_vector(void)
{
    uint8_t data[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    RTE_LOGI("test vector start!");
    // Test vector's basic function which is without deep copy, free_cb and expand.
    {
        void *vector = NULL;
        vector_configuration_t configuration = VECTOR_CONFIG_INITIALIZER;
        configuration.capacity = 8;
        rte_error_t result = ds_vector_create(&configuration, &vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        for (uint8_t i = 0; i < configuration.capacity; i++) {
            result = ds_vector_push(vector, &data[i]);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(ds_vector_at(vector, i) == &data[i]);
        }
        for (uint8_t i = 0; i < configuration.capacity; i++) {
            uint8_t *temp_data = NULL;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(temp_data == &data[i]);
            RTE_ASSERT(*temp_data == data[i]);
        }
        // |00|11|22|33|44|55|66|77|
        for (uint8_t i = 0; i < configuration.capacity; i++) {
            result = ds_vector_push(vector, &data[i]);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(ds_vector_at(vector, i) == &data[i]);
        }
        // |00|11|22|33|44|55|66|77|
        // |11|22|33|44|55|66|77|--|
        // |11|22|44|55|66|77|--|--|
        // |11|22|44|55|77|--|--|--|
        for (uint8_t i = 0; i < 3; i++) {
            result = ds_vector_remove_by_index(vector, i * 2);
            RTE_ASSERT(result == RTE_SUCCESS);
        }
        for (uint8_t i = 0; i < configuration.capacity - 3; i++) {
            uint8_t *temp_data = NULL;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(*temp_data == data[i + (i / 2 + 1)]);
        }
        result = ds_vector_destroy(vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        RTE_LOGI("test vector 's basic fuction successfully!");
    }
    // Test vector's remove api in wrapped vector at first section.
    {
        void *vector = NULL;
        vector_configuration_t configuration = VECTOR_CONFIG_INITIALIZER;
        configuration.capacity = 8;
        rte_error_t result = ds_vector_create(&configuration, &vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        // Move the head to 2, the vector will be like:
        //        h
        // |-|-|-|-|-|-|-|-|
        for (uint8_t i = 0; i < 3; i++) {
            result = ds_vector_push(vector, NULL);
            RTE_ASSERT(result == RTE_SUCCESS);
            void *temp_data = NULL;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
        }
        // |55|66|77|00|11|22|33|44|
        for (uint8_t i = 0; i < configuration.capacity; i++) {
            result = ds_vector_push(vector, &data[i]);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(ds_vector_at(vector, i) == &data[i]);
        }
        // |55|66|77|00|11|22|33|44|
        // |66|77|--|11|22|33|44|55|
        // |77|--|--|11|22|44|55|66|
        // |--|--|--|11|22|44|55|66|
        uint8_t result_data[5] = {0x11, 0x22, 0x44, 0x55, 0x66};
        for (uint8_t i = 0; i < 2; i++) {
            result = ds_vector_remove_by_index(vector, i * 2);
            RTE_ASSERT(result == RTE_SUCCESS);
        }
        result = ds_vector_remove_by_index(vector, 5);
        RTE_ASSERT(result == RTE_SUCCESS);
        for (uint8_t i = 0; i < configuration.capacity - 3; i++) {
            uint8_t *temp_data = NULL;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(*temp_data == result_data[i]);        }
        result = ds_vector_destroy(vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        RTE_LOGI("test vector with out deep copy, free cb, and expand successfully!");
    }
    // Test vector with free_cb.
    {
        void *vector = NULL;
        vector_configuration_t configuration = VECTOR_CONFIG_INITIALIZER;
        configuration.capacity = 8;
        configuration.free_cb = element_free;
        rte_error_t result = ds_vector_create(&configuration, &vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        for (uint8_t i = 0; i < configuration.capacity; i++) {
            uint8_t *temp_data = rte_malloc(sizeof(uint8_t));
            *temp_data = i;
            result = ds_vector_push(vector, temp_data);
            store_ptr[i] = temp_data;
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(ds_vector_at(vector, i) == temp_data);
        }
        ds_vector_destroy(vector);
        RTE_LOGI("test vector with free_cb successfully!");
    }
    // Test vector with expand.
    {
        void *vector = NULL;
        vector_configuration_t configuration = VECTOR_CONFIG_INITIALIZER;
        configuration.capacity = 8;
        configuration.if_expand = true;
        rte_error_t result = ds_vector_create(&configuration, &vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        for (uint8_t i = 0; i < sizeof(data); i++) {
            result = ds_vector_push(vector, &data[i]);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(ds_vector_at(vector, i) == &data[i]);
        }
        for (uint8_t i = 0; i < sizeof(data); i++) {
            uint8_t *temp_data = NULL;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(temp_data == &data[i]);
            RTE_ASSERT(*temp_data == data[i]);
        }
        result = ds_vector_destroy(vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        RTE_LOGI("test vector with expand successfully!");
    }
    // Test vector with expand and wrap the first section.
    {
        void *vector = NULL;
        vector_configuration_t configuration = VECTOR_CONFIG_INITIALIZER;
        // Create a vector like:
        //  h
        // |-|-|-|-|-|-|-|-|
        configuration.capacity = 8;
        configuration.if_expand = true;
        rte_error_t result = ds_vector_create(&configuration, &vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        // Move the head to 3, the vector will be like:
        //        h
        // |-|-|-|-|-|-|-|-|
        for (uint8_t i = 0; i < 3; i++) {
            result = ds_vector_push(vector, NULL);
            RTE_ASSERT(result == RTE_SUCCESS);
            void *temp_data = NULL;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
        }
        // Push 8 data into the vector, the vector will be like:
        //        h
        // |x|x|x|x|x|x|x|x|
        // Push another 2 data into the vector, the vector will be like:
        //        h
        // |-|-|-|x|x|x|x|x|x|x|x|-|-|-|-|-|
        for (uint8_t i = 0; i < 10; i++) {
            result = ds_vector_push(vector, &data[i]);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(ds_vector_at(vector, i) == &data[i]);
        }
        for (uint8_t i = 0; i < 10; i++) {
            uint8_t *temp_data = NULL;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(temp_data == &data[i]);
            RTE_ASSERT(*temp_data == data[i]);
        }
        result = ds_vector_destroy(vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        RTE_LOGI("test vector with expand and wrap the first section successfully!");
    }
    // Test vector with expand and wrap the second section.
    {
        void *vector = NULL;
        vector_configuration_t configuration = VECTOR_CONFIG_INITIALIZER;
        // Create a vector like:
        //  h
        // |-|-|-|-|-|-|-|-|
        configuration.capacity = 8;
        configuration.if_expand = true;
        rte_error_t result = ds_vector_create(&configuration, &vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        // Move the head to 5, the vector will be like:
        //            h
        // |-|-|-|-|-|-|-|-|
        for (uint8_t i = 0; i < 6; i++) {
            result = ds_vector_push(vector, NULL);
            RTE_ASSERT(result == RTE_SUCCESS);
            void *temp_data = NULL;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
        }
        // Push 8 data into the vector, the vector will be like:
        //            h
        // |x|x|x|x|x|x|x|x|
        for (uint8_t i = 0; i < 10; i++) {
            result = ds_vector_push(vector, &data[i]);
            RTE_ASSERT(result == RTE_SUCCESS);
        }
        for (uint8_t i = 0; i < 10; i++) {
            uint8_t *temp_data = NULL;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(temp_data == &data[i]);
            RTE_ASSERT(*temp_data == data[i]);
        }
        result = ds_vector_destroy(vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        RTE_LOGI("test vector with expand and wrap the second section successfully!");
    }
    // Replay the formal test with deep copy.
    {
        void *vector = NULL;
        vector_configuration_t configuration = VECTOR_CONFIG_INITIALIZER;
        configuration.capacity = 8;
        configuration.if_deep_copy = 1;
        configuration.element_size = sizeof(uint8_t);
        rte_error_t result = ds_vector_create(&configuration, &vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        for (uint8_t i = 0; i < configuration.capacity; i++) {
            result = ds_vector_push(vector, &data[i]);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(*((uint8_t *)ds_vector_at(vector, i)) == data[i]);
        }
        for (uint8_t i = 0; i < configuration.capacity; i++) {
            uint8_t temp_data = 0;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(temp_data == data[i]);
        }
        // |00|11|22|33|44|55|66|77|
        for (uint8_t i = 0; i < configuration.capacity; i++) {
            result = ds_vector_push(vector, &data[i]);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(*((uint8_t *)ds_vector_at(vector, i)) == data[i]);
        }
        // |00|11|22|33|44|55|66|77|
        // |11|22|33|44|55|66|77|--|
        // |11|22|44|55|66|77|--|--|
        // |11|22|44|55|77|--|--|--|
        for (uint8_t i = 0; i < 3; i++) {
            result = ds_vector_remove_by_index(vector, i * 2);
            RTE_ASSERT(result == RTE_SUCCESS);
        }
        for (uint8_t i = 0; i < configuration.capacity - 3; i++) {
            uint8_t temp_data = 0;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(temp_data == data[i + (i / 2 + 1)]);
        }
        result = ds_vector_destroy(vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        RTE_LOGI("test vector 's basic fuction with deep copy successfully!");
    }
    // Test vector with expand and wrap the first section.
    {
        void *vector = NULL;
        vector_configuration_t configuration = VECTOR_CONFIG_INITIALIZER;
        // Create a vector like:
        //  h
        // |-|-|-|-|-|-|-|-|
        configuration.capacity = 8;
        configuration.if_deep_copy = 1;
        configuration.element_size = sizeof(uint8_t);
        configuration.if_expand = true;
        rte_error_t result = ds_vector_create(&configuration, &vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        // Move the head to 3, the vector will be like:
        //        h
        // |-|-|-|-|-|-|-|-|
        for (uint8_t i = 0; i < 3; i++) {
            uint8_t temp = 0xee;
            result = ds_vector_push(vector, &temp);
            RTE_ASSERT(result == RTE_SUCCESS);
            uint8_t temp_data = 0;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(temp_data == 0xee);
        }
        // Push 8 data into the vector, the vector will be like:
        //        h
        // |x|x|x|x|x|x|x|x|
        // Push another 2 data into the vector, the vector will be like:
        //        h
        // |-|-|-|x|x|x|x|x|x|x|x|-|-|-|-|-|
        for (uint8_t i = 0; i < 10; i++) {
            result = ds_vector_push(vector, &data[i]);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(*((uint8_t *)ds_vector_at(vector, i)) == data[i]);
        }
        for (uint8_t i = 0; i < 10; i++) {
            uint8_t temp_data = 0;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(temp_data == data[i]);
        }
        result = ds_vector_destroy(vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        RTE_LOGI("test vector with expand, deep copy and wrap the first section successfully!");
    }
    // Test vector with expand and wrap the second section.
    {
        void *vector = NULL;
        vector_configuration_t configuration = VECTOR_CONFIG_INITIALIZER;
        // Create a vector like:
        //  h
        // |-|-|-|-|-|-|-|-|
        configuration.capacity = 8;
        configuration.if_deep_copy = 1;
        configuration.element_size = sizeof(uint8_t);
        configuration.if_expand = true;
        rte_error_t result = ds_vector_create(&configuration, &vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        // Move the head to 5, the vector will be like:
        //            h
        // |-|-|-|-|-|-|-|-|
        for (uint8_t i = 0; i < 6; i++) {
            uint8_t temp = 0xee;
            result = ds_vector_push(vector, &temp);
            RTE_ASSERT(result == RTE_SUCCESS);
            uint8_t temp_data = 0;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(temp_data == 0xee);
        }
        // Push 8 data into the vector, the vector will be like:
        //            h
        // |x|x|x|x|x|x|x|x|
        // Push another 2 data into the vector, the vector will be like:
        //                            h
        // |x|x|x|x|x|-|-|-|-|-|-|-|-|x|x|x|
        for (uint8_t i = 0; i < 10; i++) {
            result = ds_vector_push(vector, &data[i]);
            RTE_ASSERT(result == RTE_SUCCESS);
        }
        for (uint8_t i = 0; i < 10; i++) {
            uint8_t temp_data = 0;
            result = ds_vector_pop(vector, &temp_data);
            RTE_ASSERT(result == RTE_SUCCESS);
            RTE_ASSERT(temp_data == data[i]);
        }
        result = ds_vector_destroy(vector);
        RTE_ASSERT(result == RTE_SUCCESS);
        RTE_LOGI("test vector with expand, deep copy and wrap the second section successfully!");
    }
    RTE_LOGI("test vector end!");
}
