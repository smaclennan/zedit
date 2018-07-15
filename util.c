/* util.c - Utility functions
 * Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "buff.h"

#ifdef __linux__
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
#endif

static char *dbgfname;

const char *Dbgfname(const char *fname)
{
	FREE(dbgfname);
	dbgfname = NULL;
	if (fname) {
		dbgfname = malloc(strlen(fname) + 1);
		if (dbgfname)
			strcpy(dbgfname, fname);
	}
	return dbgfname;
}

void Dbg(const char *fmt, ...)
{
	va_list arg_ptr;

	va_start(arg_ptr, fmt);
	if (dbgfname) {
		FILE *fp = fopen(dbgfname, "a");

		if (fp) {
			vfprintf(fp, fmt, arg_ptr);
			fclose(fp);
		}
	} else
		vfprintf(stderr, fmt, arg_ptr);
	va_end(arg_ptr);
}
