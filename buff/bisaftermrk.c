/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Is the buffer Point after the mark?
 * @param buff The buffer the Point is in.
 * @param mark The mark to check against.
 * @return 1 if Point is after the mark.
 */
int bisaftermrk(struct buff *buff, struct mark *mark)
{
	struct page *tp;

	if (!mark->mpage || mark->mbuff != buff)
		return 0;
	if (mark->mpage == buff->curpage)
		return buff->curchar > mark->moffset;
	if (mark->mpage == buff->curpage->nextp)
		return 0;
	for (tp = buff->curpage->prevp;
		 tp && tp != mark->mpage;
		 tp = tp->prevp)
		;
	return tp != NULL;
}
/* @} */
