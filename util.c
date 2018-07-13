#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "buff.h"

static inline int __copy(char *dst, const char *src, int dstsize)
{
	int i;

	if (dstsize <= 0) return 0;

	--dstsize;
	for (i = 0; i < dstsize && *src; ++i)
		*dst++ = *src++;
	*dst = 0;

	return i;
}

int safecpy(char *dst, const char *src, int dstsize)
{
	return __copy(dst, src, dstsize);
}

int safecat(char *dst, const char *src, int dstsize)
{
	if (dstsize <= 0) return 0;

	int len = strlen(dst);
	dst += len;
	dstsize -= len;

	return __copy(dst, src, dstsize);
}

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
