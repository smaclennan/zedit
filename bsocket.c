/* bsocket.c - buffer socket functions
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
#include <sys/uio.h>

#include "buff.h"

#define MAX_IOVS 16

/* These can be used with files... but where written to use with
 * sockets.
 */

/** Read from a file descriptor using readv.  Simple version; only
 * reads at most two pages. Can be used for file but meant for
 * sockets.
 */
int bread(struct buff *buff, int fd)
{
	int n, n_read, left;
	struct page *npage;
	struct iovec iovs[2];

	if (buff->curchar != curplen(buff)) {
		/* Move data after point into new page */
		if (!pagesplit(buff, buff->curchar))
			return -ENOMEM;
	}

	left = PGSIZE - curplen(buff);

	/* try to fill current page */
	if (left) {
		iovs[0].iov_base = buff->curcptr;
		iovs[0].iov_len = left;
	} else
		iovs[0].iov_len = 0;

	/* plus a new page */
	npage = newpage(buff->curpage);
	if (!npage)
		return -ENOMEM;

	iovs[1].iov_base = npage->pdata;
	iovs[1].iov_len = PGSIZE;

	n = n_read = readv(fd, iovs, 2);

	if (n <= 0) {
		freepage(buff, npage);
		return n;
	}

	if (left) {
		int wrote = MIN(left, n);

		buff->curcptr += wrote;
		buff->curchar += wrote;
		curplen(buff) += wrote;
		n -= wrote;
	}

	if (n > 0) {
		makecur(buff, npage, n);
		curplen(buff) = n;
	} else
		freepage(buff, npage);

	return n_read;
}

/** Write to a file descriptor using writev.  Can be used for files but
 * meant for sockets. Leaves the point at the end of the write.
 */
int bwrite(struct buff *buff, int fd, unsigned size)
{
	struct iovec iovs[MAX_IOVS];
	struct page *pg;
	int i, n, amount, did = 0;

	do {
		unsigned have = curplen(buff) - buff->curchar;

		iovs[0].iov_base = buff->curcptr;
		iovs[0].iov_len = MIN(have, size);
		size -= iovs[0].iov_len;
		amount = iovs[0].iov_len;

		for (pg = buff->curpage->nextp, i = 1;
			 i < MAX_IOVS && size > 0 && pg;
			 ++i, pg = pg->nextp) {
			iovs[i].iov_base = pg->pdata;
			iovs[i].iov_len = MIN(pg->plen, size);
			size -= iovs[i].iov_len;
			amount += iovs[i].iov_len;
		}

		do
			n = writev(fd, iovs, i);
		while (n < 0 && errno == EINTR);

		if (n > 0) {
			bmove(buff, n);
			did += n;
		}
	} while (n == amount && size > 0);

	return did;
}

/* Some bulk insert routines that are handy with sockets. */

/* You must guarantee we are at the end of the page */
static int bappendpage(struct buff *buff, const Byte *data, int size)
{
	int appended = 0;

	/* Fill the current page */
	int n, left = PGSIZE - curplen(buff);

	if (left > 0) {
		n = MIN(left, size);
		memcpy(buff->curcptr, data, n);
		buff->curcptr += n;
		buff->curchar += n;
		curplen(buff) += n;
		size -= n;
		data += n;
		appended += n;
		undo_add(buff, n);
	}

	/* Put the rest in new pages */
	while (size > 0) {
		struct page *npage = newpage(buff->curpage);

		if (!npage)
			return appended;
		makecur(buff, npage, 0);

		n = MIN(PGSIZE, size);
		memcpy(buff->curcptr, data, n);
		curplen(buff) = n;
		makeoffset(buff, n);
		undo_add(buff, n);
		size -= n;
		data += n;
		appended += n;
	}

	return appended;
}

/** Append data to the end of the buffer. Point is left at the end of
 * the buffer. Returns how much data was actually appended.
 */
int bappend(struct buff *buff, const Byte *data, int size)
{
	btoend(buff);
	return bappendpage(buff, data, size);
}

/* Simple version to start.
 * Can use size / PGSIZE + 1 + 1 pages.
 */
/** Insert data at the current point. Point is left at the end of the
 * inserted data. Returns how much data was actually inserted.
 */
int bindata(struct buff *buff, Byte *data, unsigned size)
{
	struct page *npage;
	unsigned n, copied = 0;

	/* If we can append... use append */
	if (buff->curchar == curplen(buff))
		return bappendpage(buff, data, size);

	n = PGSIZE - curplen(buff);
	if (n >= size) { /* fits in this page */
		struct mark *m;

		n = curplen(buff) - buff->curchar;
		memmove(buff->curcptr + size, buff->curcptr, n);
		memcpy(buff->curcptr, data, size);
		foreach_global_pagemark(m, buff->curpage)
			if (m->moffset >= buff->curchar)
				m->moffset += size;
		foreach_pagemark(buff, m, buff->curpage)
			if (m->moffset >= buff->curchar)
				m->moffset += size;
		buff->curcptr += size;
		buff->curchar += size;
		curplen(buff) += size;
		undo_add(buff, size);
		return size;
	}

	n = curplen(buff) - buff->curchar;
	if (n > 0) {
		if (!pagesplit(buff, buff->curchar))
			return copied;

		/* Copy as much as possible to the end of this page */
		n = MIN(PGSIZE - buff->curchar, size);
		memcpy(buff->curcptr, data, n);
		data += n;
		size -= n;
		undo_add(buff, n);
		copied += n;
		buff->curcptr += n;
		buff->curchar += n;
		curplen(buff) = buff->curchar;
	}

	while (size > 0) {
		npage = newpage(buff->curpage);
		if (!npage)
			break;

		n = MIN(PGSIZE, size);
		memcpy(npage->pdata, data, n);
		data += n;
		size -= n;
		undo_add(buff, n);
		copied += n;

		makecur(buff, npage, n);
		curplen(buff) = n;
	}

	return copied;
}
