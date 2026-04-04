#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include "buff.h"

void dump_pages(struct buff *buff)
{
	puts("Pages:");
	for (struct page *pg = buff->firstp; pg; pg = pg->nextp) {
		printf(" %4d '%.*s'\n", pg->plen, pg->plen, pg->pdata);
		if (buff->curpage == pg) {
			printf("   ");
			for (int i = 0; i < buff->curchar; ++i)
				putchar(' ');
			printf("^\n");
		}
	}
}

int main(int argc, char *argv[])
{
	struct buff *buff = bcreate();
	assert(buff);

	//
	// Edge conditions in one page or last page
	//

	bdelete1(buff);
	assert(curplen(buff) == 0);

	binsert(buff, 'a');
	bdelete1(buff);
	assert(curplen(buff) == 1); // nothing to delete
	bmove(buff, -1);
	bdelete1(buff);
	assert(curplen(buff) == 0); // nothing to delete

	binstr(buff, "aaaabcbaaaa");
	bmove(buff, -6); // point at c
	bdelete1(buff);
	assert(strncmp(buff->curpage->pdata, "aaaabbaaaa", curplen(buff)) == 0);

	bmove(buff, -5); // first byte in page
	bdelete1(buff);
	assert(strncmp(buff->curpage->pdata, "aaabbaaaa", curplen(buff)) == 0);

	//
	// Multiple pages
	//

	bempty(buff);

	char pg1[PGSIZE], pg2[PGSIZE];
	char *pg1p = pg1, *pg2p = pg2;

	for (int i = 0; i < PGSIZE / 2 - 2; ++i) {
		binsert(buff, '1');
		*pg1p++ = '1';
	}
	binsert(buff, 'a');
	*pg1p++ = 'a';
	binsert(buff, 'b');
	*pg1p++ = 'b';
	*pg1p = '\0';
	// Page boundary after split
	struct mark *pmark = bcremark(buff);
	binsert(buff, 'c');
	*pg2p++ = 'c';
	binsert(buff, 'd');
	*pg2p++ = 'd';
	for (int i = 0; i < PGSIZE / 2 - 1; ++i) {
		binsert(buff, '2');
		*pg2p++ = '2';
	}
	*pg2p = '\0';

	// First byte in second page
	bpnttomrk(buff, pmark);
	bdelete1(buff);
	assert(strncmp(buff->firstp->pdata, pg1, buff->firstp->plen) == 0);
	assert(strncmp(buff->lastp->pdata,  pg2 + 1, buff->lastp->plen) == 0);

	// Last byte in first page
	bmove(buff, -1);
	bdelete1(buff);
	assert(strncmp(buff->firstp->pdata, pg1, buff->firstp->plen) == 0);
	assert(strncmp(buff->lastp->pdata,  pg2 + 1, buff->lastp->plen) == 0);

	// Last byte in last page - nop
	btoend(buff);
	bdelete1(buff);
	assert(strncmp(buff->firstp->pdata, pg1, buff->firstp->plen) == 0);
	assert(strncmp(buff->lastp->pdata,  pg2 + 1, buff->lastp->plen) == 0);

	puts("Success");
	return 0;
}
