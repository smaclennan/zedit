/* itoa.c - integer to ascii
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

/** Helper function for itoa() and utoa(). */
static void reverse_string(char *start, char *end)
{
	int len = (end - start) / 2;
	--end;
	for (int i = 0; i < len; ++i) {
		char temp = *end;
		*end = *start;
		*start = temp;
		start++;
		end--;
	}
}

char *itoa(int val, char *out)
{
	char *p = out;
	int neg = val < 0;

	if (val == 0) {
		strcpy(out, "0");
		return out;
	}

	if (neg)
		val = -val;

	while (val > 0) {
		*p++ = (val % 10) + '0';
		val /= 10;
	}
	if (neg) *p++ = '-';
	*p = 0;

	reverse_string(out, p);
	return out;
}

/* Returns the end of the output */
char *_utoa(unsigned val, char *out)
{
	char *p = out;

	if (val == 0) {
		strcpy(out, "0");
		return out + 1;
	}

	while (val > 0) {
		*p++ = (val % 10) + '0';
		val /= 10;
	}
	*p = 0;

	reverse_string(out, p);
	return p;
}

char *utoa(unsigned val, char *out)
{
	_utoa(val, out);
	return out;
}
