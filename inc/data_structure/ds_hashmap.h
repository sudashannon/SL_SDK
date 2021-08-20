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
 * @brief Clear the table by removing all the stored items
 * @param ptable : A valid pointer to an hashtable_t structure
 *
 * If a free_item_callback has been set, that will be called for each item removed from the table
 */
extern rte_error_t ht_clear(ds_hashtable_t ptable);
/**
 * @brief Destroy the table and removing all the stored items
 * @param ptable : A valid pointer to an hashtable_t structure
 *
 * If a free_item_callback has been set, that will be called for each item removed from the table
 */
extern rte_error_t ht_destroy(ds_hashtable_t ptable);
/**
 * @brief Set the value for a specific key
 *
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key   : The key to use
 * @param klen  : The length of the key
 * @param data  : A pointer to the data to store
 * @param dlen  : The size of the data
 * @return rte_error_t
 */
extern rte_error_t ht_set(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *data, uint32_t dlen);
/**
 * @brief Set the value for a specific key if there is no value already stored
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key   : The key to use
 * @param klen  : The length of the key
 * @param data  : A pointer to the data to store
 * @param dlen  : The size of the data
 * @return rte_error_t
 */
extern rte_error_t ht_set_if_not_exists(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *data, uint32_t dlen);
/**
 * @brief Get the value for a specific key or set a new value if none has been found
 * @param ptable    : A valid pointer to an hashtable_t structure
 * @param key      : The key to use
 * @param klen     : The length of the key
 * @param data     : A pointer to the new data to store if none is found
 * @param dlen     : The size of the data to store
 * @param cur_data : If not NULL, the referenced pointer will be set to point to the current data
 * @param cur_len  : If not NULL, the size of the current data will be stored in the memory
 *                    pointed by cur_len
 * @return rte_error_t
 */
extern rte_error_t ht_get_or_set(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *data, uint32_t dlen,
    void **cur_data, uint32_t *cur_len);
/**
 * @brief Set the value for a specific key and returns the previous value if any
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key   : The key to use
 * @param klen  : The length of the key
 * @param data  : A pointer to the data to store
 * @param dlen  : The size of the data
 * @param prev_data : If not NULL, the referenced pointer will be set to point to the previous data
 * @param prev_len  : If not NULL, the size of the previous data will be stored in the memory
 *                    pointed by prev_len
 * @return rte_error_t
 * @note If prev_data is not NULL, the previous data will not be released using the free_value callback
 *       so the caller will be responsible of releasing the previous data once done with it
 */
extern rte_error_t ht_get_and_set(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *data, uint32_t dlen,
    void **prev_data, uint32_t *prev_len);
/**
 * @brief Set the value for a specific key and returns the previous value if any.
 *
 *        The new value will be copied before being stored
 *
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key   : The key to use
 * @param klen  : The length of the key
 * @param data  : A pointer to the data to store
 * @param dlen  : The size of the data
 * @param prev_data : If not NULL, the referenced pointer will be set to point to the previous data
 * @param prev_len  : If not NULL, the size of the previous data will be stored in the memory
 *                    pointed by prev_len
 * @return rte_error_t
 * @note If prev_data is not NULL, the previous data will not be released using the free_value callback
 *       so the caller will be responsible of releasing the previous data once done with it
 */
extern rte_error_t ht_set_copy(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *data, uint32_t dlen,
    void **prev_data, uint32_t *prev_len);
/**
 * @brief call the provided callback passing the item stored at the specified key (if any)
 * @param table : A valid pointer to an hashtable_t structure
 * @param key : The key to use
 * @param klen : The length of the key
 * @param cb : The callback
 * @param user : A private pointer which will be passed to the callback when invoked
 * @note the callback is called while the bucket-level mutex is being retained
 */
extern rte_error_t ht_call(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    hashtable_pair_cb_f cb,
    void *user);
/**
 * @brief Set a new value stored at a specific key only if the actual one matches some provided data
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key : The key to use
 * @param klen : The length of the key
 * @param data  : A pointer to the data to store
 * @param dlen  : The size of the data
 * @param match : A valid pointer to the data we need to match in order to delete the value
 * @param match_size : The value of the data to match
 * @param prev_data : If not NULL the pointer will be set to point to the previous data
 * @param prev_len : If not NULL the integer pointer will be set to the size of the previous data
 * @node If the prev_data pointer is provided, the caller will be responsible of relasing
 *       the resources pointed after the call. If not provided (NULL) the free_value callback
 *       will be eventually used (if defined)
 * @return rte_error_t
 */
extern rte_error_t ht_set_if_equals(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *data, uint32_t dlen,
    void *match, uint32_t match_size,
    void **prev_data, uint32_t *prev_len);
/**
 * @brief Unset the value stored at a specific key
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key : The key to use
 * @param klen : The length of the key
 * @param prev_data : If not NULL, the referenced pointer will be set to point to the previous data
 * @param prev_len  : If not NULL, the size of the previous data will be stored in the memory
 *                    pointed by prev_len
 * @return The previous value if any, NULL otherwise
 * @note If prev_data is not NULL, the previous data will not be released using the free_value callback
 *       so the caller will be responsible of releasing the previous data once done with it
 */
extern rte_error_t ht_unset(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void **prev_data, uint32_t *prev_len);
/**
 * @brief Delete the value stored at a specific key
 * @param table : A valid pointer to an hashtable_t structure
 * @param key : The key to use
 * @param klen : The length of the key
 * @param prev_data : If not NULL, the referenced pointer will be set to point to the previous data
 * @param prev_len  : If not NULL, the size of the previous data will be stored in the memory
 *                    pointed by prev_len
 * @return 0 on success, -1 otherwise
 * @note If prev_data is not NULL, the previous data will not be released using the free_value callback
 *       so the caller will be responsible of releasing the previous data once done with it
 */
extern rte_error_t ht_delete(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void **prev_data, uint32_t *prev_len);
/**
 * @brief Delete the value stored at a specific key only if it matches some provided data
 * @param table : A valid pointer to an hashtable_t structure
 * @param key : The key to use
 * @param klen : The length of the key
 * @param match : A valid pointer to the data we need to match in order to delete the value
 * @param match_size : The value of the data to match
 * @return 0 on success, -1 otherwise
 */
extern rte_error_t ht_delete_if_equals(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *match, uint32_t match_size);
/**
 * @brief Check if a key exists in the hashtable
 * @param table : A valid pointer to an hashtable_t structure
 * @param key   : The key to use
 * @param klen  : The length of the key
 * @return 1 If the key exists, 0 if it doesn't exist and -1 in case of error
 */
extern rte_error_t ht_exists(ds_hashtable_t ptable, void *key, uint32_t klen);
/**
 * @brief Get the value stored at a specific key
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key   : The key to use
 * @param klen  : The length of the key
 * @param dlen  : If not NULL, the size of the returned data will be stored
 *                at the address pointed by dlen
 * @return The stored value if any, NULL otherwise
 */
extern void *ht_get(ds_hashtable_t ptable, void *key, uint32_t klen, uint32_t *dlen);
/**
 * @brief Get a copy of the value stored at a specific key
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key   : The key to use
 * @param klen  : The length of the key
 * @param dlen  : If not NULL, the size of the returned data will be stored
 *                at the address pointed by dlen
 * @return The stored value if any, NULL otherwise
 * @note The returned value is a copy (memcpy) of the stored value and the
 *       caller MUST release it using free() once done
 *
 * @note The copy is a simple copy done using memcpy() if the stored value
 *       is structured and requires a deep copy, then ht_get_deep_copy()
 *       should be used instead of this function
 */
extern void *ht_get_copy(ds_hashtable_t ptable, void *key, uint32_t klen, uint32_t *dlen);
/**
 * @brief Get a copy of the value stored at a specific key
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key   : The key to use
 * @param klen  : The length of the key
 * @param dlen  : If not NULL, the size of the returned data will be stored
 *               at the address pointed by dlen
 * @param copy_cb : The callback which will take care of deep-copying the data
 * @param user    : A private pointer which will be passed back to the copy_cb
 * @return The stored value if any, NULL otherwise
 * @note The returned value is eventually created by the deep_copy callback
 *       hence the caller knows if memory will need to be disposed or not and
 *       how to fully release the structured value which has been deep copied
 */
extern void *ht_get_deep_copy(
    ds_hashtable_t ptable, void *key, uint32_t klen,
    uint32_t *dlen, hashtable_deep_copy_cb_f copy_cb, void *user);
/**
 * @brief Pair iterator
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param cb    : an ht_pair_iterator_callback_t function
 * @param user  : A pointer which will be passed to the iterator callback at each call
 */
extern void ht_foreach_pair(
    ds_hashtable_t ptable, hashtable_pair_iterator_cb_f cb, void *user);
/**
 * @brief Key iterator
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param cb    : an ht_key_iterator_callback_t function
 * @param user  : A pointer which will be passed to the iterator callback at each call
 */
extern void
ht_foreach_key(ds_hashtable_t ptable, hashtable_key_iterator_cb_f cb, void *user);
/**
 * @brief Value iterator
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param cb    : an ht_value_iterator_callback_t function
 * @param user  : A pointer which will be passed to the iterator callback at each call
 */
extern void
ht_foreach_value(ds_hashtable_t ptable, hashtable_value_iterator_cb_f cb, void *user);
/**
 * @brief Return the count of items actually stored in the table
 * @param ptable : A valid pointer to an hashtable_t structure
 * @return The actual item count
 */
extern uint32_t ht_count(ds_hashtable_t ptable);
#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 expandtab:
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
