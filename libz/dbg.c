/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "libz.h"

static char *dbgfname;


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
	len = vsnprintf(line, sizeof(line), fmt, ap);
	va_end(ap);

	if (dbgfname) {
		int fd = open(dbgfname, O_CREAT | O_WRONLY | O_APPEND, 0644);
		if (fd >= 0) {
			write(fd, line, len);
			close(fd);
		}
	} else
		fputs(line, stderr);
}
