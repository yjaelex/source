#include "vpUtil.h"


bool MP4NameFirstMatches(const char* s1, const char* s2)
{
    if (s1 == NULL || *s1 == '\0' || s2 == NULL || *s2 == '\0') {
        return false;
    }

    if (*s2 == '*') {
        return true;
}

    while (*s1 != '\0') {
        if (*s2 == '\0' || strchr("[.", *s2)) {
            break;
        }
        if (tolower(*s1) != tolower(*s2)) {
            return false;
        }
        s1++;
        s2++;
    }
    return true;
}

bool MP4NameFirstIndex(const char* s, uint32_t* pIndex)
{
    if (s == NULL) {
        return false;
    }

    while (*s != '\0' && *s != '.') {
        if (*s == '[') {
            s++;
            osAssert(pIndex);
            if (sscanf(s, "%u", pIndex) != 1) {
                return false;
            }
            return true;
        }
        s++;
    }
    return false;
}

const char* MP4NameAfterFirst(const char *s)
{
    if (s == NULL)
    {
        return NULL;
    }

    while (*s != '\0')
    {
        if (*s == '.')
        {
            s++;
            if (*s == '\0')
            {
                return NULL;
            }
            return s;
        }
        s++;
    }
    return NULL;
}


uint32_t STRTOINT32(const char* s)
{
#if defined( VP_INTSTRING_ALIGNMENT )
	// it seems ARM integer instructions require 4-byte alignment so we
	// manually copy string-data into the integer before performing ops
	uint32_t tmp;
	memcpy(&tmp, s, sizeof(tmp));
	return VP_NTOHL(tmp);
#else
	return VP_NTOHL(*(uint32_t *)s);
#endif
}

void INT32TOSTR(uint32_t i, char* s)
{
#if defined( VP_INTSTRING_ALIGNMENT )
	// it seems ARM integer instructions require 4-byte alignment so we
	// manually copy string-data into the integer before performing ops
	uint32_t tmp = VP_NTOHL(i);
	memcpy(s, &tmp, sizeof(tmp));
#else
	*(uint32_t *)s = VP_HTONL(i);
#endif
	s[4] = 0;
}
