/* bdelbuff.c - delete a buffer
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

/** Delete a buffer. Buffer can be NULL. */
void bdelbuff(struct buff *tbuff)
{
	if (!tbuff)
		return;

#if HUGE_FILES
	bhugecleanup(tbuff);
#endif

	while (tbuff->firstp)	/* delete the pages */
		freepage(tbuff, tbuff->firstp);

#ifdef HAVE_BUFFER_MARKS
	while (tbuff->marks) /* delete the marks */
		bdelmark(tbuff->marks);
#endif

	free(tbuff);	/* free the buffer proper */
}

