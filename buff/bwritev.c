/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include <errno.h>
#ifndef WIN32
#include <sys/uio.h>
#endif

#include "buff.h"

#define MAX_IOVS 16

/** Write to a file descriptor using writev.  Can be used for files but
 * meant for sockets. Leaves the point at the end of the write.
 * @param buff The buffer to read from.
 * @param fd The open file descriptor to write to.
 * @param size The number of bytes to write.
 * @return The number of bytes actually written.
 */
int bwritev(struct buff *buff, int fd, unsigned int size)
{
	struct iovec iovs[MAX_IOVS];
	struct page *pg;
	int i, n, amount, did = 0;

	do {
		unsigned int have = curplen(buff) - buff->curchar;

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
