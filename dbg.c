#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "buff.h"

static char *dbgfname;

const char *Dbgfname(const char *fname)
{
	if (dbgfname)
		free(dbgfname);
	return dbgfname = strdup(fname);
}

void Dbg(const char *fmt, ...)
{
	if (!dbgfname)
		dbgfname = strdup("/tmp/z.out");

	FILE *fp = fopen(dbgfname, "a");
	if (fp) {
		va_list arg_ptr;

		va_start(arg_ptr, fmt);
		vfprintf(fp, fmt, arg_ptr);
		va_end(arg_ptr);
		fclose(fp);
	}
}
