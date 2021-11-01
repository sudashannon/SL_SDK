/**
 * @file osal_include.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-10-12
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __OSAL_INCLUDE_H
#define __OSAL_INCLUDE_H

#ifndef __NO_RETURN
#if   defined(__CC_ARM)
#define __NO_RETURN __declspec(noreturn)
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#define __NO_RETURN __attribute__((__noreturn__))
#elif defined(__GNUC__)
#define __NO_RETURN __attribute__((__noreturn__))
#elif defined(__ICCARM__)
#define __NO_RETURN __noreturn
#else
#define __NO_RETURN
#endif
#endif

#if defined (__alpha__) || defined (__ia64__) || defined (__x86_64__) \
	|| defined (_WIN64) || defined (__LP64__) || defined (__LLP64__) \
    || (__riscv_xlen == 64)
#define OS_RESERVED_VALUE 0x7FFFFFFFFFFFFFFFUL
#define OS_TICK_TYPE			uint64_t
#define OS_MAX_DELAY 			(OS_TICK_TYPE) 0xFFFFFFFFFFFFFFFFUL
#else
#define OS_TICK_TYPE			uint32_t
#define OS_MAX_DELAY 			(OS_TICK_TYPE) 0xFFFFFFFFUL
#define OS_RESERVED_VALUE 0x7FFFFFFFUL
#endif

typedef OS_TICK_TYPE os_tick_t;

#include <stdint.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C"
{
#endif


//  ==== Enumerations, structures, defines ====

/// Version information.
typedef struct {
  uint32_t                       api;   ///< API version (major.minor.rev: mmnnnrrrr dec).
  uint32_t                    kernel;   ///< Kernel version (major.minor.rev: mmnnnrrrr dec).
} os_version_t;

/// Kernel state.
typedef enum {
  OS_KERNEL_INACTIVE        =  0,         ///< Inactive.
  OS_KERNEL_READY           =  1,         ///< Ready.
  OS_KERNEL_RUNNING         =  2,         ///< Running.
  OS_KERNEL_LOCKED          =  3,         ///< Locked.
  OS_KERNEL_SUSPENDED       =  4,         ///< Suspended.
  OS_KERNEL_ERROR           = -1,         ///< Error.
  OS_KERNEL_RESERVED        = OS_RESERVED_VALUE ///< Prevents enum down-size compiler optimization.
} os_kernel_state_t;

/// Thread state.
typedef enum {
  OS_THREAD_INACTIVE        =  0,         ///< Inactive.
  OS_THREAD_READY           =  1,         ///< Ready.
  OS_THREAD_RUNNING         =  2,         ///< Running.
  OS_THREAD_BLOCKED         =  3,         ///< Blocked.
  OS_THREAD_TERMINATED      =  4,         ///< Terminated.
  OS_THREAD_ERROR           = -1,         ///< Error.
  OS_THREAD_RESERVED        = OS_RESERVED_VALUE ///< Prevents enum down-size compiler optimization.
} os_thread_state_t;

/// Priority values.
typedef enum {
  OS_PRIORITY_NONE          =  0,         ///< No priority (not initialized).
  OS_PRIORITY_IDLE          =  1,         ///< Reserved for Idle thread.
  OS_PRIORITY_LOW           =  8,         ///< Priority: low
  OS_PRIORITY_LOW1          =  8+1,       ///< Priority: low + 1
  OS_PRIORITY_LOW2          =  8+2,       ///< Priority: low + 2
  OS_PRIORITY_LOW3          =  8+3,       ///< Priority: low + 3
  OS_PRIORITY_LOW4          =  8+4,       ///< Priority: low + 4
  OS_PRIORITY_LOW5          =  8+5,       ///< Priority: low + 5
  OS_PRIORITY_LOW6          =  8+6,       ///< Priority: low + 6
  OS_PRIORITY_LOW7          =  8+7,       ///< Priority: low + 7
  OS_PRIORITY_BELOW_NORMAL   = 16,         ///< Priority: below normal
  OS_PRIORITY_BELOW_NORMAL1  = 16+1,       ///< Priority: below normal + 1
  OS_PRIORITY_BELOW_NORMAL2  = 16+2,       ///< Priority: below normal + 2
  OS_PRIORITY_BELOW_NORMAL3  = 16+3,       ///< Priority: below normal + 3
  OS_PRIORITY_BELOW_NORMAL4  = 16+4,       ///< Priority: below normal + 4
  OS_PRIORITY_BELOW_NORMAL5  = 16+5,       ///< Priority: below normal + 5
  OS_PRIORITY_BELOW_NORMAL6  = 16+6,       ///< Priority: below normal + 6
  OS_PRIORITY_BELOW_NORMAL7  = 16+7,       ///< Priority: below normal + 7
  OS_PRIORITY_NORMAL        = 24,         ///< Priority: normal
  OS_PRIORITY_NORMAL1       = 24+1,       ///< Priority: normal + 1
  OS_PRIORITY_NORMAL2       = 24+2,       ///< Priority: normal + 2
  OS_PRIORITY_NORMAL3       = 24+3,       ///< Priority: normal + 3
  OS_PRIORITY_NORMAL4       = 24+4,       ///< Priority: normal + 4
  OS_PRIORITY_NORMAL5       = 24+5,       ///< Priority: normal + 5
  OS_PRIORITY_NORMAL6       = 24+6,       ///< Priority: normal + 6
  OS_PRIORITY_NORMAL7       = 24+7,       ///< Priority: normal + 7
  OS_PRIORITY_ABOVE_NORMAL   = 32,         ///< Priority: above normal
  OS_PRIORITY_ABOVE_NORMAL1  = 32+1,       ///< Priority: above normal + 1
  OS_PRIORITY_ABOVE_NORMAL2  = 32+2,       ///< Priority: above normal + 2
  OS_PRIORITY_ABOVE_NORMAL3  = 32+3,       ///< Priority: above normal + 3
  OS_PRIORITY_ABOVE_NORMAL4  = 32+4,       ///< Priority: above normal + 4
  OS_PRIORITY_ABOVE_NORMAL5  = 32+5,       ///< Priority: above normal + 5
  OS_PRIORITY_ABOVE_NORMAL6  = 32+6,       ///< Priority: above normal + 6
  OS_PRIORITY_ABOVE_NORMAL7  = 32+7,       ///< Priority: above normal + 7
  OS_PRIORITY_HIGH          = 40,         ///< Priority: high
  OS_PRIORITY_HIGH1         = 40+1,       ///< Priority: high + 1
  OS_PRIORITY_HIGH2         = 40+2,       ///< Priority: high + 2
  OS_PRIORITY_HIGH3         = 40+3,       ///< Priority: high + 3
  OS_PRIORITY_HIGH4         = 40+4,       ///< Priority: high + 4
  OS_PRIORITY_HIGH5         = 40+5,       ///< Priority: high + 5
  OS_PRIORITY_HIGH6         = 40+6,       ///< Priority: high + 6
  OS_PRIORITY_HIGH7         = 40+7,       ///< Priority: high + 7
  OS_PRIORITY_REALTIME      = 48,         ///< Priority: realtime
  OS_PRIORITY_REALTIME1     = 48+1,       ///< Priority: realtime + 1
  OS_PRIORITY_REALTIME2     = 48+2,       ///< Priority: realtime + 2
  OS_PRIORITY_REALTIME3     = 48+3,       ///< Priority: realtime + 3
  OS_PRIORITY_REALTIME4     = 48+4,       ///< Priority: realtime + 4
  OS_PRIORITY_REALTIME5     = 48+5,       ///< Priority: realtime + 5
  OS_PRIORITY_REALTIME6     = 48+6,       ///< Priority: realtime + 6
  OS_PRIORITY_REALTIME7     = 48+7,       ///< Priority: realtime + 7
  OS_PRIORITY_ISR           = 56,         ///< Reserved for ISR deferred thread.
  OS_PRIORITY_ERROR         = -1,         ///< System cannot determine priority or illegal priority.
  OS_PRIORITY_RESERVED      = OS_RESERVED_VALUE ///< Prevents enum down-size compiler optimization.
} os_priority_t;

/// Entry point of a thread.
typedef void (*os_thread_func_t) (void *argument);

/// Timer callback function.
typedef void (*os_timer_func_t) (void *argument);

/// Timer type.
typedef enum {
  OS_TIMER_ONCE               = 0,          ///< One-shot timer.
  OS_TIMER_PERIODIC           = 1           ///< Repeating timer.
} os_timer_type_t;

// Timeout value.
#define OS_WAIT_FOREVER         OS_MAX_DELAY ///< Wait forever timeout value.

// Flags options (\ref osThreadFlagsWait and \ref osEventFlagsWait).
#define OS_FLAGS_WAIT_ANY        0x00000000U ///< Wait for any flag (default).
#define OS_FLAGS_WAIT_ALL        0x00000001U ///< Wait for all flags.
#define OS_FLAGS_NO_CLEAR        0x00000002U ///< Do not clear flags which have been specified to wait for.

// Flags errors (returned by osThreadFlagsXxxx and osEventFlagsXxxx).
#define OS_FLAGS_ERROR           0x80000000U ///< Error indicator.
#define OS_FLAGS_ERROR_UNKNOWN   0xFFFFFFFFU ///< osError (-1).
#define OS_FLAGS_ERROR_TIMEOUT   0xFFFFFFFEU ///< osErrorTimeout (-2).
#define OS_FLAGS_ERROR_RESOURCE  0xFFFFFFFDU ///< osErrorResource (-3).
#define OS_FLAGS_ERROR_PARAMETER 0xFFFFFFFCU ///< osErrorParameter (-4).
#define OS_FLAGS_ERROR_ISR       0xFFFFFFFAU ///< osErrorISR (-6).

// Thread attributes (attr_bits in \ref osThreadAttr_t).
#define OS_THREAD_DETACHED      0x00000000U ///< Thread created in detached mode (default)
#define OS_THREAD_JOINABLE      0x00000001U ///< Thread created in joinable mode

// Mutex attributes (attr_bits in \ref osMutexAttr_t).
#define OS_MUTEX_RECURSIVE      0x00000001U ///< Recursive mutex.
#define OS_MUTEX_PRIO_INHERIT   0x00000002U ///< Priority inherit protocol.
#define OS_MUTEX_ROBUST         0x00000008U ///< Robust mutex.

/// Status code values returned by CMSIS-RTOS functions.
typedef enum {
  OS_OK                       =  0,         ///< Operation completed successfully.
  OS_ERROR                    = -1,         ///< Unspecified RTOS error: run-time error but no other error message fits.
  OS_ERROR_TIMEOUT            = -2,         ///< Operation not completed within the timeout period.
  OS_ERROR_RESOURCE           = -3,         ///< Resource not available.
  OS_ERROR_PARAMETER          = -4,         ///< Parameter error.
  OS_ERROR_NO_MEMORY          = -5,         ///< System is out of memory: it was impossible to allocate or reserve memory for the operation.
  OS_ERROR_ISR                = -6,         ///< Not allowed in ISR context: the function cannot be called from interrupt service routines.
  OS_STATUS_RESERVED          = OS_RESERVED_VALUE ///< Prevents enum down-size compiler optimization.
} os_status_t;


/// \details Thread ID identifies the thread.
typedef void *os_thread_id_t;

/// \details Timer ID identifies the timer.
typedef void *os_timer_id_t;

/// \details Event Flags ID identifies the event flags.
typedef void *os_event_flags_id_t;

/// \details Mutex ID identifies the mutex.
typedef void *os_mutex_id_t;

/// \details Semaphore ID identifies the semaphore.
typedef void *os_semaphore_id_t;

/// \details Memory Pool ID identifies the memory pool.
typedef void *os_memory_pool_id_t;

/// \details Message Queue ID identifies the message queue.
typedef void *os_message_queue_id_t;


/// Attributes structure for thread.
typedef struct {
  const char                   *name;   ///< name of the thread
  uint32_t                 attr_bits;   ///< attribute bits
  void                      *cb_mem;    ///< memory for control block
  uint32_t                   cb_size;   ///< size of provided memory for control block
  void                   *stack_mem;    ///< memory for stack
  uint32_t                stack_size;   ///< size of stack
  os_priority_t             priority;   ///< initial thread priority (default: osPriorityNormal)
  uint32_t                  reserved;   ///< reserved (must be 0)
} os_thread_attr_t;

/// Attributes structure for timer.
typedef struct {
  const char                   *name;   ///< name of the timer
  uint32_t                 attr_bits;   ///< attribute bits
  void                      *cb_mem;    ///< memory for control block
  uint32_t                   cb_size;   ///< size of provided memory for control block
} os_timer_attr_t;

/// Attributes structure for event flags.
typedef struct {
  const char                   *name;   ///< name of the event flags
  uint32_t                 attr_bits;   ///< attribute bits
  void                      *cb_mem;    ///< memory for control block
  uint32_t                   cb_size;   ///< size of provided memory for control block
} os_event_flags_attr_t;

/// Attributes structure for mutex.
typedef struct {
  const char                   *name;   ///< name of the mutex
  uint32_t                 attr_bits;   ///< attribute bits
  void                      *cb_mem;    ///< memory for control block
  uint32_t                   cb_size;   ///< size of provided memory for control block
} os_mutex_attr_t;

/// Attributes structure for semaphore.
typedef struct {
  const char                   *name;   ///< name of the semaphore
  uint32_t                 attr_bits;   ///< attribute bits
  void                      *cb_mem;    ///< memory for control block
  uint32_t                   cb_size;   ///< size of provided memory for control block
} os_semaphore_attr_t;

/// Attributes structure for memory pool.
typedef struct {
  const char                   *name;   ///< name of the memory pool
  uint32_t                 attr_bits;   ///< attribute bits
  void                      *cb_mem;    ///< memory for control block
  uint32_t                   cb_size;   ///< size of provided memory for control block
  void                      *mp_mem;    ///< memory for data storage
  uint32_t                   mp_size;   ///< size of provided memory for data storage
} os_memory_pool_attr_t;

/// Attributes structure for message queue.
typedef struct {
  const char                   *name;   ///< name of the message queue
  uint32_t                 attr_bits;   ///< attribute bits
  void                      *cb_mem;    ///< memory for control block
  uint32_t                   cb_size;   ///< size of provided memory for control block
  void                      *mq_mem;    ///< memory for data storage
  uint32_t                   mq_size;   ///< size of provided memory for data storage
} os_message_queue_attr_t;


//  ==== Kernel Management Functions ====

/// Initialize the RTOS Kernel.
/// \return status code that indicates the execution status of the function.
os_status_t os_kernel_initialize (void);

///  Get RTOS Kernel Information.
/// \param[out]    version       pointer to buffer for retrieving version information.
/// \param[out]    id_buf        pointer to buffer for retrieving kernel identification string.
/// \param[in]     id_size       size of buffer for kernel identification string.
/// \return status code that indicates the execution status of the function.
os_status_t os_kernel_get_info (os_version_t *version, char *id_buf, uint32_t id_size);

/// Get the current RTOS Kernel state.
/// \return current RTOS Kernel state.
os_kernel_state_t os_kernel_get_state (void);

/// Start the RTOS Kernel scheduler.
/// \return status code that indicates the execution status of the function.
os_status_t os_kernel_start (void);

/// Lock the RTOS Kernel scheduler.
/// \return previous lock state (1 - locked, 0 - not locked, error code if negative).
int32_t os_kernel_lock (void);

/// Unlock the RTOS Kernel scheduler.
/// \return previous lock state (1 - locked, 0 - not locked, error code if negative).
int32_t os_kernel_unlock (void);

/// Restore the RTOS Kernel scheduler lock state.
/// \param[in]     lock          lock state obtained by \ref osKernelLock or \ref osKernelUnlock.
/// \return new lock state (1 - locked, 0 - not locked, error code if negative).
int32_t os_kernel_restore_lock (int32_t lock);

/// Suspend the RTOS Kernel scheduler.
/// \return time in ticks, for how long the system can sleep or power-down.
uint32_t os_kernel_suspend (void);

/// Resume the RTOS Kernel scheduler.
/// \param[in]     sleep_ticks   time in ticks for how long the system was in sleep or power-down mode.
void os_kernel_resume (uint32_t sleep_ticks);

/// Get the RTOS kernel tick count.
/// \return RTOS kernel current tick count.
os_tick_t os_kernel_get_tick_count (void);

/// Get the RTOS kernel tick frequency.
/// \return frequency of the kernel tick in hertz, i.e. kernel ticks per second.
uint32_t os_kernel_get_tick_freq (void);

/// Get the RTOS kernel system timer count.
/// \return RTOS kernel current system timer count as system based value.
os_tick_t os_kernel_get_sys_timer_count (void);

/// Get the RTOS kernel system timer frequency.
/// \return frequency of the system timer in hertz, i.e. timer ticks per second.
uint32_t os_kernel_get_sys_timer_freq (void);


//  ==== Thread Management Functions ====

/// Create a thread and add it to Active Threads.
/// \param[in]     func          thread function.
/// \param[in]     argument      pointer that is passed to the thread function as start argument.
/// \param[in]     attr          thread attributes; NULL: default values.
/// \return thread ID for reference by other functions or NULL in case of error.
os_thread_id_t os_thread_new (os_thread_func_t func, void *argument, const os_thread_attr_t *attr);

/// Get name of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return name as null-terminated string.
const char *os_thread_get_name (os_thread_id_t thread_id);

/// Return the thread ID of the current running thread.
/// \return thread ID for reference by other functions or NULL in case of error.
os_thread_id_t os_thread_get_id (void);

/// Get current thread state of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return current thread state of the specified thread.
os_thread_state_t os_thread_get_state (os_thread_id_t thread_id);

/// Get stack size of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return stack size in bytes.
uint32_t os_thread_get_stack_size (os_thread_id_t thread_id);

/// Get available stack space of a thread based on stack watermark recording during execution.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return remaining stack space in bytes.
uint32_t os_thread_get_stack_space (os_thread_id_t thread_id);

/// Change priority of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \param[in]     priority      new priority value for the thread function.
/// \return status code that indicates the execution status of the function.
os_status_t os_thread_set_priority (os_thread_id_t thread_id, os_priority_t priority);

/// Get current priority of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return current priority value of the specified thread.
os_priority_t os_thread_get_priority (os_thread_id_t thread_id);

/// Pass control to next thread that is in state \b READY.
/// \return status code that indicates the execution status of the function.
os_status_t os_thread_yield (void);

/// Suspend execution of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
os_status_t os_thread_suspend (os_thread_id_t thread_id);

/// Resume execution of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
os_status_t os_thread_resume (os_thread_id_t thread_id);

/// Detach a thread (thread storage can be reclaimed when thread terminates).
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
os_status_t os_thread_detach (os_thread_id_t thread_id);

/// Wait for specified thread to terminate.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
os_status_t os_thread_join (os_thread_id_t thread_id);

/// Terminate execution of current running thread.
__NO_RETURN void os_thread_exit (void);

/// Terminate execution of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \return status code that indicates the execution status of the function.
os_status_t os_thread_terminate (os_thread_id_t thread_id);

/// Get number of active threads.
/// \return number of active threads.
uint32_t os_thread_get_count (void);

/// Enumerate active threads.
/// \param[out]    thread_array  pointer to array for retrieving thread IDs.
/// \param[in]     array_items   maximum number of items in array for retrieving thread IDs.
/// \return number of enumerated threads.
uint32_t os_thread_enumerate (os_thread_id_t *thread_array, uint32_t array_items);


//  ==== Thread Flags Functions ====

/// Set the specified Thread Flags of a thread.
/// \param[in]     thread_id     thread ID obtained by \ref osThreadNew or \ref osThreadGetId.
/// \param[in]     flags         specifies the flags of the thread that shall be set.
/// \return thread flags after setting or error code if highest bit set.
uint32_t os_thread_flags_set (os_thread_id_t thread_id, uint32_t flags);

/// Clear the specified Thread Flags of current running thread.
/// \param[in]     flags         specifies the flags of the thread that shall be cleared.
/// \return thread flags before clearing or error code if highest bit set.
uint32_t os_thread_flags_clear (uint32_t flags);

/// Get the current Thread Flags of current running thread.
/// \return current thread flags.
uint32_t os_thread_flags_get (void);

/// Wait for one or more Thread Flags of the current running thread to become signaled.
/// \param[in]     flags         specifies the flags to wait for.
/// \param[in]     options       specifies flags options (osFlagsXxxx).
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return thread flags before clearing or error code if highest bit set.
uint32_t os_thread_flags_wait (uint32_t flags, uint32_t options, os_tick_t timeout);


//  ==== Generic Wait Functions ====

/// Wait for Timeout (Time Delay).
/// \param[in]     ticks         \ref CMSIS_RTOS_TimeOutValue "time ticks" value
/// \return status code that indicates the execution status of the function.
os_status_t os_delay (os_tick_t ticks);

/// Wait until specified time.
/// \param[in]     ticks         absolute time in ticks
/// \return status code that indicates the execution status of the function.
os_status_t os_delay_until (os_tick_t ticks);


//  ==== Timer Management Functions ====

/// Create and Initialize a timer.
/// \param[in]     func          function pointer to callback function.
/// \param[in]     type          \ref osTimerOnce for one-shot or \ref osTimerPeriodic for periodic behavior.
/// \param[in]     argument      argument to the timer callback function.
/// \param[in]     attr          timer attributes; NULL: default values.
/// \return timer ID for reference by other functions or NULL in case of error.
os_timer_id_t os_timer_new (os_timer_func_t func, os_timer_type_t type, void *argument, const os_timer_attr_t *attr);

/// Get name of a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerNew.
/// \return name as null-terminated string.
const char *os_timer_get_name (os_timer_id_t timer_id);

/// Start or restart a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerNew.
/// \param[in]     ticks         \ref CMSIS_RTOS_TimeOutValue "time ticks" value of the timer.
/// \return status code that indicates the execution status of the function.
os_status_t os_timer_start (os_timer_id_t timer_id, os_tick_t ticks);

/// Stop a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerNew.
/// \return status code that indicates the execution status of the function.
os_status_t os_timer_stop (os_timer_id_t timer_id);

/// Check if a timer is running.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerNew.
/// \return 0 not running, 1 running.
uint32_t os_timer_is_running (os_timer_id_t timer_id);

/// Delete a timer.
/// \param[in]     timer_id      timer ID obtained by \ref osTimerNew.
/// \return status code that indicates the execution status of the function.
os_status_t os_timer_delete (os_timer_id_t timer_id);


//  ==== Event Flags Management Functions ====

/// Create and Initialize an Event Flags object.
/// \param[in]     attr          event flags attributes; NULL: default values.
/// \return event flags ID for reference by other functions or NULL in case of error.
os_event_flags_id_t os_event_flags_new (const os_event_flags_attr_t *attr);

/// Get name of an Event Flags object.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \return name as null-terminated string.
const char *os_event_flags_get_name (os_event_flags_id_t ef_id);

/// Set the specified Event Flags.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \param[in]     flags         specifies the flags that shall be set.
/// \return event flags after setting or error code if highest bit set.
uint32_t os_event_flags_set (os_event_flags_id_t ef_id, uint32_t flags);

/// Clear the specified Event Flags.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \param[in]     flags         specifies the flags that shall be cleared.
/// \return event flags before clearing or error code if highest bit set.
uint32_t os_event_flags_clear (os_event_flags_id_t ef_id, uint32_t flags);

/// Get the current Event Flags.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \return current event flags.
uint32_t os_event_flags_get (os_event_flags_id_t ef_id);

/// Wait for one or more Event Flags to become signaled.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \param[in]     flags         specifies the flags to wait for.
/// \param[in]     options       specifies flags options (osFlagsXxxx).
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return event flags before clearing or error code if highest bit set.
uint32_t os_event_flags_wait (os_event_flags_id_t ef_id, uint32_t flags, uint32_t options, os_tick_t timeout);

/// Delete an Event Flags object.
/// \param[in]     ef_id         event flags ID obtained by \ref osEventFlagsNew.
/// \return status code that indicates the execution status of the function.
os_status_t os_event_flags_delete (os_event_flags_id_t ef_id);


//  ==== Mutex Management Functions ====

/// Create and Initialize a Mutex object.
/// \param[in]     attr          mutex attributes; NULL: default values.
/// \return mutex ID for reference by other functions or NULL in case of error.
os_mutex_id_t os_mutex_new (const os_mutex_attr_t *attr);

/// Get name of a Mutex object.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \return name as null-terminated string.
const char *os_mutex_get_name (os_mutex_id_t mutex_id);

/// Acquire a Mutex or timeout if it is locked.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
os_status_t os_mutex_acquire (os_mutex_id_t mutex_id, os_tick_t timeout);

/// Release a Mutex that was acquired by \ref osMutexAcquire.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \return status code that indicates the execution status of the function.
os_status_t os_mutex_release (os_mutex_id_t mutex_id);

/// Get Thread which owns a Mutex object.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \return thread ID of owner thread or NULL when mutex was not acquired.
os_thread_id_t os_mutex_get_owner (os_mutex_id_t mutex_id);

/// Delete a Mutex object.
/// \param[in]     mutex_id      mutex ID obtained by \ref osMutexNew.
/// \return status code that indicates the execution status of the function.
os_status_t os_mutex_delete (os_mutex_id_t mutex_id);


//  ==== Semaphore Management Functions ====

/// Create and Initialize a Semaphore object.
/// \param[in]     max_count     maximum number of available tokens.
/// \param[in]     initial_count initial number of available tokens.
/// \param[in]     attr          semaphore attributes; NULL: default values.
/// \return semaphore ID for reference by other functions or NULL in case of error.
os_semaphore_id_t os_semaphore_new (uint32_t max_count, uint32_t initial_count, const os_semaphore_attr_t *attr);

/// Get name of a Semaphore object.
/// \param[in]     semaphore_id  semaphore ID obtained by \ref osSemaphoreNew.
/// \return name as null-terminated string.
const char *os_semaphore_get_name (os_semaphore_id_t semaphore_id);

/// Acquire a Semaphore token or timeout if no tokens are available.
/// \param[in]     semaphore_id  semaphore ID obtained by \ref osSemaphoreNew.
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
os_status_t os_semaphore_acquire (os_semaphore_id_t semaphore_id, os_tick_t timeout);

/// Release a Semaphore token up to the initial maximum count.
/// \param[in]     semaphore_id  semaphore ID obtained by \ref osSemaphoreNew.
/// \return status code that indicates the execution status of the function.
os_status_t os_semaphore_release (os_semaphore_id_t semaphore_id);

/// Get current Semaphore token count.
/// \param[in]     semaphore_id  semaphore ID obtained by \ref osSemaphoreNew.
/// \return number of tokens available.
uint32_t os_semaphore_get_count (os_semaphore_id_t semaphore_id);

/// Delete a Semaphore object.
/// \param[in]     semaphore_id  semaphore ID obtained by \ref osSemaphoreNew.
/// \return status code that indicates the execution status of the function.
os_status_t os_semaphore_delete (os_semaphore_id_t semaphore_id);


//  ==== Message Queue Management Functions ====

/// Create and Initialize a Message Queue object.
/// \param[in]     msg_count     maximum number of messages in queue.
/// \param[in]     msg_size      maximum message size in bytes.
/// \param[in]     attr          message queue attributes; NULL: default values.
/// \return message queue ID for reference by other functions or NULL in case of error.
os_message_queue_id_t os_message_queue_new (uint32_t msg_count, uint32_t msg_size, const os_message_queue_attr_t *attr);

/// Get name of a Message Queue object.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return name as null-terminated string.
const char *os_message_queue_get_name (os_message_queue_id_t mq_id);

/// Put a Message into a Queue or timeout if Queue is full.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \param[in]     msg_ptr       pointer to buffer with message to put into a queue.
/// \param[in]     msg_prio      message priority.
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
os_status_t os_message_queue_put (os_message_queue_id_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout);

/// Get a Message from a Queue or timeout if Queue is empty.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \param[out]    msg_ptr       pointer to buffer for message to get from a queue.
/// \param[out]    msg_prio      pointer to buffer for message priority or NULL.
/// \param[in]     timeout       \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
/// \return status code that indicates the execution status of the function.
os_status_t os_message_queue_get (os_message_queue_id_t mq_id, void *msg_ptr, uint8_t *msg_prio, os_tick_t timeout);

/// Get maximum number of messages in a Message Queue.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return maximum number of messages.
uint32_t os_message_queue_get_capacity (os_message_queue_id_t mq_id);

/// Get maximum message size in a Memory Pool.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return maximum message size in bytes.
uint32_t os_message_queue_get_msg_size (os_message_queue_id_t mq_id);

/// Get number of queued messages in a Message Queue.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return number of queued messages.
uint32_t os_message_queue_get_count (os_message_queue_id_t mq_id);

/// Get number of available slots for messages in a Message Queue.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return number of available slots for messages.
uint32_t os_message_queue_get_space (os_message_queue_id_t mq_id);

/// Reset a Message Queue to initial empty state.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return status code that indicates the execution status of the function.
os_status_t os_message_queue_reset (os_message_queue_id_t mq_id);

/// Delete a Message Queue object.
/// \param[in]     mq_id         message queue ID obtained by \ref osMessageQueueNew.
/// \return status code that indicates the execution status of the function.
os_status_t os_message_queue_delete (os_message_queue_id_t mq_id);


#ifdef  __cplusplus
}
#endif

#endif