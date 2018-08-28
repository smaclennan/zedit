#include "buff.h"

/** Return 1 if the point precedes the mark. */
int bisbeforemrk(struct buff *buff, struct mark *tmark)
{
	struct page *tp;

	if (!tmark->mpage || tmark->mbuff != buff)
		return 0;
	if (tmark->mpage == buff->curpage)
		return buff->curchar < tmark->moffset;
	if (tmark->mpage == buff->curpage->prevp)
		return 0;
	for (tp = buff->curpage->nextp;
		 tp && tp != tmark->mpage;
		 tp = tp->nextp)
		;
	return tp != NULL;
}
