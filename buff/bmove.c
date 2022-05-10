/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Move the point relative to its current position. The move can be
 * forward (positive dist) or backwards (negative dist). See bmove1()
 * for a more efficient move of dist 1.
 * @param buff The buffer to move the Point in.
 * @param dist The amount to move the Point (can be negative).
 * @return 1 if the full move was possible, 0 if not.
 */
int bmove(struct buff *buff, int dist)
{
	while (dist) {
		struct page *curpage = buff->curpage;

		dist += buff->curchar;
		if (dist >= 0 && (unsigned int)dist < curplen(buff)) {
			/* within current page makeoffset dist */
			makeoffset(buff, dist);
			return 1;
		}
		if (dist < 0) { /* goto previous page */
			if (curpage == buff->firstp) {
				/* past start of buffer */
				makeoffset(buff, 0);
				return 0;
			}
			makecur(buff, curpage->prevp, curpage->prevp->plen);
		} else {	/* goto next page */
			if (lastp(curpage)) {
				/* past end of buffer */
				makeoffset(buff, curplen(buff));
				return 0;
			}
			dist -= curplen(buff); /* must use this curplen */
			makecur(buff, curpage->nextp, 0);
		}
	}
	return 1;
}
/* @} */
