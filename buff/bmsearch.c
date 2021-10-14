/* Boyer-Moore search functions
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

#include <ctype.h>
#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/* \cond skip */
#define NUMASCII	256			/* number of ascii chars */

#define buffint() ((uint8_t)(*buff->curcptr))

static inline int bequal(struct buff *buff, char ch, int sensitive)
{
	return *buff->curcptr == ch ||
		(!sensitive && tolower(*buff->curcptr) == tolower(ch));
}
/* \endcond */

/** This is an implementation of the Boyer-Moore Search.
 * It uses the delta1 only with the fast/slow loops.
 * It searches for the string 'str' starting at the current buffer location.
 * If sensitive is 0, then the match is case insensitive.
 * The point is left at the byte after the search str.
 * @param buff The buffer to search in.
 * @param str The string to search for.
 * @param sensitive Should the search be case sensitive?
 * @return 1 if string found, 0 if not.
 */
int bm_search(struct buff *buff, const char *str, int sensitive)
{
	int delta[NUMASCII], len, i, shift;

	len = strlen(str) - 1;

	/* Init the delta table to str length.  For each char in the
	 * str, store the offset from the str start in the delta
	 * table.  If we are in case insensitive mode - lower case the
	 * match string and mark both the upper case version and the
	 * lower case version of the match string chars in the delta
	 * array.
	 */
	for (i = 0; i < NUMASCII; ++i)
		delta[i] = len ? len : 1;
	if (sensitive)
		for (i = 0; i <= len;  ++i)
			delta[(uint8_t)str[i]] = len - i;
	else
		for (i = 0; i <= len;  ++i) {
			delta[toupper(str[i])] = len - i;
			delta[tolower(str[i])] = len - i;
		}

	/* search forward*/
	while (!bisend(buff)) {
		/* fast loop - delta will be 0 if matched */
		while (!bisend(buff) && delta[buffint()])
			bmove(buff, delta[buffint()]);
		/* slow loop */
		for (i = len;
			 bequal(buff, str[i], sensitive);
			 bmove(buff, -1), --i)
			if (i == 0) {
				bmove(buff, len + 1);
				return 1;
			}
		/* compute shift. shift must be forward! */
		if (i + delta[buffint()] > len)
			shift = delta[buffint()];
		else
			shift = len - i + 1;
		bmove(buff, shift);
	}

	return 0;
}

/** This is an implementation of the Boyer-Moore Search that searches backwards.
 * It uses the delta1 only with the fast/slow loops.
 * It searches for the string 'str' starting at the current buffer location.
 * If sensitive is 0, then the match is case insensitive.
 * The point is left at the start of the search str.
 * @param buff The buffer to search in.
 * @param str The string to search backwards for.
 * @param sensitive Should the search be case sensitive?
 * @return 1 if string found, 0 if not.
 */
int bm_rsearch(struct buff *buff, const char *str, int sensitive)
{
	int delta[NUMASCII], len, i, shift;

	len = strlen(str) - 1;

	/* Init the delta table to str length.  For each char in the
	 * str, store the negative offset from the str start in the
	 * delta table.
	 */
	for (i = 0; i < NUMASCII; ++i)
		delta[i] = len ? -len : -1;
	if (sensitive)
		for (i = len; i >= 0; --i)
			delta[(uint8_t)str[i]] = -i;
	else
		for (i = len; i >= 0; --i) {
			delta[toupper(str[i])] = -i;
			delta[tolower(str[i])] = -i;
		}

	/* reverse search */
	bmove(buff, -len);
	while (!bisstart(buff)) {
		/* fast loop - delta will be 0 if matched */
		while (delta[buffint()] && !bisstart(buff))
			bmove(buff, delta[buffint()]);
		/* slow loop */
		for (i = 0;
			 i <= len && bequal(buff, str[i], sensitive);
			 ++i, bmove1(buff))
			;
		if (i > len) {
			/* we matched! */
			bmove(buff, -len - 1);
			return 1;
		}
		/* compute shift. shift must be backward! */
		shift = delta[buffint()] + i < 0 ? delta[buffint()] : -i - 1;
		bmove(buff, shift);
	}

	return 0;
}
/* @} */
