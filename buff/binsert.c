/* binsert.c - insert a character
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

/** Insert a byte into a buffer. Grow the buffer if needed.
 * @param buff The buffer to insert into.
 * @param byte The byte to insert.
 * @return 1 on success, 0 if we cannot allocate space for the byte.
 */
int binsert(struct buff *buff, Byte byte)
{
	struct mark *btmark;

	if (curplen(buff) == PGSIZE && !pagesplit(buff, HALFP))
		return 0;
	memmove(buff->curcptr + 1,
		buff->curcptr,
		curplen(buff) - buff->curchar);
	*buff->curcptr++ = byte;
	buff->curchar++;
	curplen(buff)++;
	buff->bmodf = 1;

	undo_add(buff, 1);

	foreach_global_pagemark(btmark, buff->curpage)
		if (btmark->moffset >= buff->curchar)
			++(btmark->moffset);
	foreach_pagemark(buff, btmark, buff->curpage)
		if (btmark->moffset >= buff->curchar)
			++(btmark->moffset);

	bsetmod(buff);
	return 1;
}
/* @} */
