/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

static void delete_entire_page(struct buff *buff, struct page *curpage)
{	/* We deleted the entire page. */
	struct mark *tmark;
	struct page *tpage = curpage->nextp;
	unsigned int noffset = 0;

	if (tpage == NULL) {
		tpage = curpage->prevp;
		noffset = tpage->plen;
	}
	foreach_global_pagemark(tmark, curpage) {
		tmark->mpage = tpage;
		tmark->moffset = noffset;
	}
	foreach_pagemark(buff, tmark, curpage) {
		tmark->mpage = tpage;
		tmark->moffset = noffset;
	}
	freepage(buff, curpage);
	makecur(buff, tpage, noffset);
}

/** Delete byte(s) from the buffer at the current point.
 * @param buff The buffer to delete from.
 * @param quantity The number of bytes to delete.
 */
void bdelete(struct buff *buff, unsigned int quantity)
{
	unsigned int quan, noffset;
	struct mark *tmark;
	struct page *tpage, *curpage = buff->curpage;

	while (quantity > 0) {
		/* Delete as many characters as possible from this page */
		quan = MIN(curplen(buff) - buff->curchar, quantity);

		undo_del(buff, quan);

		curplen(buff) -= quan;

		memmove(buff->curcptr,
			buff->curcptr + quan,
			curplen(buff) - buff->curchar);
		if (lastp(curpage))
			quantity = 0;
		else
			quantity -= quan;
		buff->bmodf = 1;

		if (curplen(buff) == 0 && (curpage->nextp || curpage->prevp)) {
			delete_entire_page(buff, curpage);
			continue;
		}

		tpage = curpage;
		noffset = buff->curchar;
		if ((noffset >= curplen(buff)) && curpage->nextp) {
			tpage = curpage->nextp;
			noffset = 0;
		}
		foreach_global_pagemark(tmark, curpage)
			if (tmark->moffset >= buff->curchar) {
				if (tmark->moffset >= buff->curchar + quan)
					tmark->moffset -= quan;
				else {
					tmark->mpage = tpage;
					tmark->moffset = noffset;
				}
			}
		foreach_pagemark(buff, tmark, curpage)
			if (tmark->moffset >= buff->curchar) {
				if (tmark->moffset >= buff->curchar + quan)
					tmark->moffset -= quan;
				else {
					tmark->mpage = tpage;
					tmark->moffset = noffset;
				}
			}
		makecur(buff, tpage, noffset);
	}
	bsetmod(buff);
}
/* @} */
