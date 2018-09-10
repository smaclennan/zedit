/* bpeek.c - peek the previous byte
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

/** Peek the byte before the point. Does not move the point. Returns
 * LF at start of buffer. Much more efficient than moving the Point.
 * @param buff The buffer to peek.
 * @return The byte before the point. Returns LF at start of buffer.
 */
Byte bpeek(struct buff *buff)
{
	Byte ch;

	if (buff->curchar > 0)
		return *(buff->curcptr - 1);
	if (bisstart(buff))
		/* Pretend we are at the start of a line.
		 * Needed for delete-to-eol and step in reg.c.
		 */
		return '\n';
	bmove(buff, -1);
	ch = *buff->curcptr;
	bmove1(buff);
	return ch;
}
