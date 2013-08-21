/* dbg.c - debug output
 * Copyright (C) 1988-2010 Sean MacLennan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "z.h"

#if DBG
#include <stdarg.h>
#include <sys/time.h>

static char *dbgfname;

void Dbg(char *fmt, ...)
{
	FILE *fp;
	va_list arg_ptr;

	if (dbgfname) {
		fp = fopen(dbgfname, "a");
		if (fp) {
			va_start(arg_ptr, fmt);
			vfprintf(fp, fmt, arg_ptr);
			va_end(arg_ptr);
			fclose(fp);
		}
	}
}

void Dbgname(char *name)
{
	if (dbgfname)
		free(dbgfname);

	if (name) {
		dbgfname = malloc(strlen(name) + 1);
		if (dbgfname) {
			strcpy(dbgfname, name);
			unlink(dbgfname);
		}
	}
}

static struct timeval stopwatch;

void dbg_startwatch(void)
{
	gettimeofday(&stopwatch, NULL);
}

void dbg_stopwatch(char *str)
{
	struct timeval end;

	gettimeofday(&end, NULL);

	if (stopwatch.tv_usec > end.tv_usec) {
		--end.tv_sec;
		end.tv_usec += 1000000;
	}

	if (!str)
		str = "stopwatch";

	Dbg("%s: %ld.%06ld\n", str,
	    end.tv_sec - stopwatch.tv_sec,
	    end.tv_usec - stopwatch.tv_usec);
}

#else
void Dbg(char *fmt, ...) {}
void Dbgname(char *name) {}
#endif
