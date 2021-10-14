/* is one mark before another
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
