#ifndef _OS_H
#define _OS_H

#include <assert.h>

typedef enum {
	LOG_INFO = 0,
	LOG_WARN,
	LOG_ERROR,
    LOG_LAST = LOG_ERROR
} LOG_LEVEL;

#define osAssert assert

void osLog(LOG_LEVEL level, const char * pFormatStr, ...);

#endif
