#include "RTE_Vector.h"
/**
  ******************************************************************************
  * @file    RTE_Vector.c
  * @author  Shan Lei ->>lurenjia.tech ->>https://github.com/sudashannon
  * @brief   RTE自带的动态数组实现。
  * @version V1.0 2019/1/06
  * @History V1.0 创建，修改自lvgl
  ******************************************************************************
  */
#if RTE_USE_VEC == 1
#include <string.h>
void Vector_Init(vector_t **v,mem_bank_e bank)
{
    RTE_AssertParam(bank<BANK_N);
    RTE_AssertParam(bank!=BANK_NULL);
    vector_t *vector = Memory_Alloc0(bank,sizeof(vector_t));
    vector->bank = bank;
    vector->length = 0;
    vector->data = Memory_Realloc(vector->bank, vector->data, 0);
    vector->data   = NULL;
    *v = vector;
}
void Vector_Clean(vector_t *vector)
{
    for (int i=0; i<vector->length; i++) {
        Memory_Free(vector->bank, vector->data[i]);
        vector->data[i] = NULL;
    }
    Memory_Free(vector->bank, vector->data);
    vector->length = 0;
    vector->data = NULL;
}
void Vector_DeInit(vector_t **v)
{
    vector_t *vector = *v;
    Vector_Clean(vector);
    Memory_Free(vector->bank, vector);
    *v = NULL;
}
int Vector_Length(vector_t *vector)
{
    return vector->length;
}
void *Vector_GetElement(vector_t *vector, int idx)
{
    return vector->data[idx];
}
void Vector_Pushback(vector_t *vector, void *element)
{
    vector->data = Memory_Realloc(vector->bank, vector->data, (vector->length+1)*sizeof(void*));
    vector->data[vector->length] = element;
    vector->length++;
}
void *Vector_Popback(vector_t *vector)
{
    void *el=NULL;
    if (vector->length) {
        vector->length--;
        el = vector->data[vector->length];
        vector->data[vector->length] = NULL;
        vector->data = Memory_Realloc(vector->bank, vector->data, (vector->length)*sizeof(void*));
    }
    return el;
}
void *Vector_Take(vector_t *vector, int idx)
{
    void *el = vector->data[idx];
    if ((1 < vector->length) && (idx < (vector->length - 1))) {
        /* Since dst is always < src we can just use memcpy */
        memcpy(vector->data+idx, vector->data+idx+1, (vector->length-idx-1) * sizeof(void*));
    }
    vector->length--;
    vector->data[vector->length] = NULL;
    vector->data = Memory_Realloc(vector->bank, vector->data, (vector->length)*sizeof(void*));
    return el;
}
void Vector_Erase(vector_t *vector, int idx)
{
    void *el = Vector_Take(vector, idx);
    Memory_Free(vector->bank, el);
}
void Vector_Resize(vector_t *vector, int num)
{
    if (vector->length != num) {
        if (!num) {
            Vector_Clean(vector);
        } else {
            if (vector->length > num) {
                for (int i=num; i<vector->length; i++) {
                    Memory_Free(vector->bank, vector->data[i]);
                    vector->data[i] = NULL;
                }
            }
            // resize array
            vector->length = num;
            vector->data = Memory_Realloc(vector->bank, vector->data, vector->length * sizeof(void*));
        }
    }
}
static void quicksort(void **head, void **tail, vector_comp_t comp)
{
    while (head < tail) {
        void **h = head - 1;
        void **t = tail;
        void *v = tail[0];
        for (;;) {
            do ++h; while(h < t && comp(h[0], v) < 0);
            do --t; while(h < t && comp(v, t[0]) < 0);
            if (h >= t) break;
            void *x = h[0];
            h[0] = t[0];
            t[0] = x;
        }
        void *x = h[0];
        h[0] = tail[0];
        tail[0] = x;
        // do the smaller recursive call first, to keep stack within O(log(N))
        if (t - head < tail - h - 1) {
            quicksort(head, t, comp);
            head = h + 1;
        } else {
            quicksort(h + 1, tail, comp);
            tail = t;
        }
    }
}
// TODO Python defines sort to be stable but ours is not
void Vector_Sort(vector_t *vector, vector_comp_t comp)
{
    if (vector->length > 1) {
        quicksort(vector->data, vector->data + vector->length - 1, comp);
    }
}
#endif // RTE_USE_VEC
