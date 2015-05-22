#include "vpUtil.h"


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
