#include "buff.h"

/** Copy from Point to mark to buffer 'to'. Returns bytes copied. */
long bcopyrgn(struct mark *tmark, struct buff *to)
{
	struct mark *ltmrk, *btmrk;
	bool flip;
	int  srclen, dstlen;
	long copied = 0;
	struct buff *from = tmark->mbuff;

	flip = bisaftermrk(from, tmark);
	if (flip)
		bswappnt(from, tmark);

	ltmrk = bcremark(from);
	if (!ltmrk)
		return 0;

	while (bisbeforemrk(from, tmark)) {
		if (from->curpage == tmark->mpage)
			srclen = tmark->moffset - from->curchar;
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
		to->bmodf = true;
		bmove(from, dstlen);
	}

	bpnttomrk(from, ltmrk);
	bdelmark(ltmrk);

	if (flip)
		bswappnt(from, tmark);

	return copied;
}
