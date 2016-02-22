/* cursor.c - Zedit cursor commands
 * Copyright (C) 1988-2013 Sean MacLennan
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

#include "z.h"

void Zbeginning_of_line(void)
{
	bmove(Bbuff, -1);
	tobegline(Bbuff);
}

void Zend_of_line(void)
{
	bmove1(Bbuff);
	toendline(Bbuff);
}

/* Support routine to calc column to force to for line up and down */
static int forcecol(void)
{
	static int fcol;

	switch (Lfunc) {
	case ZPREVIOUS_LINE:
	case ZNEXT_LINE:
	case ZPREVIOUS_PAGE:
	case ZNEXT_PAGE:
		break;

	default:
		if (Buff() == NL)
			fcol = COLMAX + 1;
		else
			fcol = bgetcol(true, 0);
	}

	return fcol;
}

void Zprevious_line(void)
{
	int col = forcecol();
	while (Arg-- > 0)
		bcrsearch(Bbuff, NL);
	bmakecol(col);
}

void Znext_line(void)
{
	int col = forcecol();
	while (Arg-- > 0)
		bcsearch(Bbuff, NL);
	bmakecol(col);
}

void Zprevious_char(void)
{
	bmove(Bbuff, -Arg);
	Arg = 0;
}

void Znext_char(void)
{
	bmove(Bbuff, Arg);
	Arg = 0;
}

/* Return current screen col of point. */
int bgetcol(bool flag, int col)
{
	struct mark pmark;

	bmrktopnt(Bbuff, &pmark);
	if (bcrsearch(Bbuff, '\n'))
		bmove1(Bbuff);
	while (!bisatmrk(Bbuff, &pmark) && !bisend(Bbuff)) {
		col += chwidth(Buff(), col, flag);
		bmove1(Bbuff);
	}
	return col;
}

/* Try to put Point in a specific column.
 * Returns actual Point column.
 */
int bmakecol(int col)
{
	int tcol = 0;

	if (bcrsearch(Bbuff, '\n'))
		bmove1(Bbuff);
	while (tcol < col && Buff() != '\n' && !bisend(Bbuff)) {
		tcol += chwidth(Buff(), tcol, true);
		bmove1(Bbuff);
	}
	return tcol;
}

void Zprevious_page(void)
{
	int i, col = forcecol();

	bpnttomrk(Bbuff, Sstart);
	for (i = wheight() - prefline() - 2; i > 0 && bcrsearch(Bbuff, NL); --i)
		i -= bgetcol(true, 0) / Colmax;
	bmakecol(col);
	reframe();
}

void Znext_page(void)
{
	int i, col = forcecol();

	bpnttomrk(Bbuff, Sstart);
	for (i = wheight() + prefline() - 2; i > 0 && bcsearch(Bbuff, NL); --i) {
		bmove(Bbuff, -1);
		i -= bgetcol(true, 0) / Colmax;
		bmove1(Bbuff);
	}
	bmakecol(col);
	reframe();
}

#define ISWORD	bistoken

void Zprevious_word(void)
{
	moveto(ISWORD, BACKWARD);
	movepast(ISWORD, BACKWARD);
}

void Znext_word(void)
{
	movepast(ISWORD, FORWARD);
	moveto(ISWORD, FORWARD);
}

void Zbeginning_of_buffer(void)
{
	btostart(Bbuff);
}

void Zend_of_buffer(void)
{
	btoend(Bbuff);
}

void Zswap_mark(void)
{
	struct mark tmark;

	Arg = 0;
	NEED_UMARK;

	mrktomrk(&tmark, UMARK);
	Zset_mark();
	bpnttomrk(Bbuff, &tmark);
}

void Zopen_line(void)
{
	binsert(Bbuff, NL);
	bmove(Bbuff, -1);
}

static long getnum(const char *prompt)
{
	char str[10];
	long num = -1;

	/* get the line number */
	*str = '\0';
	if (Argp) {
		num = Arg;
		Arg = 0;
	} else if (getarg(prompt, str, 9) == 0)
		num = strtol(str, NULL, 0);
	return num;
}

void Zgoto_line(void)
{
	long line;

	if (Argp)
		line = Arg;
	else {
		line = getnum("Line: ");
		if (line == -1)
			return;
	}

	btostart(Bbuff);
	while (--line > 0)
		bcsearch(Bbuff, NL);
}

void Zout_to(void)
{
	int tcol = 0, col = (int)getnum("Column: ");
	if (col == -1)
		return;
	--col;

	if (bcrsearch(Bbuff, '\n'))
		bmove1(Bbuff);
	while (tcol < col && Buff() != '\n' && !bisend(Bbuff)) {
		tcol += chwidth(Buff(), tcol, false);
		bmove1(Bbuff);
	}

	if (tcol < col) {
		int wid = chwidth('\t', tcol, true);
		if (tcol + wid < col)
			tcol -= Tabsize - wid;
		tindent(col - tcol);
	}
}

void Zredisplay(void)
{
	struct zbuff *buff;

	wsize();
	redisplay();

	foreachbuff(buff)
		uncomment(buff);
}

static void do_scroll(bool (*search)(struct buff *buff, Byte what))
{
	struct mark pmark;

	bmrktopnt(Bbuff, &pmark);
	bpnttomrk(Bbuff, Sstart);
	while (Arg-- > 0 && search(Bbuff, NL))
		;
	tobegline(Bbuff);
	bmrktopnt(Bbuff, Sstart);
	bmove(Bbuff, -1);
	bmrktopnt(Bbuff, Psstart);
	Sendp = false;

	if (mrkaftermrk(Sstart, &pmark))
		bmove1(Bbuff);
	else
		bpnttomrk(Bbuff, &pmark);
}

void Zscroll_up(void)
{
	do_scroll(bcrsearch);
}

void Zscroll_down(void)
{
	do_scroll(bcsearch);
}
