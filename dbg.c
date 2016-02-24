/* dbg.c - debug output functions
 * Copyright (C) 1988-2016 Sean MacLennan <seanm@seanm.ca>
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
#include <string.h>
#include <stdarg.h>

#include "buff.h"

#ifdef WIN32
#define strdup _strdup
#endif

static char *dbgfname;

const char *Dbgfname(const char *fname)
{
	if (dbgfname) {
		free(dbgfname);
		dbgfname = NULL;
	}
	if (fname)
		dbgfname = strdup(fname);
	return dbgfname;
}

void Dbg(const char *fmt, ...)
{
	FILE *fp;

	if (!dbgfname)
		dbgfname = strdup("/tmp/z.out");

	if ((fp = fopen(dbgfname, "a"))) {
		va_list arg_ptr;

		va_start(arg_ptr, fmt);
		vfprintf(fp, fmt, arg_ptr);
		va_end(arg_ptr);
		fclose(fp);
	}
}
