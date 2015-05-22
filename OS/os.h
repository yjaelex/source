#ifndef _OS_H
#define _OS_H

#include <assert.h>

typedef enum {
	LOG_INFO = 0,
	LOG_WARN,
	LOG_ERROR,
    LOG_LAST = LOG_ERROR
} LOG_LEVEL;

#define DO_ASSERT assert

inline void OS_ASSERT_FUNC(const char * condition, const unsigned int level,
	const char* file, unsigned int line, unsigned int *ignore)
{
    fprintf(stderr, "Assert: %s\nFile = [%s:%d]\n", (condition), (file), (line));
    DO_ASSERT(0);
}

#define osAssertLevel(condition, level) {                                                                   \
    if (!(condition))                                                                                       \
	    {                                                                                                   \
        static unsigned int bIgnoreAssert##__FILE__##__LINE__ = 0;                                          \
        if (!(bIgnoreAssert##__FILE__##__LINE__))                                                           \
            OS_ASSERT_FUNC((#condition), (level), __FILE__, __LINE__, &bIgnoreAssert##__FILE__##__LINE__);  \
	    }                                                                                                   \
	}

#define osAssert(condition) osAssertLevel(condition, 0)			///< osAssert defaults to assert level 0

void osDump(unsigned int indent, const char *lpszFormat, ...);
void osLog(LOG_LEVEL level, const char * pFormatStr, ...);

#endif
