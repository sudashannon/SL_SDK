#include <stdio.h>
#include "rte_include.h"
#include "SDL.h"

/**
 * @brief 8bit unsigned integers comparator
 */
static int rbt_cmp_keys_uint8(void *k1, size_t k1size, void *k2, size_t k2size)
{
    RBT_CMP_KEYS_TYPE(int8_t, k1, k1size, k2, k2size);
}

static int
print_value(rbt_t *rbt, void *key, size_t ksize, void *value, void *priv)
{
    RTE_LOGI("rb_tree check value: %d", *((int *)value));
    return 1;
}

static int
sum_value(rbt_t *rbt, void *key, size_t ksize, void *value, void *priv)
{
    int *vsum = (int *)priv;
    *vsum += *((int *)value);
    return 1;
}

static int
check_sort(rbt_t *rbt, void *key, size_t ksize, void *value, void *priv)
{
    int *check = (int *)priv;
    int v = *((int *)value);

    if (v != *check)
        return 0;

    (*check)++;
    return 1;
}

static int
get_root(rbt_t *rbt, void *key, size_t ksize, void *value, void *priv)
{
    void **p = (void **)priv;
    *p = value;
    return 0;
}

static void element_free(void *element)
{
    rte_free(element);
}

void test_rbtree(void)
{
    int *v;
    uint8_t i;

    RTE_LOGI("rbt_create(free)");
    rbt_t *rbt = rbt_create(rbt_cmp_keys_uint8, element_free);
    RTE_ASSERT(rbt);

    RTE_LOGI("Adding 0..18");
    int sum = 0;
    for (i = 0; i < 18; i++) {
        v = rte_malloc(sizeof(int));
        *v = i;
        rbt_add(rbt, &i, sizeof(uint8_t), v);
        sum += i;
    }
    rbt_walk(rbt, print_value, NULL);
    int vsum = 0;
    int rc = rbt_walk(rbt, sum_value, &vsum);
    RTE_ASSERT(vsum == sum);

    RTE_LOGI("rbt_walk() return value");
    RTE_ASSERT(rc == 18);

    RTE_LOGI("root is '7'");
    rbt_walk(rbt, get_root, &v);
    RTE_ASSERT(*((int *)v) == 7);

    RTE_LOGI("rbt_walk_sorted()");
    int check = 0;
    rc = rbt_walk_sorted(rbt, check_sort, &check);
    RTE_ASSERT(check == 18);

    RTE_LOGI("rbt_walk_sorted() return value");
    RTE_ASSERT(rc == 18);
    rbt_walk(rbt, print_value, NULL);
    RTE_LOGI("Removing '7'");
    i = 7;
    rbt_remove(rbt, &i, sizeof(uint8_t), NULL);
    vsum = 0;
    rbt_walk(rbt, sum_value, &vsum);
    RTE_ASSERT(vsum == (sum - 7));

    RTE_LOGI("root is '6'");
    rbt_walk(rbt, get_root, &v);
    RTE_ASSERT(*((int *)v) == 6);

    rbt_walk(rbt, print_value, NULL);
    rbt_destroy(rbt);
    RTE_LOGI("rbt_destroy(free)");
}
