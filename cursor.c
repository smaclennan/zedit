/* Copyright (C) 1988-2018 Sean MacLennan */

#include "z.h"

/** @addtogroup zedit
 * @{
 */

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
	i = wheight() + prefline() - 2;
	while (i > 0 && bcsearch(Bbuff, NL)) {
		bmove(Bbuff, -1);
		i -= bgetcol(true, 0) / Colmax;
		bmove1(Bbuff);
		--i;
	}
	bmakecol(col);
	reframe();
}

#define ISWORD	bistoken

void Zprevious_word(void)
{
	bmoveto(Bbuff, ISWORD, BACKWARD);
	bmovepast(Bbuff, ISWORD, BACKWARD);
}

void Znext_word(void)
{
	bmovepast(Bbuff, ISWORD, FORWARD);
	bmoveto(Bbuff, ISWORD, FORWARD);
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
/* @} */
