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
