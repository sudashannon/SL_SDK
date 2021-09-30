/**
 * @file rte_atomic.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-07-27
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __RTE_ATOMIC_H
#define __RTE_ATOMIC_H

/* Atomic operation */
/* 内存访问栅 */
#define barrier()             	      (__sync_synchronize())
/* 原子获取 */
#define ATOMIC_READ(ptr)              ({ __typeof__(*(ptr)) volatile *_val = (ptr); barrier(); (*_val); })
/* 原子设置，如果原值和新值不一样则设置*/
#define ATOMIC_SET(ptr, value)        ((void)__sync_lock_test_and_set((ptr), (value)))
/* 原子交换，如果被设置，则返回旧值，否则返回设置值 */
#define ATOMIC_SWAP(ptr, value)       ((__typeof__(*(ptr)))__sync_lock_test_and_set((ptr), (value)))
/* 原子比较交换，如果当前值等于旧值，则新值被设置，返回旧值，否则返回新值*/
#define ATOMIC_CAS(ptr, comp, value)  ((__typeof__(*(ptr)))__sync_val_compare_and_swap((ptr), (comp), (value)))
/* 原子比较交换，如果当前值等于旧值，则新值被设置，返回真值，否则返回假 */
#define ATOMIC_CASB(ptr, comp, value) (__sync_bool_compare_and_swap((ptr), (comp), (value)) != 0 ? true : false)
/* 原子清零 */
#define ATOMIC_CLEAR(ptr)             ((void)__sync_lock_release((ptr)))
/* 通过值与旧值进行算术与位操作，返回新值 */
#define ATOMIC_ADD_F(ptr, value)      ((__typeof__(*(ptr)))__sync_add_and_fetch((ptr), (value)))
#define ATOMIC_SUB_F(ptr, value)      ((__typeof__(*(ptr)))__sync_sub_and_fetch((ptr), (value)))
#define ATOMIC_OR_F(ptr, value)       ((__typeof__(*(ptr)))__sync_or_and_fetch((ptr), (value)))
#define ATOMIC_AND_F(ptr, value)      ((__typeof__(*(ptr)))__sync_and_and_fetch((ptr), (value)))
#define ATOMIC_XOR_F(ptr, value)      ((__typeof__(*(ptr)))__sync_xor_and_fetch((ptr), (value)))
/* 通过值与旧值进行算术与位操作，返回旧值 */
#define ATOMIC_F_ADD(ptr, value)      ((__typeof__(*(ptr)))__sync_fetch_and_add((ptr), (value)))
#define ATOMIC_F_SUB(ptr, value)      ((__typeof__(*(ptr)))__sync_fetch_and_sub((ptr), (value)))
#define ATOMIC_F_OR(ptr, value)       ((__typeof__(*(ptr)))__sync_fetch_and_or((ptr), (value)))
#define ATOMIC_F_AND(ptr, value)      ((__typeof__(*(ptr)))__sync_fetch_and_and((ptr), (value)))
#define ATOMIC_F_XOR(ptr, value)      ((__typeof__(*(ptr)))__sync_fetch_and_xor((ptr), (value)))
/* 忽略返回值，算术和位操作 */
#define ATOMIC_INCREMENT(ptr)           ((void)ATOMIC_ADD_F((ptr), 1))
#define ATOMIC_DECREMENT(ptr)           ((void)ATOMIC_SUB_F((ptr), 1))
#define ATOMIC_ADD(ptr, val)            ((void)ATOMIC_ADD_F((ptr), (val)))
#define ATOMIC_SUB(ptr, val)            ((void)ATOMIC_SUB_F((ptr), (val)))
#define ATOMIC_OR(ptr, val)			    ((void)ATOMIC_OR_F((ptr), (val)))
#define ATOMIC_AND(ptr, val)			((void)ATOMIC_AND_F((ptr), (val)))
#define ATOMIC_XOR(ptr, val)			((void)ATOMIC_XOR_F((ptr), (val)))
/* 通过掩码，设置某个位为1，并返还新的值 */
#define ATOMIC_BIT_ON(ptr, mask)        ATOMIC_OR_F((ptr), (mask))
/* 通过掩码，设置某个位为0，并返还新的值 */
#define ATOMIC_BIT_OFF(ptr, mask)       ATOMIC_AND_F((ptr), ~(mask))
/* 通过掩码，交换某个位，1变0，0变1，并返还新的值 */
#define ATOMIC_BIT_XCHG(ptr, mask)      ATOMIC_XOR_F((ptr), (mask))

#endif
