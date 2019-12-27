/* bstrline.c - get the current line at the point
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

/** Store the current Point line. Leaves Point at the start of the
 * next line.
 * @param buff The buffer the Point is in.
 * @param[out] str The string to store the line to.
 * @param len The length of the string.
 * @return The actual length of the line. Can be > string length.
 */
int bstrline(struct buff *buff, char *str, int len)
{
	int count = 0;

	if (len == 0)
		return 0;

	tobegline(buff);

	while (len > 1 && !bisend(buff) && *buff->curcptr != '\n') {
		*str++ = *buff->curcptr;
		--len;
		++count;
		bmove1(buff);
	}
	*str = 0;

	while (!bisend(buff) && *buff->curcptr != '\n') {
		++count;
		bmove1(buff);
	}
	if (*buff->curcptr == '\n')
		bmove1(buff);

	return count;
}
/* @} */
