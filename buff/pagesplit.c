/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Low-level page function to split the current full page and return
 * a new page. Leaves dist in curpage. Point is moved to the new page
 * if required.
 * @param buff The buffer the Point is in.
 * @param dist The amount to leave in the old page (usually
 * HALFP). Must be <= PGSIZE.
 * @return The new page or NULL.
 */
struct page *pagesplit(struct buff *buff, unsigned int dist)
{
	struct page *curpage, *newp;
	struct mark *btmark;
	int newsize;

	if (dist > PGSIZE)
		return NULL;

	newp = newpage(buff);
	if (!newp)
		return NULL;

	curpage = buff->curpage;
	newsize = curpage->plen - dist;
	memcpy(newp->pdata, curpage->pdata + dist, newsize);
	curpage->plen = dist;
	newp->plen = newsize;

	foreach_global_pagemark(btmark, curpage)
		if (btmark->moffset >= dist) {
			btmark->mpage = newp;
			btmark->moffset -= dist;
		}
	foreach_pagemark(buff, btmark, curpage)
		if (btmark->moffset >= dist) {
			btmark->mpage = newp;
			btmark->moffset -= dist;
		}

	if (buff->curchar > dist)
		/* new page has Point in it */
		makecur(buff, newp, buff->curchar - dist);

	return newp;
}
/* @} */
