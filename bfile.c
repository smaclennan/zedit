#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "z.h"

#if ZLIB
#undef Byte
#include <zlib.h>

#define fileread(a, b, c) gzread(gz, b, c)
#define fileclose(a) gzclose(gz)
#else
#define fileread(a, b, c) read(a, b, c)
#define fileclose(a) close(a)
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif
#define READ_MODE	(O_RDONLY | O_BINARY)
#define WRITE_MODE	(O_WRONLY | O_CREAT | O_TRUNC | O_BINARY)

int raw_mode;

/*
 * Load the file 'fname' into the current buffer.
 * Returns  0  successfully opened file
 * > 0 (errno) on error
 * -1 on gzdopen error
 */
int breadfile(struct buff *buff, const char *fname, int *compressed)
{
	char buf[PSIZE];
	int fd, len;
	unsigned count = 0; /* to check for zero length files */

	fd = open(fname, O_RDONLY | O_BINARY);
	if (fd < 0)
		return errno;

	bempty(buff);

#if ZLIB
	gzFile gz = gzdopen(fd, "rb");
	if (!gz) {
		close(fd);
		return -1;
	}

	if (compressed) *compressed = gzdirect(gz) == 0;
#endif

	while ((len = fileread(fd, buf, PSIZE)) > 0) {
		if (curplen(buff)) {
			if (!newpage(buff->curpage)) {
				bempty(buff);
				fileclose(fd);
				return ENOMEM;
			}
			makecur(buff, buff->curpage->nextp, 0);
		}
		memcpy(buff->curcptr, buf, len);
		buff->curcptr += len;
		buff->curchar += len;
		curplen(buff) += len;
		count += len;
	}
	fileclose(fd);

	btostart(buff);

	/* Ubuntu 12.04 has a bug where zero length files are reported as
	 * compressed.
	 */
	if (compressed && count == 0) *compressed = 0;

	buff->bmodf = false;

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

	for (tpage = Bbuff->firstp; tpage && status; tpage = tpage->nextp)
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

	bmrktopnt(Bbuff, &smark);
	for (tpage = Bbuff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			makecur(Bbuff, tpage, 0);
			n = write(fd, tpage->pdata, tpage->plen);
			status = n == tpage->plen;
		}

	close(fd);

	bpnttomrk(Bbuff, &smark);
	return status;
}

static bool bwritedos(int fd)
{
	struct mark smark;
	struct page *tpage;
	int n, status = true;
	unsigned i;
	Byte buf[PSIZE * 2], *p;

	bmrktopnt(Bbuff, &smark);
	for (tpage = Bbuff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			int len = tpage->plen;
			makecur(Bbuff, tpage, 0);
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

	bpnttomrk(Bbuff, &smark);
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
		Bbuff->bmodf = false;
	}

	return status;
}
