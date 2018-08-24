#include "buff.h"

/** Returns true if point is after the mark. */
bool bisaftermrk(struct buff *buff, struct mark *tmark)
{
	struct page *tp;

	if (!tmark->mpage || tmark->mbuff != buff)
		return false;
	if (tmark->mpage == buff->curpage)
		return buff->curchar > tmark->moffset;
	if (tmark->mpage == buff->curpage->nextp)
		return false;
	for (tp = buff->curpage->prevp;
		 tp && tp != tmark->mpage;
		 tp = tp->prevp)
		;
	return tp != NULL;
}
