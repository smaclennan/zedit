#include "buff.h"

/** Returns 1 if point is after the mark. */
int bisaftermrk(struct buff *buff, struct mark *tmark)
{
	struct page *tp;

	if (!tmark->mpage || tmark->mbuff != buff)
		return 0;
	if (tmark->mpage == buff->curpage)
		return buff->curchar > tmark->moffset;
	if (tmark->mpage == buff->curpage->nextp)
		return 0;
	for (tp = buff->curpage->prevp;
		 tp && tp != tmark->mpage;
		 tp = tp->prevp)
		;
	return tp != NULL;
}
