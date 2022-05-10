/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Delete from the Point to the mark in the mark buffer.
 * @param mark The mark to delete to.
 * @return The number of bytes deleted.
 */
long bdeltomrk(struct mark *mark)
{
	long amount, deleted = 0;
	struct buff *buff = mark->mbuff;

	if (bisaftermrk(buff, mark))
		bswappnt(buff, mark);
	while (bisbeforemrk(buff, mark)) {
		if (buff->curpage == mark->mpage)
			amount = mark->moffset - buff->curchar;
		else
			amount = curplen(buff) - buff->curchar;
		bdelete(buff, amount);
		deleted += amount;
	}

	return deleted;
}
/* @} */
