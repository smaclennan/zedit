/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Low-level page function to create a new memory page and link into
 * chain after curpage.
 * @param buf The buffer to add the page to.
 * @return The new page or NULL.
 */
struct page *newpage(struct buff *buff)
{
	struct page *page = (struct page *)calloc(1, sizeof(struct page));

	if (page) {
		page->prevp = buff->curpage;
		if (buff->curpage->nextp) {
			page->nextp = buff->curpage->nextp;
			buff->curpage->nextp->prevp = page;
		} else
			buff->lastp = page;
		buff->curpage->nextp = page;
	}

	return page;
}

/* @} */
