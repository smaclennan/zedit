/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

/* \cond skip */
#if defined(WIN32) || defined(__QNXNTO__)
#define NO_MEMRCHR
#elif !defined(_GNU_SOURCE)
#define _GNU_SOURCE /* for memrchr */
#endif
/* \endcond */

#include "buff.h"

/* \cond skip */
/** Search for `what' in current buffer page starting at point. */
static inline Byte *memchrpage(struct buff *buff, Byte what)
{
	return memchr(buff->curcptr, what, curplen(buff) - buff->curchar);
}
/* \endcond */

/** @addtogroup buffer
 * @{
 */

/** Search forward for a single byte. If found, leaves point at the
 * byte after the match and returns 1. If not found, leaves the point
 * at the end of buffer and returns 0.
 * @param buff The buffer to search in.
 * @param what The byte to search for.
 * @return 1 if byte found, 0 if not found.
 */
int bcsearch(struct buff *buff, Byte what)
{
	Byte *n;

	if (bisend(buff))
		return 0;

	while ((n = memchrpage(buff, what)) == NULL) {
		if (lastp(buff->curpage)) {
			makeoffset(buff, buff->curpage->plen);
			return 0;
		}
		makecur(buff, buff->curpage->nextp, 0);
	}

	makeoffset(buff, n - buff->curpage->pdata);
	bmove1(buff);
	return 1;
}

#ifdef NO_MEMRCHR
/** Simple memrchr() for simple systems. */
static void *memrchr(const void *s, int c, size_t n)
{
	if (n) {
		const unsigned char *p = (const unsigned char *)s + n;
		const unsigned char cmp = c;

		do {
			if (*--p == cmp)
				return (void *)p;
		} while (--n != 0);
	}
	return NULL;
}
#endif

/** Search backward for a single byte. If byte found leaves point at
 * byte and returns 1. If not found leaves point at the start of
 * buffer and returns 0.
 * @param buff The buffer to search in.
 * @param what The byte to search backwards for.
 * @return 1 if byte found, 0 if not found.
 */
int bcrsearch(struct buff *buff, Byte what)
{
	Byte *n;

	if (bisstart(buff))
		return 0;

	while (!(n = memrchr(buff->curpage->pdata, what, buff->curchar))) {
		if (buff->curpage == buff->firstp) {
			makeoffset(buff, 0);
			return 0;
		}
		makecur(buff, buff->curpage->prevp, buff->curpage->prevp->plen);
	}

	makeoffset(buff, n - buff->curpage->pdata);
	return 1;
}
/* @} */
