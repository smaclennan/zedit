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

/* Mark a new comment from start to Point. */
static void newcomment(struct mark *start)
{
	struct comment *new = calloc(sizeof(struct comment), 1);
	if (!new)
		return;
	new->start = bcremrk();
	new->end   = bcremrk();

	mrktomrk(new->start, start);

	if (!Curbuff->chead)
		Curbuff->chead = new;
	else
		Curbuff->ctail->next = new;
	Curbuff->ctail = new;
}

/* Scan an entire buffer for comments. */
static void scanbuffer(void)
{
	struct mark tmark, start;
	int i;

	uncomment(Curbuff);

	for (i = 0; i < tmaxrow() - 2; ++i)
		Scrnmarks[i].modf = 1;

	bmrktopnt(&tmark);

	btostart();
	if (Curbuff->comchar) {
		while (bcsearch(Curbuff->comchar) && !bisend()) {
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
				if (bstrsearch("*/", FORWARD)) {
					bmove1();
					newcomment(&start);
				}
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
	if (Lfunc == ZYANK)
		Comstate = false;
	else if (Curbuff->bmode & CMODE)
		/* Was the last command a delete of any type? */
		if (delcmd() || Lfunc == ZDELETE_CHAR ||
		    Lfunc == ZDELETE_PREVIOUS_CHAR)
			for (start = Curbuff->chead; start; start = start->next)
				if (markch(start->end) != '/') {
					Comstate = false;
					break;
				}

	start = Curbuff->chead;
}

/* Called from innerdsp for each char displayed. */
void cprntchar(Byte ch)
{
	int style = T_NORMAL;

	if (!Comstate) {
		scanbuffer();
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
	while (buff->chead) {
		struct comment *com = buff->chead;
		buff->chead = buff->chead->next;
		unmark(com->start);
		unmark(com->end);
		free(com);
	}

	Comstate = false;
}
