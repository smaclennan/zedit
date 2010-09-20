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

static void Wfree(struct wdo *);
static Boolean Wdelete(struct wdo *);
static Boolean Wsplit(void);


/* Create a new window pointer - screen info invalid */
static struct wdo *Wcreate(int first, int last)
{
	struct wdo *new = calloc(sizeof(struct wdo), 1);

	if (new) {
		new->wbuff	= Curbuff;
		new->wpnt	= Bcremrk();
		new->wmrk	= Bcremrk();
		new->wstart	= Bcremrk();
		new->modeflags	= INVALID;
		new->first	= first;
		new->last	= last;
		Mrktomrk(new->wmrk, Curbuff->mark);
#ifdef SCROLLBARS
		CreateScrollBars(new);
#endif
	}
	return new;
}

/* free wdo - may invalidate Curwdo and Whead */
static void Wfree(struct wdo *wdo)
{
	Unmark(wdo->wpnt);
	Unmark(wdo->wmrk);
	Unmark(wdo->wstart);
#ifdef SCROLLBARS
	DeleteScrollBars(wdo);
#endif
	free((char *)wdo);
}

static Boolean Wdelete(struct wdo *wdo)
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

	Winvalid(wdo);

	if (wdo == Curwdo) {
		Wswitchto(new);
		Reframe();	/*SAM*/
	}
	Wfree(wdo);
#ifdef SCROLLBARS
	ResizeScrollBars(new);
#endif
	return TRUE;
}

/*
 * Split the current window in 2, putting the same buffer in both windows.
 * Leaves the user in the new window.
 */
static Boolean Wsplit()
{
	struct wdo *new;
	int first, last;

	if (Wheight() < MINWDO)
		return FALSE;

	/* Create the new window. */
	first = Curwdo->first + (Wheight() / 2) + 1;
	last = Curwdo->last;
	new = Wcreate(first, last);
	if (!new)
		return FALSE;

	/* resize the old window */
	Curwdo->last = first - 1;
	new->first = Curwdo->last + 1;
	Curwdo->modeflags = INVALID;
#ifdef SCROLLBARS
	ResizeScrollBars(Curwdo);
#endif

	/* link it into chain */
	new->prev = Curwdo;
	new->next = Curwdo->next;
	if (Curwdo->next)
		Curwdo->next->prev = new;
	Curwdo->next = new;

	/* Point may be off new screen, reframe just in case... */
	Reframe();

	/* Go to new window. */
	Wswitchto(new);
	Reframe();
	Mrktomrk(Curwdo->wstart, Sstart);
	return TRUE;
}

/* Find the window associated with buffer */
struct wdo *Findwdo(struct buff *buff)
{
	struct wdo *wdo;

	for (wdo = Whead; wdo; wdo = wdo->next)
		if (wdo->wbuff == buff)
			return wdo;
	return NULL;
}

/* Switch to another window. */
void Wswitchto(struct wdo *wdo)
{
	if (wdo != Curwdo) {
		if (Curwdo) {
			Bmrktopnt(Curwdo->wpnt);
			Mrktomrk(Curwdo->wmrk, Curbuff->mark);
			/* don't update wstart unless Sstart for this window */
			if (Sstart->mbuff == Curwdo->wbuff)
				Mrktomrk(Curwdo->wstart, Sstart);
		}
		Curwdo = wdo;
		Bswitchto(wdo->wbuff);
		Bpnttomrk(wdo->wpnt);
		Mrktomrk(Curbuff->mark, wdo->wmrk);
		Mrktomrk(Sstart, wdo->wstart);
		Sendp = FALSE;
#ifdef XWINDOWS
		XSwitchto(wdo->wbuff->bname);
#endif
	}
	Curwdo->modeflags = INVALID;
}

/* Switch to a new buffer in the current window. */
void Cswitchto(struct buff *buff)
{
	Bswitchto(buff);
	if (Curwdo->wbuff != Curbuff) {
		Curwdo->wbuff = Curbuff;
		Bmrktopnt(Curwdo->wpnt);
		Mrktomrk(Curwdo->wmrk, Curbuff->mark);
		if (Sstart->mbuff == Curbuff)
			Mrktomrk(Curwdo->wstart, Sstart);
		else {
			/* bring to start of buffer - just in case */
			Curwdo->wstart->mbuff = Curbuff;
			Curwdo->wstart->mpage = Curbuff->firstp;
			Curwdo->wstart->moffset = 0;
		}
		Curwdo->modeflags = INVALID;

		Settabsize(buff->bmode);
	}

#ifdef XWINDOWS
		XSwitchto(buff->bname);
#endif
}

/* Local routine to change the current window by 'size' lines */
static Boolean Sizewindow(int size)
{
	struct wdo *other;

	if (Wheight() + size < MINWDO)
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
	Winvalid(Curwdo);
	Winvalid(other);

#ifdef SCROLLBARS
	ResizeScrollBars(Curwdo);
	ResizeScrollBars(other);
#endif
	return TRUE;
}

int noResize;

static inline void do_wsize(int orow)
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
void Wsize()
{
	int orow;

	if (noResize)
		return;

	orow = Rowmax;
	Termsize();

	/* if Rowmax changed we must update window sizes */
	if (Rowmax != orow) {
		if (Whead->next)
			do_wsize(orow);
		else
			Whead->last = Rowmax - 2;
	}

#ifdef SCROLLBARS
	/* We always update scrollbars since width may have changed. */
	{
		struct wdo *wdo;
		for (wdo = Whead; wdo; wdo = wdo->next)
			ResizeScrollBars(wdo);
	}
#endif
}

/* Resize PAW by moving bottom window by 'diff' lines, if possible. */
Boolean Resize(int diff)
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
	Clrecho();
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
Boolean WuseOther(char *bname)
{
	struct wdo *wdo, *last;
	struct buff *buff;

	for (wdo = Whead, last = NULL; wdo; last = wdo, wdo = wdo->next)
		if (strcmp(wdo->wbuff->bname, bname) == 0)
			break;
	if (wdo)
		Wswitchto(wdo);
	else if (last != Whead)
		Wswitchto(last);
	else {
		Wsplit();
		if ((strcmp(bname, MAKEBUFF) == 0 ||
		     strcmp(bname, REFBUFF) == 0)
			&& Wheight() > 8) {
			/* .make/.ref buffers are smaller */
			Curwdo->first = Curwdo->last - 8;
			Curwdo->prev->last = Curwdo->first - 1;
#ifdef SCROLLBARS
			ResizeScrollBars(Curwdo->prev);
			ResizeScrollBars(Curwdo);
#endif
		}
	}
	Winvalid(Curwdo);
	buff = Cmakebuff(bname, NULL);
	if (buff == NULL)
		return FALSE;
	Cswitchto(buff);
	Bempty();
	return TRUE;
}

/*
 * Invalidate an entire window. i.e. next Refresh will do a complete update.
 * Note that the line BEFORE the window must be invalidated to make sure that
 * the window is updated correctly.
 */
void Winvalid(struct wdo *wdo)
{
	int i;

	if (wdo->first > Tstart)
		Scrnmarks[wdo->first - 1].modf = TRUE;
	for (i = wdo->first; i <= wdo->last; ++i)
		Scrnmarks[i].modf = TRUE;
	wdo->modeflags = INVALID;
}

/* Split the current window and enter new (bottom) window */
void Z2wind()
{
	if (!Wsplit())
		Tbell();
}

/* Tear down all but one (current) window */
void Z1wind()
{
	struct wdo *wdo;
	int i;

	while (Whead) {
		wdo = Whead;
		Whead = Whead->next;
		if (wdo != Curwdo)
			Wfree(wdo);
	}

	Curwdo->first = Tstart;
	Curwdo->last = Tmaxrow() - 2;
	Curwdo->modeflags = INVALID;
	Curwdo->prev = Curwdo->next = NULL;
	Whead = Curwdo;

	for (i = 0; i < Curwdo->last; ++i)
		Scrnmarks[i].modf = TRUE;

	Tclrwind();

#ifdef SCROLLBARS
	ResizeScrollBars(Curwdo);
#endif
}

/* Delete current window if more than one */
void Zdelwind()
{
	if (!Wdelete(Curwdo))
		Tbell();
}

/* Make previous window current */
void Zprevwind()
{
	struct wdo *wdo;

	if (Curwdo->prev)
		Wswitchto(Curwdo->prev);
	else {
		for (wdo = Whead; wdo->next; wdo = wdo->next)
			;
		if (wdo != Curwdo)
			Wswitchto(wdo);
		else
			Tbell();
	}
}

/* Make next window current */
void Znextwind()
{
	if (Curwdo->next)
		Wswitchto(Curwdo->next);
	else if (Curwdo != Whead)
		Wswitchto(Whead);
	else
		Tbell();
}

/* Make current window bigger */
void Zgrowwind()
{
	Sizewindow(Arg);
	Arg = 0;
}

/* Make current window smaller */
void Zshrinkwind()
{
	Sizewindow(-Arg);
	Arg = 0;
}

/* Make current window an absolute size */
void Zsizewind()
{
	if (!Sizewindow(Arg - Wheight() + 1))
		Tbell();
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
static struct wdo *Otherwind()
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
	Wswitchto(wdo);
	return wdo;
}

void Znxtothrwind()
{
	struct wdo *save = Curwdo;
	Otherwind();
	Znextpage();
	Wswitchto(save);
}

void Zprevothrwind()
{
	struct wdo *save = Curwdo;
	Otherwind();
	Zprevpage();
	Wswitchto(save);
}

/*
 * If buffer is in a current window, switchto that window, else put the buffer
 * in the current or other window.
 */
void Bgoto(struct buff *buff)
{
	struct wdo *wdo = Findwdo(buff);

	if (wdo)
		Wswitchto(wdo);
	else
		Cswitchto(buff);
}

/* These routines are ONLY callable at startup. */

static struct wdo *Wstart;	/* Set in Wload */

void Winit()
{
	if (Wstart == NULL) {
		/* Create first window over entire screen. */
		Whead = Wcreate(Tstart, Rowmax - 2);
		Wswitchto(Whead);
	} else {
		/* We created the window[s] with Wload[s]. */
		while (Whead->prev)
			Whead = Whead->prev;
		Wswitchto(Wstart);
		Wsize();
	}
}

void Wload(char *bname, int first, int last, unsigned long sloc, int iscurrent)
{
	struct wdo *new;
	struct buff *buff;

#ifdef SHELL
	if (strcmp(bname, SHELLBUFF) == 0) {
		/* invoke the shell */
		buff = Cmakebuff(SHELLBUFF, NULL);
		if (buff == NULL)
			NoMem();
		Doshell();
	}
#endif
	buff = Cfindbuff(bname);
	if (buff == NULL)
		buff = Cfindbuff(MAINBUFF);
	Bswitchto(buff);
	new = Wcreate(first, last);
	if (new == NULL)
		NoMem();
	Mrktomrk(buff->mark, new->wmrk);
	Boffset(sloc);
	Bmrktopnt(new->wstart);
	Bpnttomrk(new->wpnt);	/* return it */
	new->first = first;
	new->last  = last;
	if (Whead) {
		Whead->next = new;
		new->prev = Whead;
	}
	Whead = new;
	if (iscurrent)
		Wstart = new;
	Rowmax = new->last + 2;
}
