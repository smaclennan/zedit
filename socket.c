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

#define BREAD_THRESHOLD (PSIZE / 4)

/* Simple version. Can do multiple reads! */
int bread(struct buff *buff, int fd, int size)
{
	int n, ret = 0, left = PSIZE - curplen(buff);

	/* try to fill current page */
	if (left >= size || left > BREAD_THRESHOLD) {
		if (buff->curchar != curplen(buff)) {
			/* Move data after point into new page */
			if (!pagesplit(buff, buff->curchar))
				return -ENOMEM;
			left = PSIZE - buff->curchar;
		}
		n = read(fd, buff->curcptr, MIN(size, left));
		if (n <= 0)
			return n;
		buff->curchar += n;
		buff->curcptr += n;
		curplen(buff) += n;
		ret += n;
		size -= n;
	}

	/* now use full pages */
	while (size > 0) {
		struct page *npage = newpage(buff->curpage);
		if (!npage)
			return ret > 0 ? ret : -ENOMEM;
		n = read(fd, npage->pdata, MIN(size, PSIZE));
		if (n > 0) {
			makecur(buff, npage, n);
			curplen(buff) = n;
		} else {
			freepage(buff, npage);
			return ret > 0 ? ret : n;
		}
	}

	return ret;
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

