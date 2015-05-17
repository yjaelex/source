#include <stdio.h>
#include <stdarg.h>
#include "os.h"

static bool g_bLogToFile = false;
static char* pLogLevelStr[] = {
    "<LOG_INFO>:",
    "<LOG_WARN>:",
    "<LOG_ERR >:"
};

void osLog(LOG_LEVEL level, const char * pFormatStr, ...)
{
    va_list arg;
    static char buf[512] = { 0 };

    va_start(arg, pFormatStr);
    vsprintf(buf, pFormatStr, arg);
    va_end(arg);

    osAssert(level <= LOG_LAST);

    if (!g_bLogToFile)
    {
        printf("%s %s", pLogLevelStr[level], buf);
    }
}
