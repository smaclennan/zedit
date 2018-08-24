/* bmsearch.c - Boyer-Moore search functions
 * Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef WIN32
#define NO_MEMRCHR
#else
#define _GNU_SOURCE /* for memrchr */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "buff.h"

#define buff() (*buff->curcptr)

/** Search for `what' in current buffer page starting at point. */
static inline Byte *memchrpage(struct buff *buff, Byte what)
{
	return memchr(buff->curcptr, what, curplen(buff) - buff->curchar);
}

/* Not Boyer-Moore.. but I think it makes sense to put it here */

/** Search forward for a single byte `what'. If what found leaves
 * point at the byte after what and returns true. If not found leaves
 * point at the end of buffer and returns false.
 */
bool bcsearch(struct buff *buff, Byte what)
{
	Byte *n;

	if (bisend(buff))
		return false;

	while ((n = memchrpage(buff, what)) == NULL) {
		if (lastp(buff->curpage)) {
			makeoffset(buff, buff->curpage->plen);
			return false;
		}
		makecur(buff, buff->curpage->nextp, 0);
	}

	makeoffset(buff, n - buff->curpage->pdata);
	bmove1(buff);
	return true;
}

#ifdef NO_MEMRCHR
/** Simple memrchr() for simple systems. */
static void *memrchr(const void *s, int c, size_t n)
{
	if (n) {
		const unsigned char *p = (const unsigned char *)s + n;
		const unsigned char cmp = c;

		do
			if (*--p == cmp)
				return (void *)p;
		while (--n != 0);
	}
	return NULL;
}
#endif

/** Search backward for a single byte. If byte found leaves point at
 * byte and returns true. If not found leaves point at the start of
 * buffer and returns false.
 */
bool bcrsearch(struct buff *buff, Byte what)
{
	Byte *n;

	if (bisstart(buff))
		return false;

	while (!(n = memrchr(buff->curpage->pdata, what, buff->curchar))) {
		if (buff->curpage == buff->firstp) {
			makeoffset(buff, 0);
			return false;
		}
		makecur(buff, buff->curpage->prevp, buff->curpage->prevp->plen);
	}

	makeoffset(buff, n - buff->curpage->pdata);
	return true;
}
