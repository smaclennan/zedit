/* dbg.c - debugging functions
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

#include <fcntl.h>
#include <stdarg.h>
#include "buff.h"
#include "tinit.h"

/* \cond skip */
static char *dbgfname;
/* \endcond */

/** @addtogroup misc
 * @{
*/

/** Set the filename for the Dbg() function. Allocates the space for
 * the filename. If the filename is NULL, free the current filename if
 * any.
 * @param fname The filename to set. Can be NULL to free the current filename.
 * @return Returns the set filename or NULL.
 */
const char *Dbgfname(const char *fname)
{
	free(dbgfname);
	dbgfname = NULL;
	if (fname) {
		dbgfname = malloc(strlen(fname) + 1);
		if (dbgfname)
			strcpy(dbgfname, fname);
	}
	return dbgfname;
}

/** Debug output. The output either goes to a file if Dbgfname() was
 * called or to stderr. File output is appended.
 *
 * The strfmt() function is called to output the string, so the format
 * is limited. The output size is also limited to 1k.
 *
 * @param fmt The output format.
 * @param ... Zero or more arguments.
 */
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
			_twrite(fd, line, len);
			close(fd);
		}
	} else
		terror(line);
}
/* @} */
