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

#if COMMENTBOLD

static struct comment *COMhead, *COMtail;	/* list of Comments */

static struct comment *new_comment(struct mark *start)
{
	struct comment *new = calloc(sizeof(struct comment), 1);
	if (!new)
		return NULL;
	new->start = bcremrk();
	new->end   = bcremrk();

	if (start)
		mrktomrk(new->start, start);

	return new;
}

/* Mark a new comment from start to Point. */
static void newcomment(struct mark *start)
{
	struct comment *new = new_comment(start);
	if (!new)
		return;

	if (!COMhead)
		COMhead = new;
	else
		COMtail->next = new;
	COMtail = new;
}

/* Remove all comments from buffer and mark unscanned */
void uncomment(struct buff *buff, int need_update)
{
	while (buff->comments) {
		struct comment *com = buff->comments;
		buff->comments = buff->comments->next;
		unmark(com->start);
		unmark(com->end);
		free(com);
	}
	buff->comstate = 0;

	if (need_update) {
		int i;

		for (i = 0; i < tmaxrow() - 2; ++i)
			Scrnmarks[i].modf = 1;
	}
}

/* Scan an entire buffer for comments. */
static void scanbuffer(void)
{
	struct mark tmark;
	struct mark start;

	COMhead = COMtail = NULL;

	/* free existings comments */
	while (Curbuff->comments) {
		struct comment *com = Curbuff->comments;
		Curbuff->comments = Curbuff->comments->next;
		unmark(com->start);
		unmark(com->end);
		free(com);
	}

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
		Curbuff->comments = COMhead;
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

		Curbuff->comments = COMhead;
	}

	bpnttomrk(&tmark);
}

/* The following are called by the innerdsp routine. */
static struct comment *start;

/* Was the last command a delete of any type? */
static bool delcmdall(void)
{
	return delcmd() || Lfunc == ZDELETE_CHAR ||
		Lfunc == ZDELETE_PREVIOUS_CHAR;
}


/* Called from innerdsp before display loop */
void resetcomments(void)
{
	if (delcmdall()) {
		for (start = Curbuff->comments; start; start = start->next)
			if (markch(start->end) != '/') {
				uncomment(Curbuff, true);
				break;
			}
	} else if (Lfunc == ZYANK)
		uncomment(Curbuff, true);
	start = Curbuff->comments;
}

static inline void checkcomment(void)
{
}

/* Called from innerdsp for each char displayed. */
void cprntchar(Byte ch)
{
	int style = T_NORMAL;

	if (!Curbuff->comstate) {
		scanbuffer();
		Curbuff->comstate = 1;
		start = Curbuff->comments;
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

/* Called from Zcinsert when end comment entered */
void addcomment(void)
{
	scanbuffer();
}

/* Called from Zredisplay */
void recomment(void)
{
	struct buff *buff;

	for (buff = Bufflist; buff; buff = buff->next)
		uncomment(buff, true);
}
#endif
