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

#include <stdarg.h>
#include <signal.h>


static char *dbgfname;
Byte Dbgstr[256];
int Dbgint;


void Dbg(char *fmt, ...)
{
	FILE *fp;
	va_list arg_ptr;

	fp = dbgfname ? fopen(dbgfname, "a") : NULL;
	va_start(arg_ptr, fmt);

	if (fp)
		vfprintf(fp, fmt, arg_ptr);
	else
		vprintf(fmt, arg_ptr);
	va_end(arg_ptr);
	if (fp)
		fclose(fp);
}


Boolean Dbgname(char *name)
{
	if (dbgfname)
		free(dbgfname);
	dbgfname = malloc(strlen(name) + 1);
	if (dbgfname) {
		strcpy(dbgfname, name);
		unlink(dbgfname);
	}
	return dbgfname != NULL;
}

void Dbgsig(int signum)
{
	Dbg("Dbgsig: got a %d\n", signum);
	signal(signum, Dbgsig);
}
