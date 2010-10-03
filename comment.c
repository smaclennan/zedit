/* comment.c - Zedit commentbold functions
 * Copyright (C) 1988-2010 Sean MacLennan
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

static struct comment *CPPhead, *CPPtail;	/* list of CPP statements */
static struct comment *COMhead, *COMtail;	/* list of Comments */

static struct comment *new_comment(struct mark *start, struct mark *end,
				   int type)
{
	struct comment *new = calloc(sizeof(struct comment), 1);
	if (!new)
		return NULL;
	new->start = bcremrk();
	new->end   = bcremrk();

	new->type  = type;
	if (start)
		mrktomrk(new->start, start);
	if (end)
		mrktomrk(new->end,   end);

	return new;
}

/* Mark a new comment from start to end.
 * If start or end are NULL, use Point.
 */
static void newcomment(struct mark *start, struct mark *end)
{
	struct comment *new = new_comment(start, end, T_COMMENT);
	if (!new)
		return;

	if (!COMhead)
		COMhead = new;
	else
		COMtail->next = new;
	COMtail = new;
}

static void newcpp(struct mark *start, struct mark *end, int type)
{
	struct comment *new = new_comment(start, end, type);
	if (!new)
		return;

	if (!CPPhead)
		CPPhead = new;
	else
		CPPtail->next = new;
	CPPtail = new;
}

/* Merge the COMlist and the CPPlist into the buffer->comments */
static void mergecomments(void)
{
	struct buff *buff = Curbuff;

	if (!CPPhead) {
		buff->comments = COMhead;
		return;
	}
	if (!COMhead) {
		buff->comments = CPPhead;
		return;
	}

	while (COMhead && CPPhead)
		if (mrkaftermrk(CPPhead->start, COMhead->start)) {
			if (!buff->comments)
				buff->comments = COMhead;
			else
				buff->ctail->next = COMhead;
			buff->ctail = COMhead;
			COMhead = COMhead->next;
		} else {
			if (!buff->comments)
				buff->comments = CPPhead;
			else
				buff->ctail->next = CPPhead;
			buff->ctail = CPPhead;
			CPPhead = CPPhead->next;
		}

	while (COMhead) {
		buff->ctail->next = COMhead;
		buff->ctail = COMhead;
		COMhead = COMhead->next;
	}

	while (CPPhead) {
		buff->ctail->next = CPPhead;
		buff->ctail = CPPhead;
		CPPhead = CPPhead->next;
	}
}

#if 1
/* Highlight a CPP. */
static void cppstatement(void)
{
	struct mark start;
	int type;

	bmrktopnt(&start);

	/* Check for: if/elif/else/endif */
	/* WARNING: getbword is too deadly to use here */
	do {
		bmove1();			/* skip '#' and whitespace */
		if (bisend())
			return;
	} while (biswhite());

	if (Buff() == 'i' && bmove1() && Buff() == 'f')
		type = T_CPPIF;
	else if (Buff() == 'e' && bmove1() && (Buff() == 'l' || Buff() == 'n'))
		type = T_CPPIF;
	else
		type = T_CPP;

again:
	while (Buff() != '\n' && !bisend()) {
		if (Buff() == '/') {
			bmove1();
			if (Buff() == '*') {
				/* found comment start */
				bmove(-2);
				newcpp(&start, NULL, type);

				/* find comment end */
				if (bstrsearch("*/", FORWARD)) {
					bmrktopnt(&start);
					if (bcsearch('\n')) {
						bmove(-2);
						if (Buff() == '\\')
							goto again;
					}
				}
				return;
			} else if (Buff() == '/') {
				/* found c++ comment start */
				bmove(-2);
				newcpp(&start, NULL, type);

				/* find comment end */
				bcsearch('\n');
				return;
			}
		} else
			bmove1();
	}

	if (ISNL(Buff())) {
		/* check for continuation line */
		bmove(-1);
		if (Buff() == '\\') {
			/* continuation line */
			bmove(2);
			goto again;
		}
		bmove1();

	}
	newcpp(&start, NULL, type);
}
#endif

/* Remove all comments from buffer and mark unscanned */
static void uncomment(struct buff *buff)
{
	struct comment *com, *next;
	int i;

	for (com = buff->comments; com; com = next) {
		unmark(com->start);
		unmark(com->end);
		next = com->next;
		free(com);
	}
	buff->comstate = 0;
	buff->comments = NULL;

	for (i = 0; i < tmaxrow() - 2; ++i)
		Scrnmarks[i].modf = 1;
}

/* Scan an entire buffer for comments. */
static void scanbuffer(void)
{
	struct mark tmark;
	struct mark start;
	char comchar = (Curbuff->bmode & ASMMODE) ? Curbuff->comchar : 0;

	COMhead = COMtail = CPPhead = CPPtail = NULL;

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
	if (comchar)
		while (bcsearch(comchar) && !bisend()) {
			/* mark to end of line as comment */
			bmove(-1);
			bmrktopnt(&start);
			bcsearch('\n');
			bmove(-1);
			newcomment(&start, NULL);
		}
	else
		/* Look for both C and C++ comments. */
		while (bcsearch('/') && !bisend()) {
			if (Buff() == '*') {
				bmove(-1);
				bmrktopnt(&start);
				if (bstrsearch("*/", FORWARD)) {
					bmove1();
					newcomment(&start, NULL);
				}
			} else if (Buff() == '/') {
				bmove(-1);
				bmrktopnt(&start);
				toendline();
				newcomment(&start, NULL);
			}
		}

	/* find CPP statements */
	btostart();
	do {
		if (Buff() == '#')
			cppstatement();
		else if (comchar) {
			/* for assembler */
			movepast(biswhite, 1);
			if (Buff() == '.')
				cppstatement();
		}
	} while (bcsearch('\n') && !bisend());

	mergecomments();
	bpnttomrk(&tmark);
}

/* The following are called by the innerdsp routine. */
static struct comment *start;

/* Called from innerdsp before display loop */
void resetcomments(void)
{
	if (delcmdall()) {
		for (start = Curbuff->comments; start; start = start->next)
			if (markch(start->end) != '/') {
				uncomment(Curbuff);
				break;
			}
	} else if (Lfunc == ZYANK)
		uncomment(Curbuff);
	start = Curbuff->comments;
}

/* Called from innerdsp before each char displayed. */
void checkcomment(void)
{
	if (!Curbuff->comstate) {
		if (!(Curbuff->bmode & (CMODE | ASMMODE)))
			return;
		scanbuffer();
		Curbuff->comstate = 1;
		start = Curbuff->comments;
	}
	for ( ; start; start = start->next)
		if (bisbeforemrk(start->start))
			break;
		else if (bisbeforemrk(start->end) || bisatmrk(start->end)) {
			tstyle(start->type);
			return;
		}

	tstyle(T_NORMAL);
}

/* Called from Zcinsert when end comment entered */
void addcomment(void)
{
	scanbuffer();
}

/* Called from Zcinsert when '#' entered at start of line */
void addcpp(void)
{
	scanbuffer();
}

/* Called from Zredisplay */
void recomment(void)
{
	struct buff *buff;

	for (buff = Bufflist; buff; buff = buff->next)
		uncomment(buff);
}
#endif
