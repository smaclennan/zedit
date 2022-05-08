/* btest.c - test program (not part of libbuff)
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
#include "tinit.h"

#ifdef WIN32
#define FILENAME "btest.file"
#define FILENAME2 "btest2.file"
#else
#define FILENAME  "/tmp/btest.file"
#define FILENAME2 "/tmp/btest2.file"
#endif

void dump_pages(struct buff *buff, const char *str)
{
	printf("Pages %s:\n", str);
	for (struct page *pg = buff->firstp; pg; pg = pg->nextp)
		if (buff->curpage == pg) {
			printf("  '%.*s'\n", pg->plen, pg->pdata);
			printf("   ");
			for (int i = 0; i < buff->curchar; ++i)
				putchar(' ');
			printf("^\n");
		} else
			printf("  '%.*s'\n", pg->plen, pg->pdata);
}

void create_file(const char *fname, const char *str)
{
	int fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if (fd < 0) {
		perror(fname);
		exit(1);
	}

	int n, expected;
	if (str) {
		expected = strlen(str);
		n = write(fd, str, expected);
	} else {
		char buff[PGSIZE];
		memset(buff, 'R', sizeof(buff));
		n = write(fd, buff, sizeof(buff));
		expected = sizeof(buff);
	}
	close(fd);

	if (n != expected) {
		puts("Create file failed");
		exit(1);
	}
}

void bpeek_test(void)
{
	struct buff *buff = bcreate();
	assert(buff);

	// Put some data in the first page
	memset(buff->firstp->pdata, 'a', 9);
	buff->firstp->pdata[10] = 'b';
	buff->firstp->plen = 11;

	// Create a newpage and make it current
	struct page *new = newpage(buff, buff->firstp);
	makecur(buff, new, 0);
	binsert(buff, 'c');
	bmove(buff, -1);

	// We should now be sitting on the first byte of the new page (c)
	assert(buff->curpage == buff->firstp->nextp);
	assert(buff->curchar == 0);
	assert(*buff->curcptr == 'c');

	// So the bpeek() should be the last char of the previous page
	assert(bpeek(buff) == 'b');

	// Start of buffer should be LF
	btostart(buff);
	assert(bpeek(buff) == '\n');

	// Test simple offset
	*buff->curcptr = 'd';
	bmove(buff, 1);
	assert(bpeek(buff) == 'd');
}

int main(int argc, char *argv[])
{
#if 0
	bpeek_test();
#else
	struct buff *buff = { 0 };
	struct page *page = newpage(buff, NULL);
	assert(page);
	struct page *last = newpage(buff, page);
	assert(page);
	struct page *next = newpage(buff, page);
	assert(last);

	assert(page->prevp == NULL);
	assert(page->nextp == next);
	assert(next->prevp == page);
	assert(next->nextp == last);
	assert(last->prevp == next);
	assert(last->nextp == NULL);

	freepage(NULL, page);
	assert(next->prevp == NULL);

	create_file(FILENAME, NULL);

	struct buff *buff = bcreate();
	int rc = breadfile(buff, FILENAME, NULL);
	assert(rc == 0);
	dump_pages(buff, "empty");
	bdelbuff(buff);

	// half full page
	buff = bcreate();
	memset(buff->curpage->pdata, 'B', PGSIZE);
	memset(buff->curpage->pdata, 'A', HALFP);
	buff->curpage->pdata[PGSIZE - 1] = '\n';
	makeoffset(buff, HALFP);
	curplen(buff) = PGSIZE;
	rc = breadfile(buff, FILENAME, NULL);
	assert(rc == 0);
	dump_pages(buff, "half");
	assert(bwritefile(buff, FILENAME2, 0644));
	bdelbuff(buff);

	// full page
	buff = bcreate();
	memset(buff->curpage->pdata, 'A', PGSIZE);
	makeoffset(buff, PGSIZE);
	curplen(buff) = PGSIZE;
	rc = breadfile(buff, FILENAME, NULL);
	assert(rc == 0);
	dump_pages(buff, "full");
	bdelbuff(buff);

	unlink(FILENAME);
	unlink(FILENAME2);

	const char *teststr = "The quick brown fox jumped over the lazy dog.";
	buff = bcreate();
	for (const char *p = teststr; *p; ++p)
		binsert(buff, *p);
	btostart(buff);
	assert(bm_search(buff, "fox", 0));
	assert(bcsearch(buff, 'l')); // leaves the point after the l
	bmove(buff, -1);
	bdelete(buff, 4);
	dump_pages(buff, "search/delete");
	bdelbuff(buff);

	buff = bcreate();
	binstr(buff, "int %d", 666);
	binstr(buff, " unsigned %u", 666);
	dump_pages(buff, "binstr");
	bdelbuff(buff);

	create_file(FILENAME, "new ");
	buff = bcreate();
	binstr(buff, "Hello world");
	bmove(buff, -5);
	assert(breadfile(buff, FILENAME, NULL) == 0);
	dump_pages(buff, "breadfile");
	bdelbuff(buff);

// SAM	tinit();

	printf("buffer size %ld\n", sizeof(struct buff));
#endif

#ifdef WIN32
	printf("Hit return to exit "); getchar();
#endif

	return 0;
}
