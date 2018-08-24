#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include "buff.h"

#define FILENAME  "/tmp/btest.file"
#define FILENAME2 "/tmp/btest2.file"

static void dump_pages(struct buff *buff, const char *str)
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

static void create_file(const char *fname)
{
	int fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if (fd < 0) {
		perror(fname);
		exit(1);
	}

	char buff[PGSIZE];
	memset(buff, 'R', sizeof(buff));
	int n = write(fd, buff, sizeof(buff));
	close(fd);

	if (n != sizeof(buff)) {
		puts("Create file failed");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	struct page *page = newpage(NULL);
	assert(page);
	struct page *last = newpage(page);
	assert(page);
	struct page *next = newpage(page);
	assert(last);

	assert(page->prevp == NULL);
	assert(page->nextp == next);
	assert(next->prevp == page);
	assert(next->nextp == last);
	assert(last->prevp == next);
	assert(last->nextp == NULL);

	freepage(NULL, page);
	assert(next->prevp == NULL);

	create_file(FILENAME);

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
	assert(bm_search(buff, "fox", false));
	assert(bcsearch(buff, 'l')); // leaves the point after the l
	bmove(buff, -1);
	bdelete(buff, 4);
	dump_pages(buff, "search/delete");
	bdelbuff(buff);

	return 0;
}
