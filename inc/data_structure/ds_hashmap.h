/**
 * @file ds_hashmap.h
 * @author Leon Shan (813475603@qq.com)
 * @brief Fast thread-safe hashtable implementation
 * @version 1.0.0
 * @date 2021-08-09
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __DS_HASHMAP_H
#define __DS_HASHMAP_H
#ifdef __cplusplus
extern "C" {
#endif

#include "../middle_layer/rte.h"

#define HASHTABLE_MIN_CAPACITY 128

typedef enum {
    HT_ITERATOR_STOP = 0,
    HT_ITERATOR_CONTINUE = 1,
    HT_ITERATOR_REMOVE = -1,
    HT_ITERATOR_REMOVE_AND_STOP = -2
} ht_iterator_status_t;
/**
 * @brief Callback that, if provided, will be called to release the value resources
 *        when an item is being removed from the table
 */
typedef void (*hashtable_element_free_cb_f)(void *);
/**
 * @brief Callback called if an item for a given key is found
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key : The key to use
 * @param klen : The length of the key
 * @return RTE_SUCCESS on success
 *         1 on success and the item must be removed from the hashtable
 *         other on error
 */
typedef rte_error_t (*hashtable_pair_cb_f)(ds_hashtable_t ptable, void *key, uint32_t klen, void **value, uint32_t *vlen, void *user);

typedef void *(*hashtable_deep_copy_cb_f)(void *data, uint32_t dlen, void *user);
/**
 * @brief Callback for the pair iterator
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key : The key
 * @param klen : The length of the key
 * @param value : The value
 * @param vlen : The length of the value
 * @param user : The user pointer passed as argument to the ht_foreach_pair() function
 * @return HT_ITERATOR_CONTINUE to go ahead with the iteration,
 *         HT_ITERATOR_STOP to stop the iteration,
 *         HT_ITERATOR_REMOVE to remove the current item from the table and go ahead with the iteration
 *         HT_ITERATOR_REMOVE_AND_STOP to remove the current item from the table and stop the iteration
 */
typedef ht_iterator_status_t (*hashtable_pair_iterator_cb_f)(ds_hashtable_t ptable, void *key, uint32_t klen, void *value, uint32_t vlen, void *user);
/**
 * @brief Callback for the key iterator
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key : The key
 * @param klen : The length of the key
 * @param user : The user pointer passed as argument to the ht_foreach_pair() function
 * @return HT_ITERATOR_CONTINUE to go ahead with the iteration,
 *         HT_ITERATOR_STOP to stop the iteration,
 *         HT_ITERATOR_REMOVE to remove the current item from the table and go ahead with the iteration
 *         HT_ITERATOR_REMOVE_AND_STOP to remove the current item from the table and stop the iteration
 */
typedef ht_iterator_status_t (*hashtable_key_iterator_cb_f)(ds_hashtable_t ptable, void *key, size_t klen, void *user);
/**
 * @brief Callback for the value iterator
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param value : The value
 * @param vlen : The length of the value
 * @param user : The user pointer passed as argument to the ht_foreach_pair() function
 * @return HT_ITERATOR_CONTINUE to go ahead with the iteration,
 *         HT_ITERATOR_STOP to stop the iteration,
 *         HT_ITERATOR_REMOVE to remove the current item from the table and go ahead with the iteration
 *         HT_ITERATOR_REMOVE_AND_STOP to remove the current item from the table and stop the iteration
 */
typedef ht_iterator_status_t (*hashtable_value_iterator_cb_f)(ds_hashtable_t ptable, void *value, size_t vlen, void *user);

typedef struct {
    uint32_t initial_capacity;
    rte_mutex_t *bucket_mutex;
    rte_mutex_t *chain_mutex;
    hashtable_element_free_cb_f free_cb;
} hashtable_configuration_t;

/**
 * @brief Create a new table descriptor
 * @param initial_size : initial size of the table, the table will be expanded if necessary
 * @param configuration: all configuration
 * @param hashtable : a newly allocated and initialized table
 * @return rte_error_t RTE_SUCCESS When created the hash table successfully
 */
extern rte_error_t ht_create(hashtable_configuration_t *configuration, ds_hashtable_t *hashtable);
/**
 * @brief Destroy the table and removing all the stored items
 * @param ptable : A valid pointer to an hashtable_t structure
 *
 * If a free_item_callback has been set, that will be called for each item removed from the table
 */
extern rte_error_t ht_destroy(ds_hashtable_t ptable);


#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 expandtab:
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
