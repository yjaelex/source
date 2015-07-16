#ifndef OS_THREAD_H
#define OS_THREAD_H

#if defined(WIN32)
#include <Windows.h>
#endif

#ifdef OS_LINUX
#include <pthread.h>
#endif // OS_LINUX

#include "vptypes.h"

typedef intp osThreadLocalKey;

#if defined(WIN32)
typedef uint32 osThreadID;
#elif defined(OS_LINUX)
typedef pthread_t osThreadID;
#else
#error Unsupported OS.
#endif

#if defined(WIN32)
typedef uintp osThreadHandle;
#elif defined(OS_LINUX)
typedef uintp osThreadHandle;
#else
#error Unsupported OS.
#endif

#if defined(WIN32)
typedef uintp osEventHandle;
typedef uintp osSemHandle;
#elif defined(OS_ANDROID)
typedef pvoid osEventHandle;
typedef pvoid osSemHandle;
#elif defined(OS_LINUX)
typedef uintp osEventHandle;
typedef uintp osSemHandle;
#else
#error Unsupported OS.
#endif

#if defined(WIN32)
typedef uintp osLibraryHandle;
#elif defined(OS_LINUX)
typedef uintp osLibraryHandle;
#else
#error Unsupported OS.
#endif

//
/// The return value of osThreadGetExitCode
//
typedef enum
{
    OS_THREAD_SUCCESS,          ///< thread exited successfully
    OS_THREAD_FAILED,           ///< thread exited with failure
    OS_THREAD_STILL_ACTIVE      ///< thread is still active
} osThreadExitCode;


#if defined(WIN32)
typedef osThreadExitCode(*osThreadProc)(uintp param);
#elif defined(OS_LINUX)
typedef osThreadExitCode(*osThreadProc)(uintp param);
#else
#error Unsupported OS.
#endif

#if defined(OS_LINUX)
#define OS_THREAD_ID()                  pthread_self()
#endif

EXTERN osThreadID CONV
osGetCurrentThreadID(void);

EXTERN osThreadLocalKey CONV
osThreadLocalAlloc(void);

EXTERN void CONV
osThreadLocalFree(osThreadLocalKey key);

EXTERN void CONV
osThreadLocalSet(osThreadLocalKey key, const void* value);

EXTERN void* CONV
osThreadLocalGet(osThreadLocalKey key);

void CONV
osThreadLocalRelease(uint32 index);

EXTERN osThreadHandle CONV
osThreadCreate(osThreadProc proc, uintp param, bool joinable = false);

///
/// @fn osThreadGetExitCode(osThreadHandle thread)
///
/// @brief Query the thread exit code
///
/// @param thread (in) A thread handle
///
/// @return The exit code or OS_THREAD_STILL_ACTIVE if the thread is sill alive
///
EXTERN osThreadExitCode CONV
osThreadGetExitCode(osThreadHandle thread);

///
/// @fn osThreadSuspend(uint32 msec)
///
/// @brief Suspend the execution of the current thread for at least the specified interval. A value of zero causes the
///        thread to relinquish the remainder of its time slice to any other thread of equal priority that is ready to run.
///        If there are no other threads of equal priority ready to run, the function returns immediately, and the thread
///        continues execution.
///
/// @param msec (in) Minimum time interval for which execution is to be suspended, in milliseconds
///
/// @return No return value.
///
EXTERN void CONV
osThreadSuspend(uint32 msec);

EXTERN void CONV osThreadDestroy(osThreadHandle handle);

///
/// @fn osThreadSetAffinityMask(uintp mask)
///
/// @brief Sets a processor affinity mask for the current thread.
///
/// @param mask (in) Affinity mask for the thread
///
/// @return true if the function succeeds, false otherwise.
///
EXTERN bool CONV
osThreadSetAffinityMask(uintp mask);

EXTERN bool CONV
osProcessSetAffinityMask(uintp  mask);

EXTERN bool CONV
osProcessGetAffinityMask(uintp * mask);

EXTERN int32 CONV
osThreadGetPriority(osThreadHandle thread = 0);

///
/// @fn osThreadSetPriority(osThreadHandle thread, int32 priority)
///
/// @brief Sets the scheduling priority of the given thread
///
/// @param thread (in) A thread handle (NULL for current thread)
/// @param priority (in) New scheduling priority
///
/// @return Scheduling priority
///
EXTERN void CONV
osThreadSetPriority(osThreadHandle thread, int32 priority);

///
/// @fn osEventCreate(bool manualReset)
///
/// @brief Creates an event object.
///
/// @param manualReset (in) parameter to create manual reset event, default is false
///
/// @param manualReset (in) a bool value, true to create manual reset event.
///
/// @return If the function succeeds, the return value is a handle to the event object. If the function fails, the return value is NULL
///
EXTERN osEventHandle CONV
osEventCreate(bool manualReset = false);

///
/// @fn osEventDuplicate(osEventHandle orig)
///
/// @brief duplicates an event object.
///
/// @param orig (in) the event handle that we want a duplicate for
///
/// @return If the function succeeds, the return value is a handle to the duplicate event object. If the function fails, the return value is NULL
///
EXTERN osEventHandle CONV
osEventDuplicate(osEventHandle orig);

///
/// @fn osEventDestroy(osEventHandle handle)
///
/// @brief Destroys an event object.
///
/// @param handle (in) A handle to the event object.
///
/// @return No return value.
///
EXTERN void CONV
osEventDestroy(osEventHandle handle);

///
/// @fn osEventSet(osEventHandle handle)
///
/// @brief The function sets the specified event object to the signaled state.
///
/// @param handle (in) A handle to the event object.
///
/// @return No return value.
///
EXTERN void CONV
osEventSet(osEventHandle handle);

///
/// @fn osEventWait(osEventHandle handle)
///
/// @brief The function returns when the specified object is in the signaled state.
///
/// @param handle (in) A handle to the event object.
///
/// @return No return value.
///
EXTERN void CONV
osEventWait(osEventHandle handle);

///
/// @fn osEventTimedWait(osEventHandle handle, uint32 timeout)
///
/// @brief The function returns when the specified object is in the signaled state or up to some maximum time-out.
///
/// @param handle (in) A handle to the event object.
/// @param timeout (in) The maximum time to wait in milliseconds
///
/// @return true if the lock succeeded, false if timed out.
///
EXTERN bool CONV
osEventTimedWait(osEventHandle handle, uint32 timeout);

///
/// @fn osEventReset(osEventHandle handle)
///
/// @brief The function resets the specified event object to the nonsignaled state.
///
/// @param handle (in) A handle to the event object.
///
/// @return No return value.
///
EXTERN void CONV
osEventReset(osEventHandle handle);

///
/// @fn osSemaphoreCreate(uint32 initialCount, uint32 maxCount, const char *semName)
///
/// @brief Creates an semaphore object.
/// @param initialCount (in) The initial value of the semaphore.
/// @param maxCount     (in) The maximum value of the semaphore.
/// @param semName      (in) Optional named semaphore.
///
/// @return If the function succeeds, the return value is a handle to the semaphore. If the function fails, the return value is NULL
///
EXTERN osSemHandle CONV
osSemaphoreCreate(uint32 initialCount, uint32 maxCount, const char *semName = 0);

///
/// @fn osSemaphoreRelease(osSemHandle handle, uint32 releaseCount, long *previousCount)
///
/// @brief The function returns when the specified object is in the signaled state or up to some maximum time-out.
///
/// @param handle         (in) A handle to the semaphore object.
/// @param releaseCount   (in) The amount by which the semaphore object's current count is to be increased. The value must be greater than zero.
///                            If the specified amount would cause the semaphore's count to exceed the maximum count that was specified when
///                            the semaphore was created, the count is not changed and the function returns FALSE.
/// @param previousCount (out) A pointer to a variable to receive the previous count for the semaphore.
///                            This parameter can be NULL if the previous count is not required.
///
/// @return true if the lock succeeded, false if the operation failed.
EXTERN bool CONV
osSemaphoreRelease(osSemHandle handle, uint32 releaseCount, long *previousCount);

void CONV osSemWait(osSemHandle handle);
bool CONV osSemTimedWait(osSemHandle handle, uint32 timeout);
bool CONV osSemDestroy(osSemHandle handle);

///
/// @fn osOpenLibrary(const char *libraryName);
///
/// @brief The function open the library with name libraryName.
///
/// @param libraryName (in) The name for the library that want to be loaded. absolute or relative path could be included. 
///
/// @return osLibraryHandle if open is successful otherwise NULL is returned.  
EXTERN osLibraryHandle CONV
osOpenLibrary(const char *libraryName);

///
/// @fn osCloseLibrary(osLibraryHandle handle);
///
/// @brief The function close the library with library handle.
///
/// @param handle (in) The handle for the library 
///
/// @return true if the close succeeded, false if the operation failed. 
EXTERN bool CONV
osCloseLibrary(osLibraryHandle handle);

///
/// @fn osGetProcAddress(osLibraryHandle handle, const char *name);
///
/// @brief The function open the library with name libraryName.
///
/// @param  handle (in) the handle to the specific library. 
/// @param  name (in) The name string for the wanted symbol
///
/// @return the address of the wanted symbol, or return NULL if cannot be found.  
EXTERN void * CONV
osGetProcAddress(osLibraryHandle handle, const char *name);


typedef int32(*tCallback)(uintp data);

typedef enum
{
    THREAD_CMD_NONE = 0,
    THREAD_CMD_COPY_DATA = 1 << 0,
}ThreadCommandType;


typedef struct OSThreadCommandRec
{
    ThreadCommandType   op;            ///< indicate the command type.
    uintp               data;          ///< the data need to pass to callback.
    tCallback           callback;      ///< the callback function that used to handle the comand.
    OSThreadCommandRec()
    {
        op = THREAD_CMD_NONE;
        data = 0;
        callback = NULL;
    }
}OSThreadCommand;

typedef void* osLockHandle;

osLockHandle CONV osLockCreate(const char *lockName);
void CONV osLockDestroy(osLockHandle lockHandle);
void osAcquireLock(osLockHandle lockHandle);
void osReleaseLock(osLockHandle lockHandle);
FINLINE bool OS_LOCK_CAS(volatile uint32 *dst, uint32 OldValue, uint32 NewValue);

#endif


