#include "os.h"

#ifdef VP_USE_POSIX
#include <sys/time.h>
int64_t getLocalTimeMilliseconds()
{
    timeval buf;
    if (gettimeofday(&buf, 0))
        memset(&buf, 0, sizeof(buf));
    return milliseconds_t(buf.tv_sec) * 1000 + buf.tv_usec / 1000;
}
#else

#ifdef WIN32
#include <sys/timeb.h>
int64_t getLocalTimeMilliseconds()
{
    __timeb64 buf;
    _ftime64(&buf);
    return int64_t(buf.time) * 1000 + buf.millitm;
}
#else
#error "Not support platform!"
#endif

#endif

int64_t osGetLocalTimeSeconds()
{
    return getLocalTimeMilliseconds() / 1000;
}

#ifdef WIN32
#include <Windows.h>

uint64 CONV
osQueryTimer()
{
    BOOL ret;
    LARGE_INTEGER time;

    ret = QueryPerformanceCounter(&time);
    osAssert(ret == TRUE);

    return time.QuadPart;
}

uint64 timerFrequency = 0;

uint64 CONV
osQueryTimerFrequency()
{
    BOOL ret;
    LARGE_INTEGER frequency;

    if (timerFrequency == 0)
    {
        ret = QueryPerformanceFrequency(&frequency);
        osAssert(ret == TRUE);
        osAssert(frequency.QuadPart);
        timerFrequency = frequency.QuadPart;
    }
    return timerFrequency;
}

uint64 CONV
osQueryNanosecondTimer()
{
    // Convert the timer value to ticks per Nanosecond from ticks per second, then
    //    divide by the frequency.
    //    There are 1 billion (1.0 * 10 ^9) Nanoseconds in a second.
    return (osQueryTimer() * 1000000000ULL) / osQueryTimerFrequency();
}


#else

#ifdef OS_LINUX

#include <sys/time.h>
#include <time.h>

#define SECS_IN_NANOSEC         1000000000ULL
#define MICROSECS_IN_NANSEC     1000ULL


uint64 CONV
osQueryTimerFrequency()
{
    // clock_gettime returns nanoseconds regardless of which clock is queried.
    return SECS_IN_NANOSEC;
}

uint64 CONV
osQueryTimer()
{
    // See "NOTES:" section at top of file for information on why this is done
    struct timeval tv;
    gettimeofday(&tv, NULL);        // return micro_sec
    return (((uint64)tv.tv_usec * MICROSECS_IN_NANSEC) + ((uint64)tv.tv_sec * SECS_IN_NANOSEC));
}


uint64 CONV
osQueryNanosecondTimer()
{
    return osQueryTimer();
}

#endif

#endif
