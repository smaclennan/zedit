/* mark.c - low level mark functions
 * Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#include "buff.h"

#ifdef HAVE_GLOBAL_MARKS
/** Global mark list. The buffer code keeps the marks in this list
 * up to date.
 */
struct mark *Marklist;	/* the marks list tail */
#endif

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

/** Returns true if point is between start and end. This has to walk
 * all the pages between start and end. So it is most efficient for
 * small ranges and terrible if end is before start.
 *
 * Note: point == start == end returns false: it is not between.
 */
bool bisbetweenmrks(struct buff *buff, struct mark *start, struct mark *end)
{
	struct page *tp;
	bool found = false;

	if (start->mbuff != buff || end->mbuff != buff)
		return false;

	if (buff->curpage == start->mpage)
		if (buff->curchar < start->moffset)
			return false;

	for (tp = start->mpage; tp; tp = tp->nextp)
		if (tp == buff->curpage) {
			if (tp == end->mpage)
				return buff->curchar < end->moffset;
			found = true;
		} else if (tp == end->mpage)
			return found;

	return false;
}

/** Put the current buffer point at the mark */
bool bpnttomrk(struct buff *buff, struct mark *tmark)
{
	if (tmark->mbuff != buff)
		return false;
	if (tmark->mpage)
		makecur(buff, tmark->mpage, tmark->moffset);
	return true;
}

/** Swap the point and the mark. */
bool bswappnt(struct buff *buff, struct mark *tmark)
{
	struct mark tmp;

	if (tmark->mbuff != buff)
		return false;

	bmrktopnt(buff, &tmp);
	bpnttomrk(buff, tmark);
	mrktomrk(tmark, &tmp);
	return true;
}

/** True if mark1 precedes mark2 */
bool mrkbeforemrk(struct mark *mark1, struct mark *mark2)
{
	struct page *tpage;

	if (!mark1->mpage || !mark2->mpage || mark1->mbuff != mark2->mbuff)
		return false;        /* Marks not in same buffer */

	if (mark1->mpage == mark2->mpage)
		return mark1->moffset < mark2->moffset;
	if (mark1->mpage->prevp == mark2->mpage)
		return false;

	for (tpage = mark1->mpage->nextp;
		 tpage && tpage != mark2->mpage;
		 tpage = tpage->nextp)
		;
	return tpage != NULL;
}

/** True if mark1 follows mark2 */
bool mrkaftermrk(struct mark *mark1, struct mark *mark2)
{
	struct page *tpage;

	if (!mark1->mpage || !mark2->mpage || mark1->mbuff != mark2->mbuff)
		return false;        /* marks in different buffers */

	if (mark1->mpage == mark2->mpage)
		return mark1->moffset > mark2->moffset;
	if (mark1->mpage->nextp == mark2->mpage)
		return false;

	for (tpage = mark1->mpage->prevp;
		 tpage && tpage != mark2->mpage;
		 tpage = tpage->prevp)
		;
	return tpage != NULL;
}
