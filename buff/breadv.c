/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include <errno.h>
#ifndef WIN32
#include <sys/uio.h>
#endif

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

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
	npage = newpage(buff);
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
