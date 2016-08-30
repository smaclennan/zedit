/* bfile.c - low level buffer file routines
 * Copyright (C) 1988-2016 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include "buff.h"

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

#if HUGE_FILES
#if ZLIB
#error HUGE_FILES and ZLIB not supported.
#endif

/* Warning: keeps the fd open. */
int breadhuge(struct buff *buff, int fd, struct stat *sbuf)
{
	int len, pages, i;
	struct page *page;

	/* always read the first page */
	len = fileread(fd, buff->curcptr, PSIZE);
	if (len <= 0) {
		close(fd);
		return EIO;
	}
	buff->curpage->plen = len;
	buff->fd = fd;

	/* Round up... but we have one page */
	pages = (sbuf->st_size + PSIZE - 1) / PSIZE;
	page = buff->curpage;
	for (i = 1; i < pages; ++i) {
		page = newpage(page);
		if (!page) {
			bempty(buff); /* will close fd */
			return ENOMEM;
		}
		page->pgoffset = i;
	}

	btostart(buff);

	return 0;
}

/* SAM If we where smart we would read the last partial page and check
 * that all the pages in between where of length PSIZE.
 */
static void breadpage(struct buff *buff, struct page *page)
{
	unsigned long offset;
	int len;

	if (page->pgoffset == 0 || buff->fd == -1)
		return;

	offset = page->pgoffset * PSIZE;
	if (lseek(buff->fd, offset, SEEK_SET) != offset)
		goto fatal;
	len = fileread(buff->fd, page->pdata, PSIZE);
	if (len < 0)
		goto fatal;

	page->plen = len;
	page->pgoffset = 0;
	return;

fatal:
	printf("\r\nFATAL I/O Error: page %u\r\n", page->pgoffset);
	exit(2);
}

void makecur(struct buff *buff, struct page *page, int dist)
{
	if (page->pgoffset) {
		breadpage(buff, page);
		if (dist > page->plen)
			dist = page->plen - 1;
	}
	buff->curpage = page;
	makeoffset(buff, dist);
}

void bhugecleanup(struct buff *buff)
{
	if (buff->fd >= 0) {
		close(buff->fd);
		buff->fd = -1;
	}
}
#endif

/**
 * Load the file 'fname' into the current buffer.  Returns 0
 * successfully opened file, > 0 (errno) on error, -1 on gzdopen
 * error.
 * Leaves point at start of buffer.
 */
int breadfile(struct buff *buff, const char *fname, int *compressed)
{
	char buf[PSIZE];
	int fd, len;
	struct stat sbuf;

	btostart(buff);

	fd = open(fname, O_RDONLY | O_BINARY);
	if (fd < 0)
		return errno;

	if (fstat(fd, &sbuf)) {
		close(fd);
		return EIO;
	}

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

#if HUGE_FILES
	if (sbuf.st_size > HUGE_SIZE) {
		if (compressed) *compressed = 1;
		return breadhuge(buff, fd, &sbuf);
	}
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
	}
	fileclose(fd);

	btostart(buff);

#if ZLIB
	/* Ubuntu 12.04 has a bug where zero length files are reported as
	 * compressed.
	 */
	if (compressed && sbuf.st_size == 0) *compressed = 0;
#endif

	buff->bmodf = false;

	return 0;
}

#if ZLIB
/** Write out a file compressed. */
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

/** Write out a file normally. */
static bool bwritefd(struct buff *buff, int fd)
{
	struct page *tpage;
	int n, status = true;

	for (tpage = buff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			makecur(buff, tpage, 0);
			n = write(fd, tpage->pdata, tpage->plen);
			status = n == tpage->plen;
		}

	close(fd);

	return status;
}

/** Write out a DOS file. Converts LF to CR LF. */
static bool bwritedos(struct buff *buff, int fd)
{
	struct page *tpage;
	int n, status = true;
	unsigned i;
	Byte buf[PSIZE * 2], *p;

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

	return status;
}

/** Write the buffer to the file 'fname'.
 * Mode is umask + FILE_COMPRESSED + FILE_CRLF
 * Returns:	true if write successful.
 * Leaves point at start of buffer.
 */
bool bwritefile(struct buff *buff, char *fname, int mode)
{
	int fd;
	bool status = false;

	if (!fname)
		goto done;

	/* Write the output file */
	if ((fd = creat(fname, mode & 0777)) < 0)
		goto done;

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

done:
	btostart(buff);
	return status;
}
