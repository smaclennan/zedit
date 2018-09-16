/* breadv.c - socket (or file) readv
 * Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software; you can redistribute it and/or modify
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

#include <errno.h>
#ifndef WIN32
#include <sys/uio.h>
#endif

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

#ifdef WIN32
/* Not optimal... but should work */
struct iovec {
	void *iov_base;
	size_t iov_len;
};

int readv(int fd, const struct iovec *iov, int iovcnt)
{
	int i, n, n_read = 0;

	for (i = 0; i < iovcnt; ++i, ++iov)
		if (iov->iov_len) {
			n = read(fd, iov->iov_base, iov->iov_len);
			if (n < 0)
				return n;
			else if (n == 0)
				return n_read;
			n_read += n;
		}

	return n_read;
}
#endif

/** Read from a file descriptor using readv. Simple version; only
 * reads at most two pages. Can be used for files but meant for
 * sockets.
 * @param buff The buffer to write to.
 * @param fd The open file descriptor to read from.
 * @return The number of bytes read or -1 on error. errno is set on
 * error.
 */
int breadv(struct buff *buff, int fd)
{
	int n, n_read, left;
	struct page *npage;
	struct iovec iovs[2];

	if (buff->curchar != curplen(buff)) {
		/* Move data after point into new page */
		if (!pagesplit(buff, buff->curchar)) {
			errno = ENOMEM;
			return -1;
		}
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
	if (!npage) {
		errno = ENOMEM;
		return -1;
	}

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
/* @} */
