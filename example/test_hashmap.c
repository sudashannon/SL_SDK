#include <stdio.h>
#include "rte_include.h"
#include "SDL.h"

typedef struct {
    int start;
    int end;
    ds_hashtable_t table;
} parallel_insert_arg;

static int parallel_insert(void *user) {
    parallel_insert_arg *arg = (parallel_insert_arg *)user;
    int i;
    for (i = arg->start; i <= arg->end; i++) {
        char k[21];
        sprintf(k, "%d", i);
        char *v = malloc(100);
        sprintf(v, "test%d", i+1);
        ht_set(arg->table, (void *)k, strlen(k), v, strlen(v));
    }
    return 0;
}

int remove_item(ds_hashtable_t table, void *key, uint32_t klen, void *value, uint32_t vlen, void *user) {
    char *key_to_remove = (char *)user;
    size_t key_len = strlen(key_to_remove);
    if (klen == key_len && memcmp(key_to_remove, key, klen) == 0) {
        return HT_ITERATOR_REMOVE_AND_STOP;
    }
    return HT_ITERATOR_CONTINUE;
}

int check_item(ds_hashtable_t table, void *key, uint32_t klen, void *value, uint32_t vlen, void *user) {
    int *check_item_count = (int *)user;
    char test[25];
    char keystr[klen+1];
    memcpy(keystr, key, klen);
    keystr[klen] = 0;
    int num = atoi(keystr);
    sprintf(test, "test%d", num+1);
    if (strlen(test) == vlen && memcmp(test, value, vlen) == 0)
        (*check_item_count)++;
    static int last_count = 0;
    int count = *check_item_count/1000;
    if(count % 10 == 0 && count != last_count) {
        RTE_LOGI("check item count %d", count);
        last_count = count;
    }
    return HT_ITERATOR_CONTINUE;
}

static int free_count = 0;

void free_item(void *item) {
    free(item);
    free_count++;
    static int last_count = 0;
    int count = free_count/1000;
    if(count % 10 == 0 && count != last_count) {
        RTE_LOGI("check item count %d", count);
        last_count = count;
    }
}

void test_hashmap(void)
{
    int i;
    rte_mutex_t hashmap_mutex[2] = {
        {
            .mutex = NULL,
            .lock = rte_mutex_lock,
            .unlock = rte_mutex_unlock,
            .trylock = NULL
        },
        {
            .mutex = NULL,
            .lock = rte_mutex_lock,
            .unlock = rte_mutex_unlock,
            .trylock = NULL
        }
    };
    hashmap_mutex[0].mutex = SDL_CreateMutex();
    hashmap_mutex[1].mutex = SDL_CreateMutex();
    hashtable_configuration_t hashmap_config = {
        .initial_capacity = HASHTABLE_MIN_CAPACITY,
        .bucket_mutex = &hashmap_mutex[0],
        .chain_mutex = &hashmap_mutex[1],
        .free_cb = NULL
    };
    ds_hashtable_t table = NULL;
    rte_error_t result = ht_create(&hashmap_config, &table);
    RTE_LOGI("create a new hash table result %d", result);

    result = ht_set(table, "key1", 4, "value1", 6);
    RTE_LOGI( "Count is %d after setting an item %d in the table", result, ht_count(table));

    void* value = ht_get(table, "key1", 4, NULL);
    RTE_LOGI("Get value of key1 is %s", (char *)value);

    char *test_value = "blah";
    void *old_value = NULL;
    ht_get_and_set(table, "key1", 4, test_value, strlen(test_value), &old_value, NULL);
    char *new_value = ht_get(table, "key1", 4, NULL);
    if (strcmp(old_value, "value1") != 0) {
        RTE_LOGF("Old value not returned from ht_get_and_set() (got: %s , expected: value1)", old_value);
    } else if (strcmp(new_value, test_value) != 0) {
        RTE_LOGF("New value not stored properly (got: %s , expected: %s)", new_value, test_value);
    } else {
        RTE_LOGI("ht_get_and_set() overwrite the previous value and returns it");
    }

    ht_clear(table);

    result = ht_set(table, "test_key", 8, "test_value", 10);
    RTE_LOGI( "Count is %d after setting an item %d in the table", result, ht_count(table));

    result = ht_set_if_not_exists(table, "test_key", 8, "blah", 4);
    RTE_LOGI("ht_set_if_not_exists result %d", result);

    ht_get_or_set(table, "test_key", 8, "blah", 4, &old_value, NULL);
    new_value = ht_get(table, "test_key", 8, NULL);
    RTE_LOGI("ht_get_or_set() doesn't overwrite the current value and returns it new_value %s", new_value);

    old_value = NULL;
    result = ht_unset(table, "test_key", 8, &old_value, NULL);
    RTE_ASSERT(ht_get(table, "test_key", 8, NULL) == NULL);
    RTE_LOGI("unset %s result %d count %d", old_value, result, ht_count(table));

    ht_set(table, "to_remove", 9, "value", 5);
    int count = ht_count(table);
    ht_foreach_pair(table, remove_item, "to_remove");
    RTE_ASSERT(count == ht_count(table) + 1);
    RTE_LOGI("the item was not removed from the table");

    result = ht_delete(table, "test_key", 8, NULL, NULL);
    RTE_LOGI("delete result %d count %d", result, ht_count(table));

    ht_destroy(table);


    hashmap_config.free_cb = free_item;
    result = ht_create(&hashmap_config, &table);
    RTE_LOGI("create a new hash table result %d", result);

    int num_parallel_threads = 5;
    int num_parallel_items = 100000;

    RTE_LOGI("Parallel insert (%d items, %d threads)", num_parallel_items, num_parallel_threads);

    parallel_insert_arg args[num_parallel_threads];
    SDL_Thread *threads[num_parallel_threads];
    for (i = 0; i < num_parallel_threads; i++) {
        args[i].start = 0 + (i * (num_parallel_items / num_parallel_threads));
        args[i].end = args[i].start + (num_parallel_items / num_parallel_threads) -1;
        args[i].table = table;
        threads[i] = SDL_CreateThread(parallel_insert, NULL, &args[i]);
    }

    for (i = 0; i < num_parallel_threads; i++) {
        SDL_WaitThread(threads[i], NULL);
    }

    RTE_ASSERT(ht_count(table) == num_parallel_items);
    RTE_LOGI("Count is %d after parallel insert (%d)", num_parallel_items, ht_count(table));

    int check_item_count = 0;
    ht_foreach_pair(table, check_item, &check_item_count);
    RTE_ASSERT(check_item_count == num_parallel_items);
    RTE_LOGI("all items were valid (%d were valid, should have been %d)",
                check_item_count,
                num_parallel_items);

    ht_clear(table);
    RTE_ASSERT(free_count == num_parallel_items);
    RTE_LOGI("free_count is not equal to %d after clearing the table",
            num_parallel_items);

    ht_destroy(table);
    SDL_DestroyMutex(hashmap_mutex[0].mutex);
    SDL_DestroyMutex(hashmap_mutex[1].mutex);
}
