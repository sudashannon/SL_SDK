/**
 * @file ds_hashmap.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-09
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../inc/data_structure/ds_hashmap.h"
#include "../../inc/data_structure/ds_vector.h"
#include "../../inc/middle_layer/rte_atomic.h"
#include "../../inc/middle_layer/rte_log.h"
#include "../../inc/middle_layer/rte_timer.h"
#include "../../inc/middle_layer/rte_memory.h"

#define HT_KEY_EQUALS(_k1, _kl1, _k2, _kl2) \
            (_k1) && (_k1) && \
            (((char *)(_k1))[0] == ((char *)(_k2))[0] && \
            (_kl1) == (_kl2) && \
            memcmp((_k1), (_k2), (_kl1)) == 0)

#define THIS_MODULE LOG_STR(HASHTABLE)

#define HT_ASSERT(v) LOG_ASSERT(THIS_MODULE, v)

typedef struct hashtable_element {
    uint32_t hash;
    char     kbuf[32];
    void    *key;
    uint32_t klen;
    void    *data;
    uint32_t dlen;
    struct   hashtable_element *next;
} hashtable_element_t;

// NOTE : order here matters (and also numbering)
typedef enum {
    CHAIN_STATUS_INIT = 0,
    CHAIN_STATUS_WRITE = 1,
    CHAIN_STATUS_IDLE  = 2,
    CHAIN_STATUS_READ  = 3
} chain_status_t;

typedef struct hashtable_bucket {
    uint32_t hash;
    chain_status_t status;
    hashtable_element_t *first_element;
    hashtable_element_t *last_element;
} hashtable_bucket_t;

typedef struct hashtable_impl {
    uint32_t hashseed;
    uint32_t element_count;
    uint32_t buckets_count;
    hashtable_bucket_t **buckets_array;    // All buckets which can be used in the hashtable, will be maintained manually.
    ds_vector_t chains_array;     // All valid buckets which has been set int the hashtable, will be maintained by vector's API.
    hashtable_element_free_cb_f free_item_cb;
    rte_mutex_t *mutex;
} hashtable_impl_t;

#define HT_LOCK(hashtable)      RTE_LOCK(hashtable->mutex)
#define HT_UNLOCK(hashtable)    RTE_UNLOCK(hashtable->mutex)
// Single chain's lock is the minimal lock of hashtable, we used cas lock here.
#define CHAIN_LOCK(chain)       while(!ATOMIC_CASB(&(chain->status), CHAIN_STATUS_IDLE, CHAIN_STATUS_WRITE)) rte_yield();
#define CHAIN_UNLOCK(chain)     ATOMIC_SET(&(chain->status), CHAIN_STATUS_IDLE)


#define LINK_LIST_RMV_NODE(head, tail, prev, node, next)        \
do {                                                            \
    if (head == node) {                                         \
        head = NULL;                                            \
        tail = NULL;                                            \
    } else if (tail == node) {                                  \
        tail = NULL;                                            \
    }                                                           \
    if (prev) {                                                 \
        HT_ASSERT(prev->next == node);                          \
        prev->next = next;                                      \
    }                                                           \
} while(0)

#define LINK_LIST_ADD_NODE_TO_TAIL(head, tail, node)            \
do {                                                            \
    if (head == NULL) {                                         \
        head = node;                                            \
        tail = node;                                            \
    } else {                                                    \
        tail->next = node;                                      \
        tail = node;                                            \
    }                                                           \
} while(0)

#define HT_FREE_ITEM(table, item)                               \
do {                                                            \
    if (table->free_item_cb)                                    \
        table->free_item_cb(item->data);                        \
    if (item->key != item->kbuf)                                \
        rte_free(item->key);           \
    rte_free(item);                    \
} while(0)

/**
 * @brief Internal function to calculate the hashvalue.
 *
 * @param table
 * @param str
 * @param len
 * @return uint32_t
 */
static inline uint32_t ht_hash_function(hashtable_impl_t *table, const unsigned char *str, uint32_t len)
{
    const unsigned char * const end = (const unsigned char *)str + len;
    uint32_t hash = table->hashseed + len;
    while (str < end) {
        hash += *str++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    return (hash + (hash << 15));
}

/**
 * @brief Create a new table descriptor
 * @param initial_size : initial size of the table, the table will be expanded if necessary
 * @param configuration: all configuration
 * @param hashtable : a newly allocated and initialized table
 * @return rte_error_t RTE_SUCCESS When created the hash table successfully
 */
rte_error_t ht_create(hashtable_configuration_t *configuration, ds_hashtable_t *hashtable)
{
    if (RTE_UNLIKELY(hashtable == NULL)||
        RTE_UNLIKELY(configuration == NULL))
        return RTE_ERR_PARAM;
    hashtable_impl_t *table = (hashtable_impl_t *)rte_calloc(sizeof(hashtable_impl_t));
    if (!table) {
        return RTE_ERR_NO_MEM;
    }
    table->hashseed = rand() % UINT32_MAX;
    table->free_item_cb = configuration->free_cb;
    // The bucket array will hold all bucket instance and be expanded by the hashmap module.
    table->mutex = configuration->bucket_mutex;
    table->buckets_count = rte_roundup_pow_of_two(configuration->initial_capacity);
    table->buckets_array = rte_calloc(sizeof(hashtable_bucket_t *) * table->buckets_count);
    // The chain array will be expanded by the vector module, only hold the using bucket.
    vector_configuration_t chain_configuration = VECTOR_CONFIG_INITIALIZER;
    chain_configuration.if_deep_copy = 0;
    chain_configuration.if_expand = 1;
    chain_configuration.element_size = sizeof(void *);
    chain_configuration.capacity = configuration->initial_capacity;
    chain_configuration.free_cb = NULL;
    chain_configuration.mutex = configuration->chain_mutex;
    ds_vector_create(&chain_configuration, &table->chains_array);
    if (table->buckets_array == NULL ||
        table->chains_array == NULL) {
        if (table->buckets_array)
            rte_free(table->buckets_array);
        if (table->chains_array)
            ds_vector_destroy(table->chains_array);
        rte_free(table);
        return RTE_ERR_NO_RSRC;
    }

    *hashtable = table;
    return RTE_SUCCESS;
}
/**
 * @brief Clear the table by removing all the stored items
 * @param ptable : A valid pointer to an hashtable_t structure
 *
 * If a free_item_callback has been set, that will be called for each item removed from the table
 */
rte_error_t ht_clear(ds_hashtable_t ptable)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    // Poll for each bucket
    HT_LOCK(table);
    ATOMIC_SET(&table->element_count, 0);
    memset(table->buckets_array, 0, sizeof(hashtable_bucket_t *) * table->buckets_count);
    HT_UNLOCK(table);

    ds_vector_lock(table->chains_array);
    uint32_t index = 0;
    hashtable_bucket_t *hashchain = NULL;
    // Poll for each chain
    VECTOR_FOR_EACH_SAFELY(index, hashchain, table->chains_array) {
        CHAIN_LOCK(hashchain);
        hashtable_element_t *item = NULL;
        hashtable_element_t *next = NULL;
        // Poll for each element on one chain
        for (item = hashchain->first_element; item != NULL; )  {
            next = item->next;
            HT_FREE_ITEM(table, item);
            item = next;
        }
        rte_free(hashchain);
    }
    ds_vector_unlock(table->chains_array);
    ds_vector_clear(table->chains_array);
    return RTE_SUCCESS;
}
/**
 * @brief Destroy the table and removing all the stored items
 * @param ptable : A valid pointer to an hashtable_t structure
 *
 * If a free_item_callback has been set, that will be called for each item removed from the table
 */
rte_error_t ht_destroy(ds_hashtable_t ptable)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    ht_clear(ptable);

    HT_LOCK(table);
    rte_mutex_t *tmp_mutex = table->mutex;
    rte_free(table->buckets_array);
    ds_vector_destroy(table->chains_array);
    rte_free(table);
    RTE_UNLOCK(tmp_mutex);
    return RTE_SUCCESS;
}

static hashtable_bucket_t *ht_get_bucket(
    hashtable_impl_t *table,
    uint32_t hashvalue)
{
    uint32_t index = hashvalue & (table->buckets_count - 1);
    // Assume we can access the element safely.
    hashtable_bucket_t *bucket = table->buckets_array[index];
    return bucket;
}

static hashtable_bucket_t *ht_set_bucket(
    hashtable_impl_t *table,
    uint32_t hashvalue)
{
    uint32_t index = hashvalue  & (table->buckets_count - 1);
    // Check if the chain has already been set.
    hashtable_bucket_t *chain = table->buckets_array[index];
    if (chain) {
        return chain;
    }
    // Create a new bucket here.
    hashtable_bucket_t *bucket = rte_calloc(sizeof(hashtable_bucket_t));
    if (!bucket) {
        return NULL;
    }
    // Init the bucket.
    ATOMIC_SET(&bucket->status, CHAIN_STATUS_IDLE);
    bucket->hash = hashvalue;
    bucket->first_element = NULL;
    bucket->last_element = NULL;
    // Mark the bucket using.
    table->buckets_array[index] = bucket;
    // Push the bucket into iterator chain array.
    HT_ASSERT(ds_vector_push(table->chains_array, bucket) == RTE_SUCCESS);
    return bucket;
}

static void ht_grow_internal(hashtable_impl_t *table)
{
    HT_LOCK(table);
    table->buckets_count = table->buckets_count << 1;
    rte_free(table->buckets_array);
    table->buckets_array = rte_calloc(sizeof(hashtable_bucket_t *) * table->buckets_count);
    HT_UNLOCK(table);

    uint32_t index = 0;
    hashtable_bucket_t *hashchain = NULL;
    ds_vector_lock(table->chains_array);
    // Poll for each chain and put them into new position
    VECTOR_FOR_EACH_SAFELY(index, hashchain, table->chains_array) {
        table->buckets_array[hashchain->hash & (table->buckets_count - 1)] = hashchain;
    }
    ds_vector_unlock(table->chains_array);
}

static rte_error_t ht_set_internal(
    hashtable_impl_t *table,
    void *key,
    uint32_t klen,
    void *data,
    uint32_t dlen,
    void **prev_data,
    uint32_t *prev_len,
    bool if_copy,
    bool if_inx)
{
    if (!klen)
        return RTE_ERR_PARAM;

    uint32_t hashvalue = ht_hash_function(table, key, klen);
    HT_LOCK(table);
    if (RTE_UNLIKELY(table->buckets_array == NULL) ||
        RTE_UNLIKELY(table->chains_array == NULL)) {
        HT_UNLOCK(table);
        return RTE_ERR_PARAM;
    }
    // let's first try checking if we fall in an existing bucket list
    hashtable_bucket_t *bucket  = ht_get_bucket(table, hashvalue);
    // if not, let's create a new bucket list
    if (!bucket)
        bucket = ht_set_bucket(table, hashvalue);
    if (!bucket) {
        HT_UNLOCK(table);
        return RTE_ERR_NO_RSRC;
    }
    HT_UNLOCK(table);

    // Lock the chain to write some data into it.
    CHAIN_LOCK(bucket);
    // Poll the chain and check if the chain already has the element.
    void *pdata = NULL;
    uint32_t plen = 0;
    hashtable_element_t *item = NULL;
    bool if_found = false;
    for (item = bucket->first_element; item != NULL; item = item->next)  {
        if (HT_KEY_EQUALS(item->key, item->klen, key, klen)) {
            pdata = item->data;
            plen = item->dlen;
            if_found = true;
            break;
        }
    }
    if (if_found == false) {
        // If not found, create a new element and attach the element to the chain.
        item = (hashtable_element_t *)rte_calloc(sizeof(hashtable_element_t));
        if (!item) {
            CHAIN_UNLOCK(bucket);
            return RTE_ERR_NO_MEM;
        }
        item->hash = hashvalue;
        item->klen = klen;
        item->next = NULL;
        // Check if default key buffer is enough for the user's key.
        if (klen > sizeof(item->kbuf)) {
            item->key = rte_malloc(klen);
            if (!item->key) {
                rte_free(item);
                CHAIN_UNLOCK(bucket);
                return RTE_ERR_NO_MEM;
            }
        } else {
            item->key = item->kbuf;
        }
        // Copy key.
        memcpy(item->key, key, klen);
        // Check if the data needs to be copied.
        if (if_copy) {
            if (dlen) {
                item->data = rte_malloc(dlen);
                if (!item->data) {
                    if (klen > sizeof(item->kbuf))
                        rte_free(item->key);
                    rte_free(item);
                    CHAIN_UNLOCK(bucket);
                    return RTE_ERR_NO_MEM;
                }
                memcpy(item->data, data, dlen);
            } else {
                item->data = NULL;
            }
        } else {
            item->data = data;
        }
        item->dlen = dlen;
        // Add the item to the link list
        LINK_LIST_ADD_NODE_TO_TAIL(bucket->first_element, bucket->last_element, item);
        ATOMIC_INCREMENT(&table->element_count);
    } else {
        if (if_inx) {
            if (prev_data)
                *prev_data = pdata;
            if (prev_len)
                *prev_len = plen;
            CHAIN_UNLOCK(bucket);
            return RTE_SUCCESS;
        }
        // Check if the data needs to be copied.
        if (if_copy) {
            if (dlen) {
                item->data = rte_realloc(item->data, dlen);
                if (!item->data) {
                    CHAIN_UNLOCK(bucket);
                    return RTE_ERR_NO_MEM;
                }
                memcpy(item->data, data, dlen);
            } else {
                rte_free(item->data);
                item->data = NULL;
            }
        } else {
            item->data = data;
        }
        item->dlen = dlen;
    }
    CHAIN_UNLOCK(bucket);

    uint32_t current_size = table->buckets_count;
    if (ATOMIC_READ(&table->element_count) > (current_size + (current_size/3))) {
        ht_grow_internal(table);
    }

    if (if_found) {
        if (prev_data)
            *prev_data = pdata;
        else if (table->free_item_cb)
            table->free_item_cb(pdata);
    } else if (prev_data) {
        *prev_data = NULL;
    }

    if (prev_len)
        *prev_len = plen;
    return RTE_SUCCESS;
}

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
rte_error_t ht_set(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *data, uint32_t dlen)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    return ht_set_internal(table, key, klen, data, dlen, NULL, NULL, false, false);
}
/**
 * @brief Set the value for a specific key if there is no value already stored
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key   : The key to use
 * @param klen  : The length of the key
 * @param data  : A pointer to the data to store
 * @param dlen  : The size of the data
 * @return rte_error_t
 */
rte_error_t ht_set_if_not_exists(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *data, uint32_t dlen)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    return ht_set_internal(table, key, klen, data, dlen, NULL, NULL, false, true);
}
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
rte_error_t ht_get_or_set(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *data, uint32_t dlen,
    void **cur_data, uint32_t *cur_len)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    return ht_set_internal(table, key, klen, data, dlen, cur_data, cur_len, false, true);
}
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
rte_error_t ht_get_and_set(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *data, uint32_t dlen,
    void **prev_data, uint32_t *prev_len)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    return ht_set_internal(table, key, klen, data, dlen, prev_data, prev_len, false, false);
}
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
rte_error_t ht_set_copy(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *data, uint32_t dlen,
    void **prev_data, uint32_t *prev_len)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    return ht_set_internal(table, key, klen, data, dlen, prev_data, prev_len, true, false);
}

static rte_error_t ht_call_internal(
    hashtable_impl_t *table,
    void *key, uint32_t klen,
    hashtable_pair_cb_f cb,
    void *user)
{
    uint32_t hashvalue = ht_hash_function(table, key, klen);

    HT_LOCK(table);
    if (RTE_UNLIKELY(table->buckets_array == NULL) ||
        RTE_UNLIKELY(table->chains_array == NULL)) {
        HT_UNLOCK(table);
        return RTE_ERR_PARAM;
    }
    hashtable_bucket_t *bucket  = ht_get_bucket(table, hashvalue);
    if (!bucket) {
        HT_UNLOCK(table);
        return RTE_ERR_NO_RSRC;
    }
    HT_UNLOCK(table);

    CHAIN_LOCK(bucket);
    rte_error_t ret = RTE_SUCCESS;
    hashtable_element_t *item = NULL;
    hashtable_element_t *prev = NULL;
    hashtable_element_t *next = NULL;
    for (item = bucket->first_element; item != NULL; )  {
        next = item->next;
        if (HT_KEY_EQUALS(item->key, item->klen, key, klen)) {
            if (cb) {
                ret = cb(table, key, klen, &item->data, &item->dlen, user);
                if (ret == 1) {
                    LINK_LIST_RMV_NODE(bucket->first_element, bucket->last_element, prev, item, next);
                    HT_FREE_ITEM(table, item);
                    ATOMIC_DECREMENT(&table->element_count);
                }
            }
            break;
        }
        prev = item;
        item = next;
    }
    CHAIN_UNLOCK(bucket);
    return ret;
}
/**
 * @brief call the provided callback passing the item stored at the specified key (if any)
 * @param table : A valid pointer to an hashtable_t structure
 * @param key : The key to use
 * @param klen : The length of the key
 * @param cb : The callback
 * @param user : A private pointer which will be passed to the callback when invoked
 * @note the callback is called while the bucket-level mutex is being retained
 */
rte_error_t ht_call(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    hashtable_pair_cb_f cb,
    void *user)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    return ht_call_internal(table, key, klen, cb, user);
}

typedef struct {
    void *data;
    uint32_t dlen;
    void *match;
    uint32_t match_size;
    bool if_matched;
    void **prev_data;
    uint32_t *prev_len;
} ht_set_if_equals_helper_arg_t;

static rte_error_t
ht_set_if_equals_helper(
    ds_hashtable_t ptable,
    void *key __attribute__ ((unused)),
    uint32_t klen __attribute__ ((unused)),
    void **value, uint32_t *vlen, void *user)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    ht_set_if_equals_helper_arg_t *arg = (ht_set_if_equals_helper_arg_t *)user;

    if (arg->prev_len)
        *arg->prev_len = *vlen;

    if (arg->prev_data)
        *arg->prev_data = *value;

    if (arg->match_size == *vlen &&
        ((char *)*value)[0] == *((char *)arg->match) &&
        memcmp(*value, arg->match, arg->match_size) == 0) {
        arg->if_matched = true;

        if (!arg->prev_data && table->free_item_cb)
            table->free_item_cb(*value);

        *value = arg->data;
        *vlen = arg->dlen;
    }

    return RTE_SUCCESS;
}
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
rte_error_t ht_set_if_equals(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *data, uint32_t dlen,
    void *match, uint32_t match_size,
    void **prev_data, uint32_t *prev_len)
{
    if (!match && match_size == 0)
        return ht_set_if_not_exists(ptable, key, klen, data, dlen);

    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    ht_set_if_equals_helper_arg_t arg = {
        .data = data,
        .dlen = dlen,
        .match = match,
        .match_size = match_size,
        .if_matched = false,
        .prev_data = prev_data,
        .prev_len = prev_len
    };
    ht_call_internal(table, key, klen, ht_set_if_equals_helper, (void *)&arg);
    return arg.if_matched ? RTE_SUCCESS : RTE_ERR_MISMATCH;
}

typedef struct
{
    bool if_unset;
    void **prev_data;
    uint32_t *prev_len;
    void *match;
    uint32_t match_size;
} ht_delete_helper_arg_t;

static rte_error_t
ht_delete_helper(
    ds_hashtable_t ptable,
    void *key __attribute__ ((unused)),
    uint32_t klen __attribute__ ((unused)),
    void **value, uint32_t *vlen, void *user)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    ht_delete_helper_arg_t *arg = (ht_delete_helper_arg_t *)user;

    if (arg->match && (arg->match_size != *vlen ||
        memcmp(arg->match, *value, *vlen) != 0))
        return RTE_ERR_MISMATCH;

    if (arg->prev_data)
        *arg->prev_data = *value;
    else if (table->free_item_cb)
        table->free_item_cb(*value);

    if (arg->prev_len)
        *arg->prev_len = *vlen;

    if (arg->if_unset) {
        *vlen = 0;
        *value = NULL;
        return RTE_SUCCESS;
    }

    return 1; // we want the item to be removed
}
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
rte_error_t ht_unset(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void **prev_data, uint32_t *prev_len)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    ht_delete_helper_arg_t arg = {
        .if_unset = true,
        .prev_data = prev_data,
        .prev_len = prev_len,
        .match = NULL,
        .match_size = 0
    };

    rte_error_t result =  ht_call_internal(table, key, klen, ht_delete_helper, (void *)&arg) >= 0;
    return result >= 0 ? RTE_SUCCESS : result;
}
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
rte_error_t ht_delete(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void **prev_data, uint32_t *prev_len)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    ht_delete_helper_arg_t arg = {
        .if_unset = false,
        .prev_data = prev_data,
        .prev_len = prev_len,
        .match = NULL,
        .match_size = 0
    };

    rte_error_t result =  ht_call_internal(table, key, klen, ht_delete_helper, (void *)&arg);
    return result >= 0 ? RTE_SUCCESS : result;
}
/**
 * @brief Delete the value stored at a specific key only if it matches some provided data
 * @param table : A valid pointer to an hashtable_t structure
 * @param key : The key to use
 * @param klen : The length of the key
 * @param match : A valid pointer to the data we need to match in order to delete the value
 * @param match_size : The value of the data to match
 * @return 0 on success, -1 otherwise
 */
rte_error_t ht_delete_if_equals(
    ds_hashtable_t ptable,
    void *key, uint32_t klen,
    void *match, uint32_t match_size)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    ht_delete_helper_arg_t arg = {
        .if_unset = false,
        .prev_data = NULL,
        .prev_len = NULL,
        .match = match,
        .match_size = match_size
    };

    rte_error_t result =  ht_call_internal(table, key, klen, ht_delete_helper, (void *)&arg) >= 0;
    return result >= 0 ? RTE_SUCCESS : result;
}
/**
 * @brief Check if a key exists in the hashtable
 * @param table : A valid pointer to an hashtable_t structure
 * @param key   : The key to use
 * @param klen  : The length of the key
 * @return 1 If the key exists, 0 if it doesn't exist and -1 in case of error
 */
rte_error_t ht_exists(ds_hashtable_t ptable, void *key, uint32_t klen)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return RTE_ERR_PARAM;
    return ht_call_internal(table, key, klen, NULL, NULL);
}

typedef struct {
    void *data;
    uint32_t *dlen;
    bool if_copy;
    hashtable_deep_copy_cb_f copy_cb;
    void *user;
} ht_get_helper_arg_t;

static rte_error_t
ht_get_helper(
    ds_hashtable_t ptable __attribute__ ((unused)),
    void *key __attribute__ ((unused)),
    uint32_t klen __attribute__ ((unused)),
    void **value, uint32_t *vlen, void *user)
{
    ht_get_helper_arg_t *arg = (ht_get_helper_arg_t *)user;

    if (arg->if_copy) {
        if (arg->copy_cb) {
            arg->data = arg->copy_cb(*value, *vlen, arg->user);
        } else {
            arg->data = rte_malloc(*vlen);
            if (!arg->data)
                return RTE_ERR_NO_MEM;
            memcpy(arg->data, *value, *vlen);
        }
    } else {
        arg->data = *value;
    }

    if (arg->dlen)
        *arg->dlen = *vlen;

    return RTE_SUCCESS;
}
/**
 * @brief Get the value stored at a specific key
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param key   : The key to use
 * @param klen  : The length of the key
 * @param dlen  : If not NULL, the size of the returned data will be stored
 *                at the address pointed by dlen
 * @return The stored value if any, NULL otherwise
 */
void *ht_get(ds_hashtable_t ptable, void *key, uint32_t klen, uint32_t *dlen)
{
   hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return NULL;
    ht_get_helper_arg_t arg = {
        .data = NULL,
        .dlen = dlen,
        .if_copy = false,
        .copy_cb = NULL,
        .user = NULL
    };

    ht_call_internal(table, key, klen, ht_get_helper, (void *)&arg);
    return arg.data;
}
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
void *ht_get_copy(ds_hashtable_t ptable, void *key, uint32_t klen, uint32_t *dlen)
{
   hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return NULL;
    ht_get_helper_arg_t arg = {
        .data = NULL,
        .dlen = dlen,
        .if_copy = true,
        .copy_cb = NULL,
        .user = NULL
    };

    ht_call_internal(table, key, klen, ht_get_helper, (void *)&arg);
    return arg.data;
}
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
void *ht_get_deep_copy(
    ds_hashtable_t ptable, void *key, uint32_t klen,
    uint32_t *dlen, hashtable_deep_copy_cb_f copy_cb, void *user)
{
   hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return NULL;
    ht_get_helper_arg_t arg = {
        .data = NULL,
        .dlen = dlen,
        .if_copy = true,
        .copy_cb = copy_cb,
        .user = user
    };

    ht_call_internal(table, key, klen, ht_get_helper, (void *)&arg);
    return arg.data;
}
/**
 * @brief Pair iterator
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param cb    : an ht_pair_iterator_callback_t function
 * @param user  : A pointer which will be passed to the iterator callback at each call
 */
void ht_foreach_pair(
    ds_hashtable_t ptable, hashtable_pair_iterator_cb_f cb, void *user)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return;

    HT_LOCK(table);
    if (RTE_UNLIKELY(table->buckets_array == NULL) ||
        RTE_UNLIKELY(table->chains_array == NULL)) {
        HT_UNLOCK(table);
        return;
    }
    HT_UNLOCK(table);

    ht_iterator_status_t result = 0;
    uint32_t index = 0;
    hashtable_bucket_t *hashchain = NULL;
    ds_vector_lock(table->chains_array);
    // Poll for each chain and poll for their elements
    VECTOR_FOR_EACH_SAFELY(index, hashchain, table->chains_array) {
        CHAIN_LOCK(hashchain);
        hashtable_element_t *item = NULL;
        hashtable_element_t *prev = NULL;
        hashtable_element_t *next = NULL;
        // Poll for each element on one chain
        for (item = hashchain->first_element; item != NULL; )  {
            next = item->next;
            result = cb(table, item->key, item->klen, item->data, item->dlen, user);
            if (result != HT_ITERATOR_CONTINUE)
                break;
            prev = item;
            item = next;
        }
        // Check the result and decide what to do for next poll
        if (item) {
            if (result == HT_ITERATOR_STOP) {
                CHAIN_UNLOCK(hashchain);
                break;
            } else {
                // result is either HT_ITERATOR_REMOVE or HT_ITERATOR_REMOVE_AND_STOP
                LINK_LIST_RMV_NODE(hashchain->first_element, hashchain->last_element, prev, item, next);
                HT_FREE_ITEM(table, item);
                ATOMIC_DECREMENT(&table->element_count);
                if (result == HT_ITERATOR_REMOVE_AND_STOP) {
                    CHAIN_UNLOCK(hashchain);
                    break;
                }
            }
        }
        CHAIN_UNLOCK(hashchain);
    }
    ds_vector_unlock(table->chains_array);
}

typedef struct {
    int (*cb)();
    void *user;
} ht_iterator_arg_t;

static ht_iterator_status_t
ht_foreach_key_helper(
    ds_hashtable_t ptable, void *key, uint32_t klen,
    void *value __attribute__ ((unused)), uint32_t vlen __attribute__ ((unused)), void *user)
{
    ht_iterator_arg_t *arg = (ht_iterator_arg_t *)user;
    return arg->cb(ptable, key, klen, arg->user);
}
/**
 * @brief Key iterator
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param cb    : an ht_key_iterator_callback_t function
 * @param user  : A pointer which will be passed to the iterator callback at each call
 */
void
ht_foreach_key(ds_hashtable_t ptable, hashtable_key_iterator_cb_f cb, void *user)
{
    ht_iterator_arg_t arg = { cb, user };
    ht_foreach_pair(ptable, ht_foreach_key_helper, &arg);
}

static ht_iterator_status_t
ht_foreach_value_helper(
    ds_hashtable_t ptable, void *key __attribute__ ((unused)),
    uint32_t klen __attribute__ ((unused)), void *value, uint32_t vlen, void *user)
{
    ht_iterator_arg_t *arg = (ht_iterator_arg_t *)user;
    return arg->cb(ptable, value, vlen, arg->user);
}
/**
 * @brief Value iterator
 * @param ptable : A valid pointer to an hashtable_t structure
 * @param cb    : an ht_value_iterator_callback_t function
 * @param user  : A pointer which will be passed to the iterator callback at each call
 */
void
ht_foreach_value(ds_hashtable_t ptable, hashtable_value_iterator_cb_f cb, void *user)
{
    ht_iterator_arg_t arg = { cb, user };
    ht_foreach_pair(ptable, ht_foreach_value_helper, &arg);
}
/**
 * @brief Return the count of items actually stored in the table
 * @param ptable : A valid pointer to an hashtable_t structure
 * @return The actual item count
 */
uint32_t ht_count(ds_hashtable_t ptable)
{
    hashtable_impl_t *table = RTE_CAST(hashtable_impl_t *, ptable);
    if (RTE_UNLIKELY(table == NULL))
        return 0;
    return ATOMIC_READ(&table->element_count);
}
