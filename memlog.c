/* memlog.c - log memory allocation
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

#ifdef MEMLOG
#include <stdio.h>
#include <string.h>

static FILE *logfp;


static void logmem(char ch, char *m, unsigned n, char *f, unsigned l)
{
	if (logfp)
		fprintf(logfp, "%c %8x %4u  %s:%u\n", ch, m, n, f, l);
}

void loginit(char *name)
{
	if (logfp)
		return;
	logfp = fopen(name, "w");
}

void logfini()
{
	if (logfp) {
		fputs("Fini.\n", logfp);
		fclose(logfp);
		logfp = NULL;
	}
}

char *logmalloc(unsigned n, unsigned l, char *f)
{
	char *m;

	m = malloc(n);
	logmem('A', m, n, f, l);
	return m;
}

char *logdup(char *s, char *f, unsigned l)
{
	char *m = strdup(s);
	logmem('S', m, strlen(s) + 1, f, l);
	return m;
}


void logfree(char *m, char *f, unsigned l)
{
	logmem('F', m, 0, f, l);
	free(m);
}
#endif
