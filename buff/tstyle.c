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

/** Allow different styles to be applied to terminal output in a generic way.
 * The three generically supported styles are T_NORMAL, T_REVERSE, and
 * T_BOLD. Mainly useful for Zedit.
 * @param style The T_xxx style (see tinit.h).
 */

void tstyle(int style)
{
	static int cur_style = -1;

	if (style == cur_style)
		return;

#ifdef TERMCAP
	switch (style) {
	case T_NORMAL:
		TPUTS(cm[3]);
		break;
	case T_REVERSE:
		TPUTS(cm[4]);
		break;
	case T_BELL:
		TPUTS(cm[5]);
		break;
	case T_BOLD:
		TPUTS(cm[6]);
		break;
	default:
		return;
	}
#elif defined(TERMINFO)
	switch (style) {
	case T_NORMAL:
		TPUTS(exit_attribute_mode);
		break;
	case T_REVERSE:
		TPUTS(enter_reverse_mode);
		break;
	case T_BOLD:
		TPUTS(enter_bold_mode);
		break;
	case T_BELL:
		TPUTS(tparm(set_a_background, COLOR_RED));
		break;
	}
#else
	char str[32], *p = str;

	*p++ = '\033'; *p++ = '[';
	p = _uint2str(style, p);
	*p++ = 'm';
	twrite(str, p - str);
#endif

	cur_style = style;
	tflush();
}
/* @} */
