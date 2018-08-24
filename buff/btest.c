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

#define FILENAME "/tmp/btest.file"

static void dump_pages(struct buff *buff, const char *str)
{
	printf("Pages %s:\n", str);
	for (struct page *pg = buff->firstp; pg; pg = pg->nextp)
		if (buff->curpage == pg) {
			printf("  >%.*s\n", PGSIZE, pg->pdata);
			printf("   ");
			for (int i = 0; i < buff->curchar; ++i)
				putchar(' ');
			printf("^\n");
		} else
			printf("   %.*s\n", PGSIZE, pg->pdata);
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
	makeoffset(buff, HALFP);
	curplen(buff) = PGSIZE;
	rc = breadfile(buff, FILENAME, NULL);
	assert(rc == 0);
	dump_pages(buff, "half");
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

	return 0;
}
