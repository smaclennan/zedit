/* window.c - Zedit windowing commands and functions
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

struct wdo *Whead, *Curwdo;

#define MINWDO		5		/* minimum window size */

/* Create a new window pointer - screen info invalid */
static struct wdo *wcreate(int first, int last)
{
	struct wdo *wdo = (struct wdo *)calloc(1, sizeof(struct wdo));

	if (wdo) {
		wdo->wbuff	= Curbuff;
		wdo->wpnt	= zcreatemrk();
		wdo->wmrk	= zcreatemrk();
		wdo->wstart	= zcreatemrk();
		wdo->modeflags	= INVALID;
		wdo->first	= first;
		wdo->last	= last;
		if (UMARK_SET) {
			mrktomrk(wdo->wmrk, UMARK);
			wdo->umark_set = 1;
		}
	}
	return wdo;
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
	if (wdo->first > 0)
		invalidate_scrnmarks(wdo->first - 1, wdo->first);
	invalidate_scrnmarks(wdo->first, wdo->last + 1);
	wdo->modeflags = INVALID;
}

/*
 * Split the current window in 2, putting the same buffer in both windows.
 * Leaves the user in the new window.
 */
static bool wsplit(void)
{
	struct wdo *new_wdo;
	int first, last;

	if (wheight() < MINWDO)
		return false;

	/* Create the new window. */
	first = Curwdo->first + (wheight() / 2) + 1;
	last = Curwdo->last;
	new_wdo = wcreate(first, last);
	if (!new_wdo)
		return false;

	/* resize the old window */
	Curwdo->last = first - 1;
	new_wdo->first = Curwdo->last + 1;
	Curwdo->modeflags = INVALID;

	/* link it into chain */
	new_wdo->next = Curwdo->next;
	Curwdo->next = new_wdo;

	/* Point may be off new screen, reframe just in case... */
	reframe();

	/* Go to new window. */
	wswitchto(new_wdo);
	reframe();
	mrktomrk(Curwdo->wstart, Sstart);
	return true;
}

/* Find the window associated with buffer */
struct wdo *findwdo(struct buff *buff)
{
	struct wdo *wdo;

	foreachwdo(wdo)
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
			if (UMARK_SET) {
				mrktomrk(Curwdo->wmrk, UMARK);
				Curwdo->umark_set = 1;
			} else
				Curwdo->umark_set = 0;
			/* don't update wstart unless Sstart for this window */
			if (Sstart->mbuff == Curwdo->wbuff)
				mrktomrk(Curwdo->wstart, Sstart);
		}
		Curwdo = wdo;
		bswitchto(wdo->wbuff);
		bpnttomrk(wdo->wpnt);
		if (Curwdo->umark_set)
			set_umark(wdo->wmrk);
		mrktomrk(Sstart, wdo->wstart);
		Sendp = false;
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
		if (UMARK_SET) {
			mrktomrk(Curwdo->wmrk, UMARK);
			Curwdo->umark_set = 1;
		} else
			Curwdo->umark_set = 0;
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
static bool sizewindow(int size)
{
	struct wdo *other;

	if (wheight() + size < MINWDO)
		return false;
	other = Curwdo->next;
	if (other && other->last - other->first - size > MINWDO) {
		Curwdo->last += size;
		other->first += size;
	} else {
		for (other = Whead;
		     other && other->next != Curwdo;
		     other = other->next)
			;
		if (other && other->last - other->first - size > MINWDO) {
			Curwdo->first -= size;
			other->last   -= size;
		} else
			return false;
	}

	/* invalidate the windows */
	winvalidate(Curwdo);
	winvalidate(other);

	return true;
}

static void do_wsize(int orow)
{
	struct wdo *wdo;
	bool changed = true;
	int i, d = Rowmax - orow;

	if (d > 0) {
		/* make the windows bigger starting at the top */
		while (d > 0) {
			i = 1;
			foreachwdo(wdo) {
				wdo->last += i;
				if (wdo->next)
					wdo->next->first += i;
				if (--d > 0)
					++i;
			}
		}
	} else {
		/* make the windows smaller starting at the bottom */
		d = -d;
		while (d > 0 && changed) {
			changed = false;
			i = 1;
			foreachwdo(wdo) {
				if (wdo->last - wdo->first - 1 > 3 && d > 0) {
					wdo->last -= i;
					if (wdo->next)
						wdo->next->first -= i;
					if (d-- > 0)
						++i;
					changed = true;
				} else {
					wdo->last -= i - 1;
					if (wdo->next)
						wdo->next->first -= i - 1;
				}
			}
		}
		if (d > 0)
			Zone_window();
	}
}

/* See if window size has changed */
void wsize(void)
{
	int orow = Rowmax;
	termsize();

	/* if Rowmax changed we must update window sizes */
	if (Rowmax != orow) {
		if (Whead->next)
			do_wsize(orow);
		else
			Whead->last = Rowmax - 2;
	}
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
bool wuseother(const char *bname)
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
	else
		wsplit();
	winvalidate(Curwdo);
	buff = cmakebuff(bname, NULL);
	if (buff == NULL)
		return false;
	cswitchto(buff);
	bempty();
	return true;
}

void Zsplit_window(void)
{
	wsplit();
}

void Zone_window(void)
{
	struct wdo *wdo;

	while (Whead) {
		wdo = Whead;
		Whead = Whead->next;
		if (wdo != Curwdo)
			wfree(wdo);
	}

	Curwdo->first = 0;
	Curwdo->last = Rowmax - 2;
	Curwdo->modeflags = INVALID;
	Curwdo->next = NULL;
	Whead = Curwdo;

	invalidate_scrnmarks(0, Curwdo->last);

	tclrwind();
}

void Znext_window(void)
{
	if (Curwdo->next)
		wswitchto(Curwdo->next);
	else if (Curwdo != Whead)
		wswitchto(Whead);
}

void Zgrow_window(void)
{
	sizewindow(Arg);
	Arg = 0;
}

void Zsize_window(void)
{
	if (!sizewindow(Arg - wheight() + 1))
		tbell();
	Arg = 0;
}

static void other_page(void (*action)(void))
{
	struct wdo *wdo, *save = Curwdo;

	/* Find the bottom window */
	for (wdo = Whead; wdo->next; wdo = wdo->next)
		;
	/* If we are already the bottom, use the top */
	if (wdo == Curwdo)
		wdo = Whead;

	wswitchto(wdo);
	action();
	wswitchto(save);
}

void Zother_next_page(void)
{
	other_page(Znext_page);
}

void Zother_previous_page(void)
{
	other_page(Zprevious_page);
}

/*
 * If buffer is in a current window, switchto that window, else put the buffer
 * in the current or other window.
 */
void wgoto(struct buff *buff)
{
	struct wdo *wdo = findwdo(buff);

	if (wdo)
		wswitchto(wdo);
	else
		cswitchto(buff);
}

static void wfini(void)
{
	while (Whead) {
		struct wdo *next = Whead->next;
		wfree(Whead);
		Whead = next;
	}
}

void winit(void)
{
	/* Create first window over entire screen. */
	Whead = wcreate(0, Rowmax - 2);
	wswitchto(Whead);
	atexit(wfini);
}

struct wdo *wfind(int row)
{
	struct wdo *wdo;

	/* wdo->last == modeline */
	for (wdo = Whead; wdo; wdo = wdo->next)
		if (row >= wdo->first && row < wdo->last)
			return wdo;

	return NULL;
}
