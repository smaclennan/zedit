#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "buff.h"
#include "mark.h"
#include "page.h"

#ifdef ZEDIT
#include "z.h"
#else
#define COMPRESSED			0x0008
#define CRLF				0x0010
#endif

#if ZLIB
#undef Byte
#include <zlib.h>

#define bread(a, b, c) gzread(gz, b, c)
#define bclose(a) gzclose(gz)
#else
#define bread(a, b, c) read(a, b, c)
#define bclose(a) close(a)
#endif

int raw_mode;

static void crfixup(void)
{
	char *p = (char *)memchr(Cpstart + 1, '\n', Curpage->plen - 1);
	if (!p)
		return;

	if (*(p - 1) != '\r')
		return;

	if (raw_mode)
		return;

	Curbuff->bmode |= CRLF;

	while (bcsearch('\r'))
		if (*Curcptr == '\n') {
			bmove(-1);
			bdelete(1);
		}

	btostart();
}

/*
 * Load the file 'fname' into the current buffer.
 * Returns  0  successfully opened file
 * > 0 (errno) on error
 * -1 on gzdopen error
 */
int breadfile(const char *fname)
{
	char buf[PSIZE];
	struct stat sbuf;
	int fd, len;

	fd = open(fname, O_RDONLY | O_BINARY);
	if (fd < 0)
		return errno;

	if (fstat(fd, &sbuf) == 0)
		Curbuff->mtime = sbuf.st_mtime;
	else
		Curbuff->mtime = -1;

	bempty();

#if ZLIB
	gzFile gz = gzdopen(fd, "rb");
	if (!gz) {
		close(fd);
		return -1;
	}

	/* Ubuntu 12.04 has a bug where zero length files are reported as
	 * compressed.
	 */
	if (sbuf.st_size && gzdirect(gz) == 0)
		Curbuff->bmode |= COMPRESSED;
#endif

	while ((len = bread(fd, buf, PSIZE)) > 0) {
		Curmodf = true;
		if (Curpage->plen) {
			if (!newpage(Curpage)) {
				bempty();
				bclose(fd);
				return ENOMEM;
			}
			makecur(Curbuff, Curpage->nextp, 0);
		}
		memcpy(Curcptr, buf, len);
		Curcptr += len;
		Curchar += len;
		Curpage->plen += len;
	}
	(void)bclose(fd);

	btostart();

	if (Curpage->plen && !(Curbuff->bmode & COMPRESSED))
		crfixup();

	Curbuff->bmodf = false;

	return 0;
}

#if ZLIB
static bool bwritegzip(int fd)
{
	struct page *tpage;
	int status = true;

	gzFile gz = gzdopen(fd, "wb");
	if (!gz) {
		close(fd);
		return false;
	}

	for (tpage = Curbuff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			int n = gzwrite(gz, tpage->pdata, tpage->plen);
			status = n == tpage->plen;
		}

	gzclose(gz); /* also closes fd */

	return status;
}
#endif

static bool bwritefd(int fd)
{
	struct mark smark;
	struct page *tpage;
	int n, status = true;

	bmrktopnt(&smark);
	for (tpage = Curbuff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			makecur(Curbuff, tpage, 0); /* DOS_EMS requires */
			n = write(fd, tpage->pdata, tpage->plen);
			status = n == tpage->plen;
		}

	close(fd);

	bpnttomrk(&smark);
	return status;
}

static bool bwritedos(int fd)
{
	struct mark smark;
	struct page *tpage;
	int i, n, status = true;
	Byte buf[PSIZE * 2], *p;

	bmrktopnt(&smark);
	for (tpage = Curbuff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			int len = tpage->plen;
			makecur(Curbuff, tpage, 0); /* DOS_EMS requires */
			p = buf;
			for (i = 0; i < tpage->plen; ++i) {
				if (tpage->pdata[i] == '\n') {
					*p++ = '\r';
					++len;
				}
				*p++ = tpage->pdata[i];
			}

			n = write(fd, buf, len);
			status = n == len;
		}

	close(fd);

	bpnttomrk(&smark);
	return status;
}

/*	Write the current buffer to the file 'fname'.
 *	Handles the backup scheme according to VAR(VBACKUP).
 *	Returns:	true	if write successful
 *				false	if write failed
 */
bool bwritefile(char *fname)
{
	static int Cmask;
	int fd, mode, status = true;
	struct stat sbuf;

	if (!fname)
		return true;

	/* If the file existed, check to see if it has been modified. */
	if (Curbuff->mtime && stat(fname, &sbuf) == 0) {
		if (sbuf.st_mtime > Curbuff->mtime) {
			/* file has been modified */
		}
		mode  = sbuf.st_mode;
	} else {
		if (Cmask == 0) {
			Cmask = umask(0);	/* get the current umask */
			umask(Cmask);		/* set it back */
			Cmask = ~Cmask & 0666;	/* make it usable */
		}
		mode  = Cmask;
	}

	/* Write the output file */
	fd = open(fname, WRITE_MODE, mode);
	if (fd != EOF) {
#if ZLIB
		if (Curbuff->bmode & COMPRESSED)
			status = bwritegzip(fd);
		else
#endif
			if (Curbuff->bmode & CRLF)
				status = bwritedos(fd);
			else
				status = bwritefd(fd);
	} else
		status = false;

	/* cleanup */
	if (status) {
		if (stat(fname, &sbuf) == 0)
			Curbuff->mtime = sbuf.st_mtime;
		else
			Curbuff->mtime = -1;
		Curbuff->bmodf = false;
	}

	return status;
}
