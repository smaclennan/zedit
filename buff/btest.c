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


int main(int argc, char *argv[])
{
	struct page *page = newpage(NULL);
	assert(page);
	struct page *next = newpage(page);
	assert(page);

	assert(page->prevp == NULL);
	assert(page->nextp == next);
	assert(next->prevp == page);
	assert(next->nextp == NULL);

	return 0;
}
