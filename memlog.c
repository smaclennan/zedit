#ifdef MEMLOG
#include <stdio.h>
#include <string.h>

static FILE *logfp = NULL;

static void logmem();


void loginit(name)
char *name;
{
	if(logfp) return;
	logfp = fopen(name, "w");
}


void logfini()
{
	if(logfp)
	{
		fputs("Fini.\n", logfp);
		fclose(logfp);
		logfp = NULL;
	}
}


char *logmalloc(n, f, l)
unsigned n, l;
char *f;
{
	extern char *malloc();
	char *m;

	m = malloc(n);
	logmem('A', m, n, f, l);
	return m;
}


char *logdup(s, f, l)
char *s, *f;
unsigned l;
{
	char *m;

	m = strdup(s);
	logmem('S', m, strlen(s) + 1, f, l);
	return m;
}


void logfree(char *m, *f, unsigned l)
{
	logmem('F', m, 0, f, l);
	free(m);
}

static void logmem(char ch, char *m, unsigned n, char *f, unsigned l)
{
	if(logfp)
		fprintf(logfp, "%c %8x %4u  %s:%u\n", ch, m, n, f, l);
}
#endif
