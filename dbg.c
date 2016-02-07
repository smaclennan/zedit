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
	if (dbgfname)
		free(dbgfname);
	return dbgfname = strdup(fname);
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
