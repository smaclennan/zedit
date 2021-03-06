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

/** @addtogroup buffer
 * @{
 */

#if defined(__linux__) || defined(WIN32)
/* A strlcpy implementation for systems without.
 * @param dst The destination buffer.
 * @param src The source buffer.
 * @param dstsize The destination size.
 * @return The size of the source.
 */
size_t strlcpy(char *dst, const char *src, size_t dstlen)
{
	int srclen = strlen(src);

	if (dstlen > 0) {
		if (dstlen > srclen)
			strcpy(dst, src);
		else {
			strncpy(dst, src, dstlen - 1);
			dst[dstlen - 1] = 0;
		}
	}

	return srclen;
}

/* A simple strlcat implementation for systems without.
 * @param dst The destination buffer.
 * @param src The source buffer.
 * @param dstsize The destination size.
 * @return The size of the source plus the size of the original destination.
 */
size_t strlcat(char *dst, const char *src, size_t dstsize)
{
	int dstlen = strlen(dst);
	int srclen = strlen(src);
	int left   = dstsize - dstlen;

	if (left > 0) {
		if (left > srclen)
			strcpy(dst + dstlen, src);
		else {
			strncpy(dst + dstlen, src, left - 1);
			dst[dstsize - 1] = 0;
		}
	}

	return dstlen + srclen;
}
#endif

/* Concatenates any number of string. The last string must be NULL.
 * @param str The string to copy to.
 * @param len The length of the string.
 * @param ... The strings to concat.
 * @return The length actually copied.
 */
int strconcat(char *str, int len, ...)
{
	char *arg;
	int total = 0;
	va_list ap;

	va_start(ap, len);
	while ((arg = va_arg(ap, char *)) && len > 0) {
		int n = strlcpy(str, arg, len);

		str += n;
		len -= n;
		total += n;
	}
	va_end(ap);

	return total;
}
/* @} */
