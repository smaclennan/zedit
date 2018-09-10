/* bswappnt.c - swap point and mark
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

/** Swap the Point and the mark.
 * @param buff The buffer the point is in.
 * @param mark The mark to swap with.
 * @return 1 on success, 0 if mark not in buffer.
 */
int bswappnt(struct buff *buff, struct mark *mark)
{
	struct mark tmp;

	if (mark->mbuff != buff)
		return 0;

	bmrktopnt(buff, &tmp);
	bpnttomrk(buff, mark);
	mrktomrk(mark, &tmp);
	return 1;
}
