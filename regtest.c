#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "z.h"

/* Dummy's */
struct buff thebuff;
struct page thepage;
struct mark themark;

struct buff *Curbuff = &thebuff;
struct page *Curpage = &thepage;

/* Needed */
Byte *pdata; /* What Curcptr points into */
int pdatalen;

int Curchar, Curplen;
Byte *Curcptr;
Byte *Cpstart;

void bmrktopnt(struct mark *tmark) { tmark->moffset = Curchar; }

void bpnttomrk(struct mark *tmark)
{
	Curchar = tmark->moffset;
	Curcptr = pdata + tmark->moffset;
}

/* Make the point be dist chars into the page. */
void makeoffset(int dist)
{
	Curchar = dist;
	Curcptr = Cpstart + dist;
}

void bswappnt(struct mark *tmark)
{
	int tmp = Curchar;
	bpnttomrk(tmark);
	tmark->moffset = tmp;
}

bool bisatmark(struct mark *tmark)    { return Curchar == tmark->moffset; }
bool bisbeforemrk(struct mark *tmark) { return Curchar < tmark->moffset; }
bool bisaftermrk(struct mark *tmark)  { return Curchar > tmark->moffset; }


void bmove1(void)
{
	if (Curchar < Curplen) {
		++Curchar;
		++Curcptr;
	}
}

/* The char *past* the match. If not found leaves at EOB. */
bool bcsearch(Byte what)
{
	Byte *n;

	if (bisend())
		return false;

	if ((n = (Byte *)memchr(Curcptr, what, Curplen - Curchar)) == NULL) {
		makeoffset(Curplen);
		return false;
	}

	makeoffset(n - Cpstart);
	bmove1();
	return true;
}

bool bcrsearch(Byte what)
{
	while (Curchar >= 0) {
		if (*Curcptr == what)
			return true;
		if (!bmove(-1))
			return false;
	}

	return false;
}

void tobegline(void)
{
	if (Curchar > 0 && *(Curcptr - 1) == NL)
		return;
	if (bcrsearch(NL))
		bmove1();
}

/* Leaves us on the NL */
void toendline(void)
{
	if (bcsearch(NL))
		bmove(-1);
}

bool bmove(int dist)
{
	if (dist > 0) {
		if (dist < (Curplen - Curchar)) {
			Curcptr += dist;
			Curchar += dist;
			return true;
		} else {
			Curcptr = pdata + Curplen;
			Curchar = Curplen;
			return false;
		}
	} else {
		dist = -dist;
		if (dist <= Curchar) {
			Curcptr -= dist;
			Curchar -= dist;
			return true;
		} else {
			Curcptr = pdata;
			Curchar = 0;
			return false;
		}
	}
}

Byte bpeek(void)
{
	if (Curchar > 0)
		return *(Curcptr - 1);
	else
		/* Pretend we are at the start of a line.
		 * Needed for delete-to-eol and step in reg.c. */
		return NL;
}

static bool readfile(char *fname)
{
	int fd = open(fname, O_RDONLY);
	if (fd < 0) {
		perror(fname);
		return false;
	}

	Curplen = lseek(fd, 0, SEEK_END);
	if (Curplen == -1 || lseek(fd, 0, SEEK_SET) == -1) {
		perror("seeks");
		goto failed;
	}

	if (pdata && pdatalen < Curplen) {
		free(pdata);
		pdata = NULL;
	}

	if (!pdata) {
		pdata = malloc(Curplen + 1);
		if (!pdata) {
			perror("Out of memory\n");
			goto failed;
		}
		pdatalen = Curplen;
	}

	int n = read(fd, pdata, Curplen);

	close(fd);

	if (n != Curplen) {
		printf("Short read: %d/%d\n", n, Curplen);
		return false;
	}

	pdata[n] = '\0';

	Curchar = 0;
	Curcptr = pdata;
	Cpstart = pdata;

	// printf("Processing: %s [%d]\n", fname, Curplen);

	return true;

failed:
	close(fd);
	return false;
}

static void grepit(Byte *ebuf)
{
	while (step(ebuf)) {
		Byte was = *Curcptr;
		*Curcptr = '\0';
		printf("Matched '%s'\n", pdata + REstart->moffset);
		*Curcptr = was;
	}
}

static void zgrepit(Byte *ebuf, const char *fname)
{
	while (step(ebuf)) {
		printf("%s:", fname);
		tobegline();
		while (*Curcptr != '\n' && !bisend()) {
			putchar(*Curcptr);
			bmove(1);
		}
		putchar('\n');
		bmove1();
	}
}

static bool is_zgrep(char *prog)
{
	char *p = strrchr(prog, '/');
	if (p)
		++p;
	else
		p = prog;
	return strcmp(p, "zgrep") == 0;
}

/* Poor man's grep using Zedit regex functions. */
int main(int argc, char *argv[])
{
	bool zgrep = is_zgrep(argv[0]);

	if (argc < 2) {
		puts("I need a regular expression and a file.");
		exit(1);
	}

	/* Some fixups */
	REstart = &themark;
	Curbuff->firstp = Curbuff->lastp = Curpage;

	Byte ebuf[ESIZE];

	int rc = compile((Byte *)argv[1], ebuf, &ebuf[ESIZE]);
	if (rc) {
		regerr(rc);
		exit(1);
	}

	int arg;
	for (arg = 2; arg < argc; ++arg)
		if (readfile(argv[arg])) {
			if (zgrep)
				zgrepit(ebuf, argv[arg]);
			else
				grepit(ebuf);
		}

	return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -g -Wall regtest.c reg.c -o regtest"
 * End:
 */
