#include <stdio.h>
#include <stdarg.h>
#include "os.h"

static bool g_bLogToFile = false;
static char* pLogLevelStr[] = {
    "<LOG_INFO>:",
    "<LOG_WARN>:",
    "<LOG_ERR >:"
};

void
osDump(unsigned int indent, const char *lpszFormat, ...)
{
	char szMsg[2048];
	va_list marker;
	static FILE* fp = NULL;

	va_start(marker, lpszFormat);
	vsprintf(szMsg, lpszFormat, marker);
	va_end(marker);

	if (fp == NULL)
	{
		char szFileName[64];

		sprintf(szFileName, "VideoLibraryDump.txt");
		fp = fopen(szFileName, "wb");
		if (fp == NULL)
		{
			return;
		}
	}

	fprintf(fp, "%*c", indent, ' ');
	fprintf(fp, szMsg);
	fflush(fp);
}


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
