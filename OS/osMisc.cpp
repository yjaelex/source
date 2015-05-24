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
