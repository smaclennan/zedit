/* comment.c - Zedit commentbold functions
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

static bool Comstate;

struct comment {
	struct mark *start, *end;
	struct comment *next;
};

/* Mark a new comment from start to Point. */
static void newcomment(struct mark *start)
{
	struct comment *com;
	com = (struct comment *)calloc(sizeof(struct comment), 1);
	if (!com)
		return;
	com->start = zcreatemrk();
	com->end   = zcreatemrk();

	mrktomrk(com->start, start);

	if (!Curbuff->chead)
		Curbuff->chead = com;
	else
		((struct comment *)Curbuff->ctail)->next = com;
	Curbuff->ctail = com;
}

/* Scan an entire buffer for comments. */
static void scanbuffer(struct buff *buff)
{
	struct mark tmark, start;
	int i;
	Byte comchar = zapp(Curbuff) ? zapp(Curbuff)->comchar : 0;

	uncomment(buff);

	for (i = 0; i < Rowmax - 2; ++i)
		Scrnmarks[i].modf = true;

	bswitchto(buff);
	bmrktopnt(&tmark);

	btostart();
	if (comchar) {
		while (bcsearch(comchar) && !bisend()) {
			if (bmove(-2) == 0) { /* skip to char *before* # */
				/* # is first character in buffer */
			} else if (isspace(*Curcptr))
				bmove1();
			else {
				bmove(2);
				continue;
			}

			/* mark to end of line as comment */
			bmrktopnt(&start);
			bcsearch('\n');
			bmove(-1);
			newcomment(&start);
		}
	} else {
		/* Look for both C and C++ comments. */
		while (bcsearch('/') && !bisend()) {
			if (Buff() == '*') {
				bmove(-1);
				bmrktopnt(&start);
				if (bstrsearch("*/", FORWARD))
					newcomment(&start);
			} else if (Buff() == '/') {
				bmove(-1);
				bmrktopnt(&start);
				toendline();
				newcomment(&start);
			}
		}
	}

	bpnttomrk(&tmark);
	Comstate = true;
}

/* The following are called by the innerdsp routine. */
static struct comment *start;

/* Called from innerdsp before display loop */
void resetcomments(void)
{
	start = Curbuff->chead;

	switch (Lfunc) {
	/* Was the last command a delete of any type? */
	case ZDELETE_TO_EOL:
	case ZDELETE_LINE:
	case ZDELETE_REGION:
	case ZDELETE_WORD:
	case ZDELETE_PREVIOUS_WORD:
	case ZCOPY_REGION:
	case ZCOPY_WORD:
	case ZAPPEND_KILL:
	case ZDELETE_CHAR:
	case ZDELETE_PREVIOUS_CHAR:

	/* Or an insert? */
	case ZYANK:
	case ZINSERT:
	case ZC_INSERT:

	/* Can be insert or delete */
	case ZUNDO:
		Comstate = false;
	}
}

/* Called from innerdsp for each char displayed. */
void cprntchar(Byte ch)
{
	int style = T_NORMAL;

	if (!Comstate) {
		struct wdo *wdo;
		struct buff *was = Curbuff;

		scanbuffer(Curbuff);
		for (wdo = Whead; wdo; wdo = wdo->next)
			if (wdo->wbuff != Curbuff)
				scanbuffer(wdo->wbuff);
		bswitchto(was);
		start = Curbuff->chead;
	}

	for (; start; start = start->next)
		if (bisbeforemrk(start->start))
			break;
		else if (bisbeforemrk(start->end) || bisatmrk(start->end)) {
			style = T_COMMENT;
			break;
		}

	tstyle(style);
	tprntchar(ch);
}

/* Remove all comments from buffer and mark unscanned */
void uncomment(struct buff *buff)
{
	if (buff)
		while (buff->chead) {
			struct comment *com = buff->chead;
			buff->chead = com->next;
			unmark(com->start);
			unmark(com->end);
			free(com);
		}

	Comstate = false;
}
