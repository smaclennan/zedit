/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Copy from Point to mark to another buffer.
 * @param mark Copy from Point to mark.
 * @param to The buffer to copy to.
 * @return The number of bytes copied. Can return 0 if a temporary
 * mark cannot be created.
 */
long bcopyrgn(struct mark *mark, struct buff *to)
{
	struct mark ltmrk, *btmrk;
	int flip;
	int  srclen, dstlen;
	long copied = 0;
	struct buff *from = mark->mbuff;

	flip = bisaftermrk(from, mark);
	if (flip)
		bswappnt(from, mark);

	bmrktopnt(from, &ltmrk);

	while (bisbeforemrk(from, mark)) {
		if (from->curpage == mark->mpage)
			srclen = mark->moffset - from->curchar;
		else
			srclen = curplen(from) - from->curchar;

		dstlen = PGSIZE - curplen(to);
		if (dstlen == 0) {
			if (pagesplit(to, HALFP))
				dstlen = PGSIZE - curplen(to);
			else
				break;
		}
		if (srclen < dstlen)
			dstlen = srclen;
		/* Make a gap */
		memmove(to->curcptr + dstlen,
			to->curcptr,
			curplen(to) - to->curchar);
		/* and fill it in */
		memmove(to->curcptr, from->curcptr, dstlen);
		curplen(to) += dstlen;
		copied += dstlen;
		foreach_global_pagemark(btmrk, to->curpage)
			if (btmrk->moffset > to->curchar)
				btmrk->moffset += dstlen;
		foreach_pagemark(to, btmrk, to->curpage)
			if (btmrk->moffset > to->curchar)
				btmrk->moffset += dstlen;
		makeoffset(to, to->curchar + dstlen);
		bsetmod(to);
		to->bmodf = 1;
		bmove(from, dstlen);
	}

	bpnttomrk(from, &ltmrk);

	if (flip)
		bswappnt(from, mark);

	return copied;
}
/* @} */
