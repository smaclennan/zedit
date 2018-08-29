/* strlcpy.c - strlcpy/strlcat for systems without
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
