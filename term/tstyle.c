/* Copyright (C) 1988-2017 Sean MacLennan */

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
	cur_style = style;

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
	char str[8], *p = str;
	int n = 4;

	*p++ = '\033'; *p++ = '[';
	if (style >= T_FG) {
		if (style >= T_BG) {
			*p++ = '4';
			style -= 40;
		} else {
			*p++ = '3';
			style -= 30;
		}
		++n;
	}
	*p++ = style + '0';
	*p++ = 'm';
	twrite(str, n);
#endif

	tflush();
}
/* @} */
