/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Low-level page function to free a memory page.
 * @param buff The buffer to free the page from. Can be NULL.
 * @param page The page to free. Can be NULL.
 */
void freepage(struct buff *buff, struct page *page)
{
	if (!page)
		return;

	if (page->nextp)
		page->nextp->prevp = page->prevp;
	else
		buff->lastp = page->prevp;
	if (page->prevp)
		page->prevp->nextp = page->nextp;
	else if (buff)
		buff->firstp = page->nextp;

	free(page);
}
/* @} */
