#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <sys/time.h>
#include "rte_include.h"
#include "SDL.h"
typedef struct {
    int start;
    int end;
    linked_list_t *list;
} parallel_insert_arg;

static void *parallel_insert(void *user) {
    parallel_insert_arg *arg = (parallel_insert_arg *)user;
    int i;
    for (i = arg->start; i <= arg->end; i++) {
        char *v = malloc(100);
        sprintf(v, "test%d", i+1);
        list_set_value(arg->list, i, v);
    }
    return NULL;
}

static int free_count = 0;
static void free_value(void *val) {
    free(val);
    free_count++;
}

int iterator_callback(void *item, size_t idx, void *user) {
    int *failed = (int *)user;
    char *val = (char *)item;
    char test[100];
    sprintf(test, "test%d", idx+1);
    if (strcmp(test, val) != 0) {
        RTE_LOGF("Value at index %d doesn't match %s  (%s)", idx, test, val);
        *failed = 1;
        return 0;
    }
    return 1;
}

int slice_iterator_callback(void *item, size_t idx, void *user) {
    int *count = (int *)user;
    (*count)++;
    return 1;
}

typedef struct {
    linked_list_t *list;
    int count;
} queue_worker_arg;

void *queue_worker(void *user) {
    void *v;
    queue_worker_arg *arg = (queue_worker_arg *)user;
    for (;;) {
        v = list_shift_value(arg->list);
        if (v) {
            ATOMIC_INCREMENT(&arg->count);
            free(v);
        }
        pthread_testcancel();
    }
    return NULL;
}

static int
cmp(void *v1, void *v2)
{
    int *i1 = (int *)v1;
    int *i2 = (int *)v2;
    if (*i1 == *i2)
        return 0;
    else if (*i1 > *i2)
        return -1;
    return 1;
}

void test_linklist(void)
{
    int i;
    rte_mutex_t ll_mutex[4] = {
        {
            .mutex = SDL_CreateMutex(),
            .lock = rte_mutex_lock,
            .unlock = rte_mutex_unlock,
            .trylock = NULL
        },
        {
            .mutex = SDL_CreateMutex(),
            .lock = rte_mutex_lock,
            .unlock = rte_mutex_unlock,
            .trylock = NULL
        },
        {
            .mutex = SDL_CreateMutex(),
            .lock = rte_mutex_lock,
            .unlock = rte_mutex_unlock,
            .trylock = NULL
        },
        {
            .mutex = SDL_CreateMutex(),
            .lock = rte_mutex_lock,
            .unlock = rte_mutex_unlock,
            .trylock = NULL
        }
    };
    RTE_LOGI("list_create()");
    linked_list_t *list = list_create(&ll_mutex[0]);
    RTE_ASSERT(list != NULL);

    RTE_LOGI("list_push_tail_value() return value on success");
    RTE_ASSERT(list_push_tail_value(list, strdup("test1")) == 0);

    list_push_tail_value(list, strdup("test2"));
    list_push_tail_value(list, strdup("test3"));

    RTE_LOGI("list_count() after push");
    RTE_ASSERT(list_count(list) == 3);

    RTE_LOGI("list_pick_value()");
    RTE_ASSERT(strcmp(list_pick_value(list, 1), "test2") == 0);

    RTE_LOGI("list_shift_value()");
    char *v = list_shift_value(list);
    RTE_ASSERT(strcmp(v, "test1") == 0);

    RTE_LOGI("list_count() after shift");
    RTE_ASSERT(list_count(list) == 2);

    RTE_LOGI("list_unshift_value()");
    list_unshift_value(list, v);
    RTE_ASSERT(strcmp(list_pick_value(list, 0), "test1") == 0 );

    RTE_LOGI("list_push_tail_value() accepts NULL");
    RTE_ASSERT(list_push_tail_value(list, NULL) == 0);

    RTE_LOGI("length updated after pushing NULL");
    RTE_ASSERT(list_count(list) == 4);

    RTE_LOGI("list_pop_tail_value()");
    v = list_pop_tail_value(list);
    RTE_ASSERT(list_count(list) == 3);
    RTE_LOGI("list_pop_tail_value() returned last element");
    RTE_ASSERT(v == NULL);

    RTE_LOGI("still list_pop_tail_value() return value");
    v = list_pop_tail_value(list);
    RTE_ASSERT(strcmp(v, "test3") == 0);

    list_push_tail_value(list, v);
    RTE_LOGI("list_count() consistent");
    RTE_ASSERT(list_count(list) == 3);

    RTE_LOGI("pushing 100 values to the list");
    for (i = 4; i <= 100; i++) {
        char *val = malloc(100);
        sprintf(val, "test%d", i);
        list_push_tail_value(list, val);
    }
    RTE_ASSERT(list_count(list) == 100);

    RTE_LOGI("Order is preserved");
    int failed = 0;
    for (i = 0; i < 100; i++) {
        char test[100];
        sprintf(test, "test%d", i+1);
        char *val = list_pick_value(list, i);
        if (strcmp(val, test) != 0) {
            RTE_LOGF("Value at index %d doesn't match %s  (%s)", i, test, val);
            failed = 1;
            break;
        }
    }

    RTE_LOGI("Value iterator");
    failed = 0;
    list_foreach_value(list, iterator_callback, &failed);
    if (failed)
        RTE_LOGF("Order is wrong");

    RTE_LOGI("list_set_value() overrides previous value (and returns it)");
    // Note: we need to copy the string because the value will be released from our free callback
    char *test_value = strdup("blah");
    char *old_value = list_set_value(list, 5, test_value);
    char *new_value = list_pick_value(list, 5);
    if (strcmp(old_value, "test6") != 0) {
        RTE_LOGF("Old value is wrong ('%s' should have been 'test6', old_value)", old_value);
    } else if (strcmp(new_value, test_value) != 0) {
        RTE_LOGF("New value is wrong ('%s' should have been '%s', old_value)", new_value, test_value);
    }
    free(old_value);

    RTE_LOGI("list_swap_values()");
    list_swap_values(list, 9, 19);
    char *v1 = list_pick_value(list, 9);
    char *v2 = list_pick_value(list, 19);
    if (strcmp(v1, "test20") != 0) {
        RTE_LOGF("Value at position 9 (%s) should have been equal to 'test20'", v1);
    } else if (strcmp(v2, "test10") != 0) {
        RTE_LOGF("Value at position 19 (%s) should have been equal to 'test10'", v2);
    }

    RTE_LOGI("list_move_value()");
    old_value = list_pick_value(list, 45);
    list_move_value(list, 45, 67);
    RTE_ASSERT(strcmp(list_pick_value(list, 67), old_value) == 0);

    list_set_free_value_callback(list, free_value);

    RTE_LOGI("list_clear()");
    list_clear(list);
    RTE_ASSERT(list_count(list) == 0);

    RTE_LOGI("free value callback");
    RTE_ASSERT(free_count == 100);

    int num_parallel_threads = 5;
    int num_parallel_items = 10000;

    RTE_LOGI("Parallel insert (%d items)", num_parallel_items);

    parallel_insert_arg args[num_parallel_threads];
    pthread_t threads[num_parallel_threads];
    for (i = 0; i < num_parallel_threads; i++) {
        args[i].start = 0 + (i * (num_parallel_items / num_parallel_threads));
        args[i].end = args[i].start + (num_parallel_items / num_parallel_threads) -1;
        args[i].list = list;
        pthread_create(&threads[i], NULL, parallel_insert, &args[i]);
    }

    for (i = 0; i < num_parallel_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    RTE_ASSERT(list_count(list) == num_parallel_items);

    RTE_LOGI("Order after parallel insertion");
    failed = 0;
    list_foreach_value(list, iterator_callback, &failed);
    if (failed)
        RTE_LOGF("Order is wrong");

    free_count = 0;
    RTE_LOGI("list_destroy()");
    list_destroy(list);
    RTE_ASSERT(free_count == num_parallel_items);

    queue_worker_arg arg = {
        .list = list_create(&ll_mutex[1]),
        .count = 0
    };

    int num_queued_items = 10000;
    RTE_LOGI("Threaded queue (%d pull-workers, %d items pushed to the queue from the main thread)",
                num_parallel_threads, num_queued_items);

    for (i = 0; i < num_parallel_threads; i++) {
        pthread_create(&threads[i], NULL, queue_worker, &arg);
    }

    for (i = 0; i < num_queued_items; i++) {
        char *val = malloc(21);
        sprintf(val, "%d", i);
        list_push_tail_value(arg.list, val);
    }

    while(list_count(arg.list))
        usleep(500);

    for (i = 0; i < num_parallel_threads; i++) {
        pthread_cancel(threads[i]);
        pthread_join(threads[i], NULL);
    }

    RTE_ASSERT(arg.count == num_queued_items);

    list_destroy(arg.list);

    linked_list_t *tagged_list = list_create(&ll_mutex[2]);
    for (i = 0; i < 100; i++) {
        char key[21];
        char val[21];
        sprintf(key, "key%d", i);
        sprintf(val, "value%d", i);
        tagged_value_t *tv1 = list_create_tagged_value(key, val, strlen(val));
        list_push_tagged_value(tagged_list, tv1);
    }
    RTE_LOGI("get_tagged_value()");
    tagged_value_t *test_tagged_value = list_get_tagged_value(tagged_list, "key10");
    RTE_ASSERT(strcmp(test_tagged_value->value, "value10") == 0 );
    list_destroy_tagged_value(test_tagged_value);

    RTE_LOGI("set_tagged_value()");
    test_tagged_value = list_set_tagged_value(tagged_list, "key10", "test", 4, 0);
    if (strcmp(test_tagged_value->value, "value10") != 0)
        RTE_LOGF("Old value doesn't match");
    else {
        test_tagged_value = list_get_tagged_value(tagged_list, "key10");
        RTE_ASSERT(strcmp(test_tagged_value->value, "test") == 0);
        list_destroy_tagged_value(test_tagged_value);
    }

    list_destroy(tagged_list);

    RTE_LOGI("list_sort()");
    linked_list_t *t = list_create(&ll_mutex[3]);

    int max_num = 1000;
    int a[max_num];
    struct timeval tv = { 0, 0 };
    gettimeofday(&tv, NULL);
    int seed = tv.tv_sec + tv.tv_usec;
    srand(seed);

    int j;
    for (j = 0; j < max_num; ++j) {
        a[j] = rand() % max_num;
        list_push_tail_value(t, a + j);
    }

    list_sort(t, cmp);
    failed = 0;
    int prev, len = list_count(t);
    for (i = 0; i < len; i++) {
        int cur = *((int *)list_pick_value(t, i));
        if (i > 0 && cur < prev) {
            RTE_LOGF("%d is smaller than the previous element %d (index: %d)", cur, prev, i);
            failed++;
        }
        prev = cur;
    }

    slice_t *slice = slice_create(t, max_num / 2, max_num / 2);

    int count = 0;

    RTE_LOGI("slice_foreach_value");
    slice_foreach_value(slice, slice_iterator_callback, &count);
    RTE_ASSERT(count == max_num / 2);

    list_destroy(t);
    SDL_DestroyMutex(ll_mutex[0].mutex);
    SDL_DestroyMutex(ll_mutex[1].mutex);
    SDL_DestroyMutex(ll_mutex[2].mutex);
    SDL_DestroyMutex(ll_mutex[3].mutex);
}
