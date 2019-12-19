/* bdeltomrk.c - delete from point to mark
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
