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

/** Delete one byte from the buffer at the current point.
 * @param buff The buffer to delete from.
 */
void bdelete1(struct buff *buff)
{
	/* This can only happen on an empty buffer */
	if (curplen(buff) == 0)
		return;

	/* End of last (or only) page. Nothing to delete. */
	if (curplen(buff) == buff->curchar)
		return;

	undo_del(buff, 1);

	curplen(buff)--;

	struct page *curpage = buff->curpage;
	if (curplen(buff) == 0) {
		if (curpage->nextp || curpage->prevp)
			delete_entire_page(buff, curpage);
		else
			makeoffset(buff, 0);
	} else {
		memmove(buff->curcptr,
			buff->curcptr + 1,
			curplen(buff) - buff->curchar);
	}

	buff->bmodf = 1;

	struct mark *tmark;
	foreach_global_pagemark(tmark, curpage)
		if (tmark->moffset >= buff->curchar)
			if (tmark->moffset >= buff->curchar)
				tmark->moffset--;
	foreach_pagemark(buff, tmark, curpage)
		if (tmark->moffset >= buff->curchar)
			tmark->moffset--;

	bsetmod(buff);
}
/* @} */
