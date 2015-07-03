#ifndef _OS_H
#define _OS_H

#include <assert.h>
#include <string.h>
#include "vptypes.h"

typedef enum {
	LOG_INFO = 0,
	LOG_WARN,
	LOG_ERROR,
    LOG_LAST = LOG_ERROR
} LOG_LEVEL;

void osLog(LOG_LEVEL level, const char * pFormatStr, ...);

#define STRINGIFY(x)  #x
#define DO_ASSERT assert

#ifdef _DEBUG
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
        osLog(LOG_ERROR, "assert failure (%s): File (%s), Line %u.\n", STRINGIFY(expr), __FILE__, __LINE__);\
        if (!(bIgnoreAssert##__FILE__##__LINE__))                                                           \
            OS_ASSERT_FUNC((#condition), (level), __FILE__, __LINE__, &bIgnoreAssert##__FILE__##__LINE__);  \
	    }                                                                                                   \
	}

#define osAssert(condition) osAssertLevel(condition, 0)			///< osAssert defaults to assert level 0

#define osWARNING(expr) \
    if (expr) { \
        osLog(LOG_ERROR, "Warning (%s) in %s at line %u.\n", \
                         STRINGIFY(expr), __FILE__, __LINE__); \
            }

#else

#define osAssert(condition)
#define osWARNING(expr)

#endif


inline void* osMalloc(size_t size) {
    if (size == 0) return NULL;
    void* p = malloc(size);
    if (p == NULL && size > 0) {
        osAssert(!"malloc failed");
    }
    return p;
}

inline void* osCalloc(size_t size) {
    if (size == 0) return NULL;
    return memset(osMalloc(size), 0, size);
}

/*
inline char* osStralloc(const char* s1) {
    char* s2 = (char*)osMalloc(strlen(s1) + 1);
    strcpy(s2, s1);
    return s2;
}
*/

inline void* osRealloc(void* p, uint32_t newSize) {
    // workaround library bug
    if (p == NULL && newSize == 0) {
        return NULL;
    }

    void* temp = realloc(p, newSize);
    if (temp == NULL && newSize > 0) {
        osAssert(!"malloc failed");
    }
    return temp;
}

inline void osFree(void * p)
{
	free(p);
}

#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))


void osDump(unsigned int indent, const char *lpszFormat, ...);
int64_t osGetLocalTimeSeconds();
EXTERN uint64 CONV osQueryTimer();
EXTERN uint64 CONV osQueryTimerFrequency();
EXTERN uint64 CONV osQueryNanosecondTimer();

#endif
