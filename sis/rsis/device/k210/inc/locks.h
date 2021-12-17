#ifndef __LOCKS_H
#define __LOCKS_H

#include <stdint.h>
#include <stddef.h>

typedef long _lock_t;

extern void _lock_init(_lock_t *lock);

extern void _lock_init_recursive(_lock_t *lock);

extern void _lock_close(_lock_t *lock);

extern void _lock_close_recursive(_lock_t *lock);

extern void _lock_acquire(_lock_t *lock);

extern void _lock_acquire_recursive(_lock_t *lock);

extern int _lock_try_acquire(_lock_t *lock);

extern int _lock_try_acquire_recursive(_lock_t *lock);

extern void _lock_release(_lock_t *lock);

extern void _lock_release_recursive(_lock_t *lock);

#endif
