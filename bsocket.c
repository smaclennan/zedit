#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/uio.h>

#include "buff.h"

/* These can be used with files... but where written to use with
 * sockets. */

/* Simple version. Optimized for appends. */
int bread(struct buff *buff, int fd)
{
	int n;
	struct page *npage;
	struct iovec iovs[2];

	if (buff->curchar != curplen(buff)) {
		/* Move data after point into new page */
		if (!pagesplit(buff, buff->curchar))
			return -ENOMEM;
	}

	int left = PSIZE - curplen(buff);

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
	iovs[1].iov_len = PSIZE;

	n = readv(fd, iovs, 2);

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

	return n;
}

/* Writes are easy! Leaves the point at the end of the write. */
int bwrite(struct buff *buff, int fd, int size)
{
	struct iovec iovs[MAX_IOVS];
	struct page *pg;
	int i, n;

	int have = curplen(buff) - buff->curchar;
	iovs[0].iov_base = buff->curcptr;
	iovs[0].iov_len = MIN(have, size);
	size -= iovs[0].iov_len;

	for (pg = buff->curpage->nextp, i = 1;
		 i < MAX_IOVS && size > 0 && pg;
		 ++i, pg = pg->nextp) {
		iovs[i].iov_base = pg->pdata;
		iovs[i].iov_len = MIN(pg->plen, size);
		size -= iovs[i].iov_len;
	}

	do
		n = writev(fd, iovs, i);
	while (n < 0 && errno == EINTR);

	if (n > 0)
		bmove(buff, n);

	return n;
}

bool blookingat(struct buff *buff, const Byte *str)
{
	int moved = 0; /* SAM worth it to add mark? */

	while (*buff->curcptr == ' ' || *buff->curcptr == '\t') {
		bmove1(buff);
		++moved;
	}
	while (*str)
		if (*buff->curcptr == *str) {
			++str;
			bmove1(buff);
			++moved;
		}
	if (*str) bmove(buff, -moved);

	return *str ? false : true;
}
