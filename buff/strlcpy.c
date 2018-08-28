#include "buff.h"

#if defined(__linux__) || defined(WIN32)
/* A simple strlcpy implementation for Linux */
size_t strlcpy(char *dst, const char *src, size_t dstsize)
{
	size_t i = 0;

	if (dstsize > 0) {
		--dstsize;
		while (*src && i < dstsize) {
			*dst++ = *src++;
			++i;
		}
		*dst = 0;
	}

	/* strlcpy returns the size of the src */
	while (*src++) ++i;

	return i;
}

/* A simple strlcat implementation for Linux */
size_t strlcat(char *dst, const char *src, size_t dstsize)
{
	size_t i = 0;

	while (dstsize > 0 && *dst) {
		--dstsize;
		++dst;
		++i;
	}

	if (dstsize > 0) {
		--dstsize;
		while (*src && i < dstsize) {
			*dst++ = *src++;
			++i;
		}
		*dst = 0;
	}

	/* strlcat returns the size of the src + initial dst */
	while (*src++) ++i;

	return i;
}
#endif
