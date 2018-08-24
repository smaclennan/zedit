#include "buff.h"

/** Create a new memory page and link into chain after curpage. */
struct page *newpage(struct page *curpage)
{
	struct page *page = (struct page *)calloc(1, sizeof(struct page));

	if (!page)
		return NULL;

#ifdef PAGE_DBG
	memset(page->pdata, 'x', PGSIZE);
#endif

	if (curpage) {
		page->prevp = curpage;
		if (curpage->nextp) {
			page->nextp = curpage->nextp;
			curpage->nextp->prevp = page;
		}
		curpage->nextp = page;
	}

	return page;
}
