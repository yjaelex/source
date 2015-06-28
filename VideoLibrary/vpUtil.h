#ifndef _VP_UTIL_H
#define _VP_UTIL_H

#include "vptypes.h"
#include "os.h"

#define VP_BYTESWAP_16(x) \
    uint16_t( ((uint16_t(x) & 0xff00U) >> 8) \
                             | ((uint16_t(x) & 0x00ffU) << 8) )

#define VP_BYTESWAP_32(x) \
    uint32_t( ((uint32_t(x) & 0xff000000U) >> 24) \
                             | ((uint32_t(x) & 0x00ff0000U) >>  8) \
                             | ((uint32_t(x) & 0x0000ff00U) <<  8) \
                             | ((uint32_t(x) & 0x000000ffU) << 24) )

#define VP_BYTESWAP_64(x) \
    uint64_t( ((uint64_t(x) & 0xff00000000000000ULL) >> 56) \
                             | ((uint64_t(x) & 0x00ff000000000000ULL) >> 40) \
                             | ((uint64_t(x) & 0x0000ff0000000000ULL) >> 24) \
                             | ((uint64_t(x) & 0x000000ff00000000ULL) >>  8) \
                             | ((uint64_t(x) & 0x00000000ff000000ULL) <<  8) \
                             | ((uint64_t(x) & 0x0000000000ff0000ULL) << 24) \
                             | ((uint64_t(x) & 0x000000000000ff00ULL) << 40) \
                             | ((uint64_t(x) & 0x00000000000000ffULL) << 56) )

#if defined( __BIG_ENDIAN__ )
#   define VP_HTONS(x)  (x)
#   define VP_HTONL(x)  (x)
#   define VP_NTOHS(x)  (x)
#   define VP_NTOHL(x)  (x)
#else
#   define VP_HTONS(x)  VP_BYTESWAP_16(x)
#   define VP_HTONL(x)  VP_BYTESWAP_32(x)
#   define VP_NTOHS(x)  VP_BYTESWAP_16(x)
#   define VP_NTOHL(x)  VP_BYTESWAP_32(x)
#endif

typedef uint32_t    MP4TrackId;
typedef uint32_t    MP4SampleId;
typedef uint64_t    MP4Timestamp;
typedef uint64_t    MP4Duration;
typedef uint32_t    MP4EditId;

uint32_t STRTOINT32(const char* s);
void INT32TOSTR(uint32_t i, char* s);
bool MP4NameFirstMatches(const char* s1, const char* s2);
bool MP4NameFirstIndex(const char* s, uint32_t* pIndex);
const char* MP4NameAfterFirst(const char *s);
MP4Timestamp MP4GetAbsTimestamp();

#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

#endif
