#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "buff.h"

#define NUMASCII	256			/* number of ascii chars */

#define buff() (*buff->curcptr)

/* This is an implementation of the Boyer-Moore Search.
 * It uses the delta1 only with the fast/slow loops.
 * It searches for the string 'str' starting at the current buffer location.
 * The search will be case insensitive if the buffers current mode is so set.
 */
bool bm_search(struct buff *buff, const char *str, bool sensitive)
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
			delta[(int)str[i]] = len - i;
	else
		for (i = 0; i <= len;  ++i) {
			delta[toupper(str[i])] = len - i;
			delta[tolower(str[i])] = len - i;
		}

	/* search forward*/
	while (!bisend(buff)) {
		/* fast loop - delta will be 0 if matched */
		while (!bisend(buff) && delta[buff()])
			bmove(buff, delta[buff()]);
		/* slow loop */
		for (i = len;
			 (char)buff() == str[i] ||
				 (!sensitive && tolower(buff()) == tolower(str[i]));
			 bmove(buff, -1), --i)
			if (i == 0) {
				bmove(buff, len + 1);
				return true;
			}
		/* compute shift. shift must be forward! */
		if (i + delta[buff()] > len)
			shift = delta[buff()];
		else
			shift = len - i + 1;
		bmove(buff, shift);
	}

	return false;
}

bool bm_rsearch(struct buff *buff, const char *str, bool sensitive)
{
	int delta[NUMASCII], len, i;

	len = strlen(str) - 1;

	/* Init the delta table to str length.  For each char in the
	 * str, store the negative offset from the str start in the
	 * delta table.
	 */
	for (i = 0; i < NUMASCII; ++i)
		delta[i] = len ? -len : -1;
	if (sensitive)
		for (i = len; i >= 0; --i)
			delta[(int)str[i]] = -i;
	else
		for (i = len; i >= 0; --i) {
			delta[toupper(str[i])] = -i;
			delta[tolower(str[i])] = -i;
		}

	/* reverse search */
	bmove(buff, -len);
	while (!bisstart(buff)) {
		/* fast loop - delta will be 0 if matched */
		while (delta[buff()] && !bisstart(buff))
			bmove(buff, delta[buff()]);
		/* slow loop */
		for (i = 0;
			 i <= len &&
				 ((char)buff() == str[i] ||
				  (!sensitive &&
				   tolower(buff()) == tolower(str[i])));
			 ++i, bmove1(buff))
			;
		if (i > len) {
			/* we matched! */
			bmove(buff, -len - 1);
			return true;
		}
		/* compute shift. shift must be backward! */
		bmove(buff, delta[buff()] + i < 0 ? delta[buff()] : -i - 1);
	}

	return false;
}
