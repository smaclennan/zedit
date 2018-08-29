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

#include <fcntl.h>
#include <stdarg.h>
#include "buff.h"

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
	va_list ap;
	char line[1024];
	int len;

	va_start(ap, fmt);
	len = strfmt_ap(line, sizeof(line), fmt, ap);
	va_end(ap);

	if (dbgfname) {
		int fd = open(dbgfname, O_CREAT | O_WRONLY | O_APPEND, 0644);

		if (fd >= 0) {
			write(fd, line, len);
			close(fd);
		}
	} else
		write(2, line, len);
}
