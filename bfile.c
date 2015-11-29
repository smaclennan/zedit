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
#else
	if (compressed) *compressed = 0;
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

#if ZLIB
	/* Ubuntu 12.04 has a bug where zero length files are reported as
	 * compressed.
	 */
	if (compressed && count == 0) *compressed = 0;
#endif

	buff->bmodf = false;

	return 0;
}

#if ZLIB
static bool bwritegzip(struct buff *buff, int fd)
{
	struct page *tpage;
	bool status = true;

	gzFile gz = gzdopen(fd, "wb");
	if (!gz) {
		close(fd);
		return false;
	}

	for (tpage = buff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			int n = gzwrite(gz, tpage->pdata, tpage->plen);
			status = n == tpage->plen;
		}

	gzclose(gz); /* also closes fd */

	return status;
}
#endif

static bool bwritefd(struct buff *buff, int fd)
{
	struct mark smark;
	struct page *tpage;
	int n, status = true;

	bmrktopnt(buff, &smark);
	for (tpage = buff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			makecur(buff, tpage, 0);
			n = write(fd, tpage->pdata, tpage->plen);
			status = n == tpage->plen;
		}

	close(fd);

	bpnttomrk(buff, &smark);
	return status;
}

static bool bwritedos(struct buff *buff, int fd)
{
	struct mark smark;
	struct page *tpage;
	int n, status = true;
	unsigned i;
	Byte buf[PSIZE * 2], *p;

	bmrktopnt(buff, &smark);
	for (tpage = buff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			int len = tpage->plen;
			makecur(buff, tpage, 0);
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

	bpnttomrk(buff, &smark);
	return status;
}

/* Write the buffer to the file 'fname'.
 * Mode is umask + FILE_COMPRESSED + FILE_CRLF
 * Returns:	true if write successful.
 */
bool bwritefile(struct buff *buff, char *fname, int mode)
{
	int fd;
	bool status;

	if (!fname)
		return true;

	/* Write the output file */
	if ((fd = creat(fname, mode & 0777)) < 0)
		return false;

#if ZLIB
	if (mode & FILE_COMPRESSED)
		status = bwritegzip(buff, fd);
	else
#endif
		if (mode & FILE_CRLF)
			status = bwritedos(buff, fd);
		else
			status = bwritefd(buff, fd);

	/* cleanup */
	if (status)
		buff->bmodf = false;

	return status;
}
