#ifndef __RTE_VECTOR_H
#define __RTE_VECTOR_H
#include "RTE_Config.h"
#if RTE_USE_VEC == 1
#include "RTE_Memory.h"
typedef int  (*vector_comp_t)(const void*, const void*);
typedef struct {
    int length;
    mem_bank_e bank;
    void **data;
}vector_t;
extern void Vector_Init(vector_t **v,mem_bank_e bank);
extern void Vector_DeInit(vector_t **v);
extern void Vector_Clean(vector_t *vector);
extern int Vector_Length(vector_t *vector);
extern void *Vector_GetElement(vector_t *vector, int idx);
extern void Vector_Pushback(vector_t *vector, void *element);
extern void *Vector_Popback(vector_t *vector);
extern void *Vector_Take(vector_t *vector, int idx);
extern void Vector_Erase(vector_t *vector, int idx);
extern void Vector_Resize(vector_t *vector, int num);
extern void Vector_Sort(vector_t *vector, vector_comp_t comp);
#endif // RTE_USE_VEC
#endif // __RTE_VECTOR_H
