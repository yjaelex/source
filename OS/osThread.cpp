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


