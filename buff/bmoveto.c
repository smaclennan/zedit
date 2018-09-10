/* bmoveto.c - move to a thingy
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

/** Go forward or back to a thingy. See bistoken() for an example
 * thingy predicate function.
 * @param buff The buffer to move in.
 * @param pred The thingy function.
 * @param forward 1 to move forward, 0 to move backward.
 */
void bmoveto(struct buff *buff, int (*pred)(int), int forward)
{
	if (forward)
		while (!bisend(buff) && !pred(*buff->curcptr))
			bmove1(buff);
	else {
		bmove(buff, -1);
		while (!bisstart(buff) && !pred(*buff->curcptr))
			bmove(buff, -1);
		if (!bisstart(buff))
			bmove1(buff);
	}
}
/* @} */
