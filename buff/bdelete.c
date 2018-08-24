#include "buff.h"

/** Delete quantity characters from the buffer at the current point. */
void bdelete(struct buff *buff, unsigned quantity)
{
	unsigned quan, noffset;
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
		buff->bmodf = true;
		if (curplen(buff) == 0 && (curpage->nextp || curpage->prevp)) {
			/* We deleted entire page. */
			tpage = curpage->nextp;
			noffset = 0;
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
		} else {
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
		}
		makecur(buff, tpage, noffset);
	}
	bsetmod(buff);
}
