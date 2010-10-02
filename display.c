/* display.c - Zedit main display update
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
#include "assert.h"

static void Pawdisplay(struct mark *, struct mark *);

struct mark *Sstart, *Psstart;	/* Screen start and 'prestart' */
struct mark *Send;		/* Screen end */
Boolean Sendp;			/* Screen end set */
struct mark Scrnmarks[ROWMAX + 1];	/* Screen marks - one per line */
int Tlrow;			/* Last row displayed */

static int NESTED;		/* Refresh can go recursive... */
Byte tline[COLMAX + 1];

/* Mark screen invalid */
void Redisplay(void)
{
	int i;

	Tclrwind();
	for (i = 0; i < Tmaxrow() - 2; ++i)
		Scrnmarks[i].modf = TRUE;
	Tlrow = -1;
}

/* Do the actual display update from the buffer */
void Refresh(void)
{
	int pntrow, col, bcol;
	struct mark *pmark;
	struct wdo *wdo;
	int tsave;
	static struct mark *was;	/* last location of user mark */

	if (was == NULL)
		was = Bcremrk();
	pmark = Bcremrk();
	if (InPaw) {
		Pawdisplay(pmark, was);
		return;
	}
	ASSERT(++NESTED < 10);

	Setmodes(Curbuff);	/* SAM make sure OK */

	if (!Mrkatmrk(was, Curbuff->mark)) {
		/* the user mark has moved! */
		Vsetmrk(was);
		Vsetmrk(Curbuff->mark);
		Tlrow = -1;
		Mrktomrk(was, Curbuff->mark);
	}

	if (Bisbeforemrk(Sstart) || (Sendp && !Bisbeforemrk(Send)) ||
	   Sstart->mbuff != Curbuff)
		/* The cursor has moved before/after the screen marks */
		Reframe();
	Bpnttomrk(Sstart);
	if (Bisatmrk(Psstart) && !Bisstart()) {
		/* Deleted first char in window that is not at buffer start */
		Bpnttomrk(pmark);
		Reframe();
		Bpnttomrk(Sstart);
	}
	pntrow = Innerdsp(Curwdo->first, Curwdo->last, pmark);
	if (Bisbeforemrk(pmark) && !Tkbrdy()) {
		Bpnttomrk(pmark);
		Unmark(pmark);
		Reframe();
		Refresh();
		--NESTED;
		return;
	}

	/* update the other windows except Curwdo */
	tsave = Tabsize;
	for (wdo = Whead; wdo; wdo = wdo->next)
		if (wdo != Curwdo) {
			struct mark *point;
			Bswitchto(wdo->wbuff);
			Settabsize(Curbuff->bmode);
			point = Bcremrk();
			Bpnttomrk(wdo->wstart);
			Innerdsp(wdo->first, wdo->last, NULL);
			Modeflags(wdo);
			Bpnttomrk(point);
			Unmark(point);
			Bswitchto(Curwdo->wbuff);
		}
	Tabsize = tsave;

	Bpnttomrk(pmark);
	Unmark(pmark);
	bcol = Bgetcol(TRUE, 0);
	/* position the cursor */
	col = bcol % (Tmaxcol() - 1 - Tstart);
	/* special case for NL or Bisend at column 80 */
	if (col == 0 && bcol && (ISNL(Buff()) || Bisend()))
		col = Tmaxcol() - 1 - Tstart;
	else if (!Bisend() && (col + Width(Buff(), col, FALSE) >= Tmaxcol()))
		col = 0;
	Tgoto(pntrow, col + Tstart);

#ifndef XWINDOWS
	/*
	 * If we display the cursor on the mark, they both disappear.
	 * This code checks for this case: if true it removes the mark
	 * and invalidates its position so it will be updated when the
	 * cursor moves on...
	 */
	if (Bisatmrk(Curbuff->mark)) {
		Tstyle(T_NORMAL);
		Tprntchar((Bisend() || ISNL(Buff())) ? ' ' : Buff());
		Tgoto(pntrow, col + Tstart);
		was->moffset = PSIZE + 1; /* Invalidate it */
	}
#endif

	Modeflags(Curwdo);
	Setmodes(Curbuff);	/* displaying other windows can blow modes */
	Tflush();
	Tstyle(T_NORMAL);

#ifdef SCROLLBARS
	UpdateScrollbars();
#endif

	--NESTED;
}

/* Test and clear modified flag on screen mark. */
static inline Boolean Btstmrk(struct mark *tmark)
{
	Boolean temp = tmark->modf;
	tmark->modf  = FALSE;
	return temp;
}

static inline int buff_col(void)
{	/* Current column after current buffer char */
	return Tgetcol() + Twidth(Buff());
}

/*
 * Do the acutal screen update.
 * Curwdo is not valid.
 */
int Innerdsp(int from, int to, struct mark *pmark)
{
	int trow;
	 Byte *lptr;


	static int pntrow;
	int needpnt = TRUE, col;

#if COMMENTBOLD
	ResetComments();
#endif
	for (trow = from; trow < to; ++trow) {
#ifdef HSCROLL
		Bmove(Hshift);
#endif
		if (Btstmrk(&Scrnmarks[trow]) || !Bisatmrk(&Scrnmarks[trow])) {
			Bmrktopnt(&Scrnmarks[trow]); /* Do this before Tkbrdy */
			lptr = tline;
			col = Tstart;
			Tsetpoint(trow, col);
			while (!Bisend() && !ISNL(Buff()) &&
			       (col = buff_col()) < Tmaxcol()) {
				if (trow == Tlrow &&
				    Buff() == *lptr &&
				    Buff() != (Byte)'\376')
					Tgetcol() = col;
				else {
					if (Bisatmrk(Curbuff->mark))
						SetMark(TRUE);
					else {
#if COMMENTBOLD
						CheckComment();
#endif
						Tprntchar(Buff());
					}
					if (trow == Tlrow &&
					    (!ISPRINT(*lptr) ||
					     !ISPRINT(Buff())))
						Tlrow = -1;
				}
				*lptr++ = Buff();
				Bmove1();
			}
			Tcleol();
			if (Bisatmrk(Curbuff->mark) &&
				(ISNL(Buff()) || Bisstart() || Bisend()))
					SetMark(FALSE);
#ifdef HSCROLL
			if (!ISNL(Buff()))
				Bcsearch(NL);
#else
			if (col >= Tmaxcol())
				ExtendedLineMarker();
#endif
			memset(lptr, '\376', Colmax - (lptr - tline));
			Tlrow = trow;
			if (Tgetcol() < Tmaxcol()) {
				if (Bisend())
					Bshoveit();
				else if (ISNL(Buff()))
					Bmove1();
			}
		} else
			Bpnttomrk(&Scrnmarks[trow + 1]);
		if (pmark && Bisaftermrk(pmark) && needpnt) {
			pntrow = trow;
			needpnt = FALSE;
		}
	}
	Bmrktopnt(&Scrnmarks[trow]);
	if (pmark) {
		Bmrktopnt(Send);
		Sendp = TRUE;
		if (needpnt) {
			/* the user has typed past the end of the screen */
			Reframe();
			Refresh();
		}
	}

#if COMMENTBOLD
	Tstyle(T_NORMAL);
#endif

	return pntrow;
}

/* Work for centering redisplay */
void Reframe(void)
{
	int cnt;
	struct mark *pmark;

	pmark = Bcremrk();
	for (cnt = Prefline(); cnt > 0 && Bcrsearch(NL); --cnt)
			cnt -= Bgetcol(TRUE, 0) / Tmaxcol();
	if (cnt < 0)
		Bmakecol((-cnt) * Tmaxcol(), FALSE);
	else
		Tobegline();
	Bmrktopnt(Sstart);
	Bmove(-1);
	Bmrktopnt(Psstart);
	Sendp = FALSE;
	Bpnttomrk(pmark);
	Unmark(pmark);
}

/* Set one windows modified flags. */
static void Subset(int from, int to, int flag)
{
	struct mark *btmark, *ltmark;

	if (Scrnmarks[from].mbuff != Curbuff)
		return;
	for (btmark = &Scrnmarks[from], ltmark = &Scrnmarks[to];
		 btmark <= ltmark && btmark->mpage != Curpage;
		 ++btmark)
		;
	if (btmark > ltmark) {
		for (btmark = &Scrnmarks[from];
		     btmark <= ltmark &&
			     (btmark->mbuff != Curbuff || Bisaftermrk(btmark));
		     ++btmark)
			;
		if (btmark > &Scrnmarks[from]) {
			while ((--btmark)->mbuff != Curbuff)
				;
			btmark->modf = TRUE;
		}
	} else {
		while (btmark->mpage == Curpage && btmark->moffset <= Curchar &&
			   btmark <= ltmark)
			++btmark;
		if (--btmark >= &Scrnmarks[from])
			btmark->modf = TRUE;
		if (flag)
			while (btmark > &Scrnmarks[from] &&
			       btmark->mpage == Curpage &&
			       btmark->moffset == Curchar)
				(--btmark)->modf = TRUE;
	}
}

/* Insert the correct modified flags. */
void Vsetmod(int flag)
{
	struct wdo *wdo;

	for (wdo = Whead; wdo; wdo = wdo->next)
		if (wdo->wbuff == Curbuff)
			Subset(wdo->first, wdo->last, flag);
}

void Vsetmrk(struct mark *mrk)
{
	int row;

	for (row = 0; row < Tmaxrow() - 1; ++row)
		if (Mrkaftermrk(&Scrnmarks[row], mrk)) {
			if (row > 0)
				Scrnmarks[row - 1].modf = TRUE;
			return;
		}
}

void Tobegline(void)
{
	if (Bcrsearch(NL))
		Bmove1();
}

void Toendline(void)
{
	if (Bcsearch(NL))
		Bmove(-1);
}

#define SHIFT	(Colmax / 4 + 1)

static void Pawdisplay(struct mark *pmark, struct mark *was)
{
	int bcol = 0, i, nested = 0;
#ifndef XWINDOWS
	Boolean mrkmoved = !Mrkatmrk(was, Curbuff->mark);
#endif
	Prow = Rowmax - 1;
pawshift:
	Btostart(); Bmove(Pshift);
	for (i = 0, Pcol = Pawcol;
	     Pcol < Colmax - 2 && !Bisend();
	     Bmove1(), ++i) {
		if (Bisatmrk(pmark))
			bcol = Pcol;
#ifdef XWINDOWS
		if (Bisatmrk(Curbuff->mark)) {
			SetMark(TRUE);
			tline[i] = Buff();
		} else if (Bisatmrk(was)) {
			Tprntchar(Buff());
			tline[i] = Buff();
		}
#else
		if (mrkmoved && (Bisatmrk(Curbuff->mark) || Bisatmrk(was))) {
			if (Bisatmrk(Curbuff->mark))
				Tstyle(T_REVERSE);
			Tprntchar(Buff());
			Tstyle(T_NORMAL);
			tline[i] = Buff();
		}
#endif
		else if (tline[i] == Buff())
			Pcol += Width(Buff(), 0, 0);
		else {
			tline[i] = Buff();
			Tprntchar(Buff());
		}
	}
	memset(&tline[i], '\376', &tline[COLMAX] - &tline[i]);
	Tcleol();

	if (Bisend()) {
		if (Bisatmrk(Curbuff->mark)) {
			SetMark(FALSE);
			--Pcol;		/* space always 1 character! */
		} else if (Bisatmrk(pmark))
			bcol = Pcol;
	}

	if (!bcol) {
		if (Pshift) {
			/* shift right */
			Pshift -= SHIFT;
			if (Pshift < 0)
				Pshift = 0;
			if (++nested == 1)
				goto pawshift;
#if DBG
			else
				Dbg("Shift right nested too deep!\n");
#endif
		} else if (Pcol >= Colmax - 2) {
			/* shift left */
			Pshift += SHIFT;
			if (++nested == 1)
				goto pawshift;
#if DBG
			else
				Dbg("Shift left nested too deep!\n");
#endif
		}
	}

	if (bcol)
		Pcol = bcol;
	Bpnttomrk(pmark);
	Mrktomrk(was, Curbuff->mark);

#ifndef XWINDOWS
	/*
	 * If we display the cursor on the mark, they both disappear.
	 * This code checks for this case: if true it removes the mark
	 * and invalidates its position so it will be updated when the
	 * cursor moves on...
	 */
	if (Bisatmrk(Curbuff->mark)) {
		i = Pcol;
		Tprntchar(Bisend() ? ' ' : Buff());
		Pcol = i;
		was->moffset = PSIZE + 1;		/* Invalidate it */
	}
#endif

	Unmark(pmark);
	--NESTED;
	Tforce();
	Tflush();
}

void initScrnmarks(void)
{
	int cnt;

	/* Set the screen marks */
	memset((char *)Scrnmarks, 0, sizeof(Scrnmarks));
	Scrnmarks[0].next = &Scrnmarks[1];
	for (cnt = 1; cnt < ROWMAX; ++cnt) {
		Scrnmarks[cnt].prev  = &Scrnmarks[cnt - 1];
		Scrnmarks[cnt].next  = &Scrnmarks[cnt + 1];
	}
	Scrnmarks[ROWMAX - 1].next = NULL;

	/* init the Mrklist */
	Mrklist = &Scrnmarks[ROWMAX - 1];
}
