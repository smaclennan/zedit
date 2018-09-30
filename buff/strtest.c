/* strtest.c - test strfmt program (not part of libbuff)
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

#include <stdio.h> // for snprintf
#include <assert.h>
#include "buff.h"

int main(int argc, char *argv[])
{
	char str1[100], str2[100], str[100];
	int n1, n2, i;

	i = -1;
	n1 = snprintf(str1, sizeof(str1), "test %d and %u\n", i, i);
	n2 = strfmt(str2, sizeof(str2), "test %d and %u\n", i, i);
	assert(n1 == n2);
	assert(strcmp(str1, str2) == 0);

	i = 42;
	strcpy(str, "hello");
	n1 = snprintf(str1, sizeof(str1), "%s world %d\n", str, i);
	n2 = strfmt(str2, sizeof(str2), "%s world %d\n", str, i);
	assert(n1 == n2);
	assert(strcmp(str1, str2) == 0);

	n1 = snprintf(str1, sizeof(str1), "The quick brown fox\n");
	n2 = strfmt(str2, sizeof(str2), "The quick brown fox\n");
	assert(n1 == n2);
	assert(strcmp(str1, str2) == 0);

#ifdef WANT_FLOATS
	// We must be a bit careful with the float number since snprintf is more accurate.
	n1 = snprintf(str1, sizeof(str1), "%f\n", 727.141586);
	n2 = strfmt(str2, sizeof(str2), "%f\n", 727.141586);
	assert(n1 == n2);
	assert(strcmp(str1, str2) == 0);
#endif

	return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -O2 -Wall strtest.c -o strtest libbuff.a"
 * End:
 */
