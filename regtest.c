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

void bmrktopnt(struct mark *tmark) { tmark->moffset = Curchar; }

void bpnttomrk(struct mark *tmark)
{
	Curchar = tmark->moffset;
	Curcptr = pdata + tmark->moffset;
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

/* The char *past* the match. If not found leaves at EOB.
 * If we are on a match find the next one.
 */
bool bcsearch(Byte what)
{
	char *is = (char *)Curcptr;
	if (*is == what) ++is;
	char *p = strchr(is, what);
	if (p) {
		++p;
		int len = p - is;
		Curcptr = (Byte *)p;
		Curchar += len;
		return true;
	}

	Curcptr = pdata + Curplen;
	Curchar = Curplen;

	return false;
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


/* Leaves us on the NL */
void toendline(void)
{
	if (bcsearch(NL))
		bmove(-1);
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

	printf("Processing: %s [%d]\n", fname, Curplen);

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

/* Poor man's grep using Zedit regex functions. */
int main(int argc, char *argv[])
{
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
		if (readfile(argv[arg]))
		    grepit(ebuf);

	return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -g -Wall regtest.c reg.c -o regtest"
 * End:
 */
