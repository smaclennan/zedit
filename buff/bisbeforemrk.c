#include "buff.h"

/** Return true if the point precedes the mark. */
bool bisbeforemrk(struct buff *buff, struct mark *tmark)
{
	struct page *tp;

	if (!tmark->mpage || tmark->mbuff != buff)
		return false;
	if (tmark->mpage == buff->curpage)
		return buff->curchar < tmark->moffset;
	if (tmark->mpage == buff->curpage->prevp)
		return false;
	for (tp = buff->curpage->nextp;
		 tp && tp != tmark->mpage;
		 tp = tp->nextp)
		;
	return tp != NULL;
}
