#include "buff.h"

/** Move the point relative to its current position. The move can be
 * forward (positive dist) or backwards (negative dist). Returns 1
 * if the full move was possible.
 */
int bmove(struct buff *buff, int dist)
{
	while (dist) {
		struct page *curpage = buff->curpage;

		dist += buff->curchar;
		if (dist >= 0 && (unsigned)dist < curplen(buff)) {
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

