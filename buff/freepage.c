#include "buff.h"

/** Free a memory page. */
void freepage(struct buff *buff, struct page *page)
{
	if (page->nextp)
		page->nextp->prevp = page->prevp;
	if (page->prevp)
		page->prevp->nextp = page->nextp;
	else if (buff)
		buff->firstp = page->nextp;

	free(page);
}
