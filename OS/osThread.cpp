#include "os.h"

#if defined(WIN32)
#include "osThreadWindows.h"
#elif defined(OS_LINUX)
#include "osThreadLinux.h"
#else
#error Unsupported OS.
#endif

#ifdef ATI_OS_LINUX
//just for the root process, any new process create will trigue a new array allocate in stack
#include <sys/types.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#define MAX_THREAD_NUMBERS 1024
#define MAX_TLS_SLOTS     64

#endif

osThreadLocalKey CONV
osThreadLocalAlloc(void)
{
    osThreadLocalKey key;

    OS_THREAD_LOCAL_ALLOC(key);
    return key;
}

osThreadID CONV
osGetCurrentThreadID(void)
{
    COMPILE_TIME_ASSERT(sizeof(osThreadID) == sizeof(TID_t));
    TID_t tid = OS_THREAD_ID();
    return static_cast<osThreadID>(tid);
}

void CONV
osThreadLocalFree(osThreadLocalKey key)
{
    OS_THREAD_LOCAL_FREE(key);
}

#if defined(WIN32)
void CONV
osThreadLocalSet(osThreadLocalKey key, const void* value)
{
    OS_THREAD_LOCAL_SET(key, value);
}

void* CONV
osThreadLocalGet(osThreadLocalKey key)
{
   return OS_THREAD_LOCAL_GET(key);
}
#endif

osLockHandle CONV
osLockCreate(const char *lockName)
{
    static osLock zeroLock = { 0 }; // For quick zeroing of structures
    osLock *lock = (osLock*)osMalloc(sizeof(*lock));
    *lock = zeroLock;

    if (lockName)
    {
        OS_NAMED_LOCK_CREATE(lock, lockName);
        lock->namedLock = true;
    }
    else
    {
        OS_LOCK_CREATE(lock);
        lock->namedLock = false;
    }
    return (osLockHandle)lock;
}

//
//  Destroy lock resources
//
void CONV
osLockDestroy(osLockHandle lockHandle)
{
    osLock* lock = (osLock*)lockHandle;

    if (lock->namedLock)
    {
        OS_NAMED_LOCK_DESTROY(lock);
    }
    else
    {
        OS_LOCK_DESTROY(lock);
    }
    osFree(lockHandle);
}

void osAcquireLock(osLockHandle lockHandle)
{
    osLock* lock = (osLock*)lockHandle;
    OS_LOCK_ENTER(lock);
}

void osReleaseLock(osLockHandle lockHandle)
{
    osLock* lock = (osLock*)lockHandle;
    OS_LOCK_LEAVE(lock);
}
