#include <stdio.h>
#include <stdarg.h>
#include "os.h"

static bool g_bLogToFile = false;
static char* pLogLevelStr[] = {
    "<LOG_INFO>:",
    "<LOG_WARN>:",
    "<LOG_ERR >:"
};
static osLogPrintCB pfnPrintCB = NULL;
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

void osLogSetPrintCB(osLogPrintCB pfunc)
{
    pfnPrintCB = pfunc;
}

#define LOG_HEADER_STRING_LEN       16
void osLog(LOG_LEVEL level, const char * pFormatStr, ...)
{
    va_list arg;
    static char buf[512] = { 0 };
    char * pStr = buf + LOG_HEADER_STRING_LEN;
    va_start(arg, pFormatStr);
    vsprintf(pStr, pFormatStr, arg);
    va_end(arg);

    osAssert(level <= LOG_LAST);

    if (!g_bLogToFile)
    {
        if (!pfnPrintCB)
        {
            printf("%s %s \n", pLogLevelStr[level], pStr);
        }
        else
        {
            char* pHdr = pLogLevelStr[level];
            uint32 len = strlen(pHdr);
            len = min(len, LOG_HEADER_STRING_LEN);
            uint32 logStrLen = strlen(pStr);
            pStr[logStrLen] = '\n';
            pStr[logStrLen + 1] = 0;
            pStr = pStr - len;
            char * p = pStr;
            while (len--)
            {
                *pStr++ = *pHdr++;
            }
            pfnPrintCB(p);
        }
    }
}
