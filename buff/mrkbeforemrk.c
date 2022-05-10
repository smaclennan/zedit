/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** True if mark1 precedes mark2.
 * @param mark1 First mark.
 * @param mark2 Second mark.
 * @return 1 if mark1 before mark2.
 */
int mrkbeforemrk(struct mark *mark1, struct mark *mark2)
{
	struct page *tpage;

	if (!mark1->mpage || !mark2->mpage || mark1->mbuff != mark2->mbuff)
		return 0;        /* Marks not in same buffer */

	if (mark1->mpage == mark2->mpage)
		return mark1->moffset < mark2->moffset;
	if (mark1->mpage->prevp == mark2->mpage)
		return 0;

	for (tpage = mark1->mpage->nextp;
		 tpage && tpage != mark2->mpage;
		 tpage = tpage->nextp)
		;
	return tpage != NULL;
}
/* @} */
