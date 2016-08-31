/* hugefile.c - Deal with huge files
 * Copyright (C) 2016 Sean MacLennan <seanm@seanm.ca>
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

#if HUGE_FILES
#if ZLIB
#error HUGE_FILES and ZLIB not supported.
#endif

/* Warning: keeps the fd open. */
int breadhuge(struct buff *buff, int fd, unsigned long size)
{
	int len, pages, i;
	struct page *page;

	/* always read the first page */
	len = read(fd, buff->curcptr, PSIZE);
	if (len <= 0) {
		close(fd);
		return EIO;
	}
	buff->curpage->plen = len;
	buff->fd = fd;

	/* Round up... but we have one page */
	pages = (size + PSIZE - 1) / PSIZE;
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
	len = read(buff->fd, page->pdata, PSIZE);
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
