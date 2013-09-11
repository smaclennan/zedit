/* window.c - Zedit windowing commands and functions
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

struct wdo *Whead, *Curwdo;

#define MINWDO		5		/* minimum window size */

/* Create a new window pointer - screen info invalid */
static struct wdo *wcreate(int first, int last)
{
	struct wdo *new = calloc(1, sizeof(struct wdo));

	if (new) {
		new->wbuff	= Curbuff;
		new->wpnt	= bcremrk();
		new->wmrk	= bcremrk();
		new->wstart	= bcremrk();
		new->modeflags	= INVALID;
		new->first	= first;
		new->last	= last;
		mrktomrk(new->wmrk, Curbuff->mark);
	}
	return new;
}

/* free wdo - may invalidate Curwdo and Whead */
static void wfree(struct wdo *wdo)
{
	unmark(wdo->wpnt);
	unmark(wdo->wmrk);
	unmark(wdo->wstart);
	free((char *)wdo);
}

/*
 * Invalidate an entire window. i.e. next refresh will do a complete update.
 * Note that the line BEFORE the window must be invalidated to make sure that
 * the window is updated correctly.
 */
static void winvalidate(struct wdo *wdo)
{
	int i;

	if (wdo->first > Tstart)
		Scrnmarks[wdo->first - 1].modf = TRUE;
	for (i = wdo->first; i <= wdo->last; ++i)
		Scrnmarks[i].modf = TRUE;
	wdo->modeflags = INVALID;
}

static Boolean wdelete(struct wdo *wdo)
{
	struct wdo *new;

	/* can't delete last window */
	if (!Whead->next)
		return FALSE;

	/* give more space to the smaller of the 2 windows (favour next) */
	if ((wdo->next ? wdo->next->last - wdo->next->first : ROWMAX + 1) <=
	   (wdo->prev ? wdo->prev->last - wdo->prev->first : ROWMAX + 1)) {
		/* give it to the next window */
		new = wdo->next;
		new->first = wdo->first;
		new->prev = wdo->prev;
		if (wdo->prev)
			wdo->prev->next = new;
		else
			Whead = new;
	} else if (wdo->prev) {
		/* give it to the previous window */
		new = wdo->prev;
		new->last = wdo->last;
		new->next = wdo->next;
		if (wdo->next)
			wdo->next->prev = new;
		new->modeflags = INVALID;
	} else
		return FALSE;

	winvalidate(wdo);

	if (wdo == Curwdo) {
		wswitchto(new);
		reframe();	/*SAM*/
	}
	wfree(wdo);
	return TRUE;
}

/*
 * Split the current window in 2, putting the same buffer in both windows.
 * Leaves the user in the new window.
 */
static Boolean wsplit(void)
{
	struct wdo *new;
	int first, last;

	if (wheight() < MINWDO)
		return FALSE;

	/* Create the new window. */
	first = Curwdo->first + (wheight() / 2) + 1;
	last = Curwdo->last;
	new = wcreate(first, last);
	if (!new)
		return FALSE;

	/* resize the old window */
	Curwdo->last = first - 1;
	new->first = Curwdo->last + 1;
	Curwdo->modeflags = INVALID;

	/* link it into chain */
	new->prev = Curwdo;
	new->next = Curwdo->next;
	if (Curwdo->next)
		Curwdo->next->prev = new;
	Curwdo->next = new;

	/* Point may be off new screen, reframe just in case... */
	reframe();

	/* Go to new window. */
	wswitchto(new);
	reframe();
	mrktomrk(Curwdo->wstart, Sstart);
	return TRUE;
}

/* Find the window associated with buffer */
struct wdo *findwdo(struct buff *buff)
{
	struct wdo *wdo;

	for (wdo = Whead; wdo; wdo = wdo->next)
		if (wdo->wbuff == buff)
			return wdo;
	return NULL;
}

/* Switch to another window. */
void wswitchto(struct wdo *wdo)
{
	if (wdo != Curwdo) {
		if (Curwdo) {
			bmrktopnt(Curwdo->wpnt);
			mrktomrk(Curwdo->wmrk, Curbuff->mark);
			/* don't update wstart unless Sstart for this window */
			if (Sstart->mbuff == Curwdo->wbuff)
				mrktomrk(Curwdo->wstart, Sstart);
		}
		Curwdo = wdo;
		bswitchto(wdo->wbuff);
		bpnttomrk(wdo->wpnt);
		mrktomrk(Curbuff->mark, wdo->wmrk);
		mrktomrk(Sstart, wdo->wstart);
		Sendp = FALSE;
	}
	Curwdo->modeflags = INVALID;
}

/* Switch to a new buffer in the current window. */
void cswitchto(struct buff *buff)
{
	bswitchto(buff);
	if (Curwdo->wbuff != Curbuff) {
		Curwdo->wbuff = Curbuff;
		bmrktopnt(Curwdo->wpnt);
		mrktomrk(Curwdo->wmrk, Curbuff->mark);
		if (Sstart->mbuff == Curbuff)
			mrktomrk(Curwdo->wstart, Sstart);
		else {
			/* bring to start of buffer - just in case */
			Curwdo->wstart->mbuff = Curbuff;
			Curwdo->wstart->mpage = Curbuff->firstp;
			Curwdo->wstart->moffset = 0;
		}
		Curwdo->modeflags = INVALID;

		settabsize(buff->bmode);
	}

}

/* Local routine to change the current window by 'size' lines */
static Boolean sizewindow(int size)
{
	struct wdo *other;

	if (wheight() + size < MINWDO)
		return FALSE;
	other = Curwdo->next;
	if (other && other->last - other->first - size > MINWDO) {
		Curwdo->last += size;
		other->first += size;
	} else {
		other = Curwdo->prev;
		if (other && other->last - other->first - size > MINWDO) {
			Curwdo->first -= size;
			other->last   -= size;
		} else
			return FALSE;
	}

	/* invalidate the windows */
	winvalidate(Curwdo);
	winvalidate(other);

	return TRUE;
}

static int nopaw_resize;

static void do_wsize(int orow)
{
	struct wdo *wdo;
	Boolean changed = TRUE;
	int i, d = Rowmax - orow;

	if (d > 0) {
		/* make the windows bigger starting at the top */
		while (d > 0)
			for (i = 1, wdo = Whead; wdo; wdo = wdo->next) {
				wdo->last += i;
				if (wdo->next)
					wdo->next->first += i;
				if (--d > 0)
					++i;
			}
	} else {
		/* make the windows smaller starting at the bottom */
		d = -d;
		while (d > 0 && changed) {
			changed = FALSE;
			for (i = 1, wdo = Whead; wdo; wdo = wdo->next) {
				if (wdo->last - wdo->first - 1 > 3 && d > 0) {
					wdo->last -= i;
					if (wdo->next)
						wdo->next->first -= i;
					if (d-- > 0)
						++i;
					changed = TRUE;
				} else {
					wdo->last -= i - 1;
					if (wdo->next)
						wdo->next->first -= i - 1;
				}
			}
		}
		if (d > 0)
			Z1wind();
	}
}

/* See if window size has changed */
void wsize(void)
{
	int orow;

	if (nopaw_resize)
		return;

	orow = Rowmax;
	termsize();

	/* if Rowmax changed we must update window sizes */
	if (Rowmax != orow) {
		if (Whead->next)
			do_wsize(orow);
		else
			Whead->last = Rowmax - 2;
	}
}

/* paw_resize PAW by moving bottom window by 'diff' lines, if possible. */
Boolean paw_resize(int diff)
{
	struct wdo *last;
	int i;

	/* find the last window */
	for (last = Whead; last->next; last = last->next)
		;

	if (last->last - last->first + diff < 1)
		return FALSE;
	if (diff > 0)
		for (i = 0; i < diff; ++i)
			Scrnmarks[i + last->last].modf = TRUE;
	last->last += diff;
	Rowmax += diff;
	last->modeflags = INVALID;
	clrecho();
	return TRUE;
}

/*
 * Create/Reuse a buffer in another window. Window to use:
 *	1. if buffer in a window - use window
 *	2. if >1 windows - use bottom window
 *	3. if 1 window - create 2nd 10 line window and use it
 *	4. if all else fails, use current
 * Makes new window and buffer current.
 * NOTE: Blows away previous buffer.
 */
Boolean wuseother(char *bname)
{
	struct wdo *wdo, *last;
	struct buff *buff;

	for (wdo = Whead, last = NULL; wdo; last = wdo, wdo = wdo->next)
		if (strcmp(wdo->wbuff->bname, bname) == 0)
			break;
	if (wdo)
		wswitchto(wdo);
	else if (last != Whead)
		wswitchto(last);
	else {
		wsplit();
		if ((strcmp(bname, MAKEBUFF) == 0 ||
		     strcmp(bname, REFBUFF) == 0)
			&& wheight() > 8) {
			/* .make/.ref buffers are smaller */
			Curwdo->first = Curwdo->last - 8;
			Curwdo->prev->last = Curwdo->first - 1;
		}
	}
	winvalidate(Curwdo);
	buff = cmakebuff(bname, NULL);
	if (buff == NULL)
		return FALSE;
	cswitchto(buff);
	bempty();
	return TRUE;
}

/* Split the current window and enter new (bottom) window */
void Z2wind(void)
{
	if (!wsplit())
		tbell();
}

/* Tear down all but one (current) window */
void Z1wind(void)
{
	struct wdo *wdo;
	int i;

	while (Whead) {
		wdo = Whead;
		Whead = Whead->next;
		if (wdo != Curwdo)
			wfree(wdo);
	}

	Curwdo->first = Tstart;
	Curwdo->last = tmaxrow() - 2;
	Curwdo->modeflags = INVALID;
	Curwdo->prev = Curwdo->next = NULL;
	Whead = Curwdo;

	for (i = 0; i < Curwdo->last; ++i)
		Scrnmarks[i].modf = TRUE;

	tclrwind();
}

/* Delete current window if more than one */
void Zdelwind(void)
{
	if (!wdelete(Curwdo))
		tbell();
}

/* Make previous window current */
void Zprevwind(void)
{
	struct wdo *wdo;

	if (Curwdo->prev)
		wswitchto(Curwdo->prev);
	else {
		for (wdo = Whead; wdo->next; wdo = wdo->next)
			;
		if (wdo != Curwdo)
			wswitchto(wdo);
		else
			tbell();
	}
}

/* Make next window current */
void Znextwind(void)
{
	if (Curwdo->next)
		wswitchto(Curwdo->next);
	else if (Curwdo != Whead)
		wswitchto(Whead);
	else
		tbell();
}

/* Make current window bigger */
void Zgrowwind(void)
{
	sizewindow(Arg);
	Arg = 0;
}

/* Make current window smaller */
void Zshrinkwind(void)
{
	sizewindow(-Arg);
	Arg = 0;
}

/* Make current window an absolute size */
void Zsizewind(void)
{
	if (!sizewindow(Arg - wheight() + 1))
		tbell();
	Arg = 0;
}

/*
 * Find the "other" window:
 *	- if there is an Arg - go to Arg window (start at 1)
 *	- use bottom window unless current
 *	- use top window
 *	- use current if only one
 * Makes new window current.
 * Never fails.
 */
static struct wdo *otherwind(void)
{
	struct wdo *wdo;

	if (Argp)
		for (wdo = Whead;
		     --Arg > 0;
		     wdo = wdo->next ? wdo->next : Whead)
			;
	else {
		for (wdo = Whead; wdo->next; wdo = wdo->next)
			;
		if (wdo == Curwdo)
			wdo = Whead;
	}
	wswitchto(wdo);
	return wdo;
}

void Znxtothrwind(void)
{
	struct wdo *save = Curwdo;
	otherwind();
	Znextpage();
	wswitchto(save);
}

void Zprevothrwind(void)
{
	struct wdo *save = Curwdo;
	otherwind();
	Zprevpage();
	wswitchto(save);
}

/*
 * If buffer is in a current window, switchto that window, else put the buffer
 * in the current or other window.
 */
void bgoto(struct buff *buff)
{
	struct wdo *wdo = findwdo(buff);

	if (wdo)
		wswitchto(wdo);
	else
		cswitchto(buff);
}

void winit(void)
{
	/* Create first window over entire screen. */
	Whead = wcreate(Tstart, Rowmax - 2);
	wswitchto(Whead);
}

void wfini(void)
{
	while (Whead) {
		struct wdo *next = Whead->next;
		wfree(Whead);
		Whead = next;
	}
}
