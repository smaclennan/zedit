/* term.c - generic terminal routines
 * Copyright (C) 1988-2017 Sean MacLennan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "tinit.h"

/** @addtogroup term
 * @{
*/

extern int no_tsize;

/** Get the current terminal size. If it can't get the size it returns
 * 80x30.
 * @param[out] rows The number of rows.
 * @param[out] cols The number of columns.
 */
void tsize(int *rows, int *cols)
{
	*cols = 80;
	*rows = 30;

	if (no_tsize) {
		return;
	}

	char buf[12], *p;
	int n, r = 0, c = 0;

	/* Save cursor position */
	twrite("\033[s", 3);
	/* Send the cursor to the extreme right corner */
	twrite("\033[999;999H", 10);
	/* Ask where we really ended up */
	twrite("\033[6n", 4);
	tflush();
	n = read(0, buf, sizeof(buf) - 1);
	/* Restore cursor */
	twrite("\033[u", 3);
	tflush();

	if (n > 0) {
		buf[n] = '\0';
		if (buf[0] == '\033' && buf[1] == '[') {
			r = strtol(buf + 2, &p, 10);
			if (*p++ == ';')
				c = strtol(p, NULL, 10);
		}
		if (r > 0 && c > 0) {
			*rows = r;
			*cols = c;
		}
	}
}
/* @} */
