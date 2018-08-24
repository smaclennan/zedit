#include "buff.h"

/** Move the point relative to its current position. The move can be
 * forward (positive dist) or backwards (negative dist). Returns true
 * if the full move was possible.
 */
bool bmove(struct buff *buff, int dist)
{
	while (dist) {
		struct page *curpage = buff->curpage;

		dist += buff->curchar;
		if (dist >= 0 && (unsigned)dist < curplen(buff)) {
			/* within current page makeoffset dist */
			makeoffset(buff, dist);
			return true;
		}
		if (dist < 0) { /* goto previous page */
			if (curpage == buff->firstp) {
				/* past start of buffer */
				makeoffset(buff, 0);
				return false;
			}
			makecur(buff, curpage->prevp, curpage->prevp->plen);
		} else {	/* goto next page */
			if (lastp(curpage)) {
				/* past end of buffer */
				makeoffset(buff, curplen(buff));
				return false;
			}
			dist -= curplen(buff); /* must use this curplen */
			makecur(buff, curpage->nextp, 0);
		}
	}
	return true;
}

