/* xscroll.c - Zedit X scrollbar routines
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

#ifdef SCROLLBARS

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../z.h"

#define THUMB_SIZE	26

static int thumbSize = THUMB_SIZE;

#define THUMB_WIDTH		(SCROLLBAR_WIDTH - 2)

#ifdef HSCROLL
static void hscrollevent(XEvent *event, struct wdo *wdo);
#endif
static void vscrollevent(XEvent *event, struct wdo *wdo);

/* Called by Wcreate to create scrollbars from line first to last */
void createscrollbars(struct wdo *wdo)
{
	static int set = 0, thumbColor, troughColor;

	if (!set) {
		if (!colour_resource(".thumbColor", ".thumbColor",
				   &thumbColor))
			thumbColor = foreground;
		if (!colour_resource(".troughColor", ".troughColor",
				   &troughColor))
			troughColor = background;
		set = 1;
	}

	wdo->vheight = Xrow[wdo->last] - Xrow[wdo->first] - 2;

	wdo->vscroll = createwindow(Zroot,
		win_width - SCROLLBAR_WIDTH - 2, Xrow[wdo->first],
		SCROLLBAR_WIDTH, wdo->vheight,
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

	wdo->vthumb = createwindow(wdo->vscroll,
		0, 0,
		THUMB_WIDTH, THUMB_SIZE, 0);

	XDefineCursor(display, wdo->vscroll, vscrollcursor);

	XSetWindowBackground(display, wdo->vscroll, troughColor);
	XSetWindowBackground(display, wdo->vthumb, thumbColor);
	XClearWindow(display, wdo->vscroll);	/* force update */
	XClearWindow(display, wdo->vthumb);		/* force update */

#ifdef HSCROLL
	wdo->hscroll = createwindow(Zroot,
		0, Xrow[wdo->last],
		Xcol[Colmax] - 3, SCROLLBAR_WIDTH,
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

	wdo->hthumb = createwindow(wdo->hscroll,
		0, 0,
		Xcol[Colmax] - 4, THUMB_WIDTH, 0);

	XDefineCursor(display, wdo->hscroll, hscrollcursor);

	XSetWindowBackground(display, wdo->hscroll, troughColor);
	XSetWindowBackground(display, wdo->hthumb, thumbColor);
	XClearWindow(display, wdo->hscroll);	/* force update */
	XClearWindow(display, wdo->hthumb);		/* force update */
#endif

	XFlush(display); /* necessary */
}

/* Called by Wfree */
void deletescrollbars(struct wdo *wdo)
{
	XDestroyWindow(display, wdo->vscroll);	/* this will destroy thumb */
#ifdef HSCROLL
	XDestroyWindow(display, wdo->hscroll);	/* this will destroy thumb */
#endif
}

void resizescrollbars(struct wdo *wdo)
{
	wdo->vheight = Xrow[wdo->last] - Xrow[wdo->first] - 2;
	XMoveResizeWindow(display, wdo->vscroll,
		win_width - SCROLLBAR_WIDTH - 2, Xrow[wdo->first],
		SCROLLBAR_WIDTH, wdo->vheight);
#ifdef HSCROLL
	XMoveResizeWindow(display, wdo->hscroll,
		0, Xrow[wdo->last],
		Xcol[Colmax] - 3, SCROLLBAR_WIDTH);
#endif
	/* update thumb */
	updatescrollbars();
}

/* Move thumb to pixel value */
static void thumbto(struct wdo *wdo, int y)
{
	if (y < thumbSize)
		y = 0;
	else if (y > wdo->vheight - thumbSize)
		y = wdo->vheight - thumbSize;
	else
		y -= thumbSize / 2;
	XMoveWindow(display, wdo->vthumb, 0, y);
}

static void thumbsize(struct wdo *wdo, int height)
{
	if (thumbSize != height) {
		if (height < 5)
			height = 5;	/* minimum size */
		XResizeWindow(display, wdo->vthumb, THUMB_WIDTH, height);
		thumbSize = height;
	}
}

/* Note that GotoLine is an implicit Wswitchto */
static void gotoline(struct wdo *wdo, int line)
{
	Argp = TRUE;
	Arg  = line;
	showcursor(FALSE);
	wswitchto(wdo);
	Zlgoto();
	refresh();
	showcursor(TRUE);
}

void scrollevent(XEvent *event)
{
	struct wdo *wdo;
	Window window = event->xany.window;

	if (event->type != ButtonPress)
		return;

	/* find the wdo associated with this window */
	for (wdo = Whead; wdo; wdo = wdo->next)
		if (window == wdo->vscroll) {
			vscrollevent(event, wdo);
			return;
		}
#ifdef HSCROLL
		else if (window == wdo->hscroll) {
			hscrollevent(event, wdo);
			return;
		}
#endif
}

/* We received a ButtonPress event in a vscroll window */
static void vscrollevent(XEvent *event, struct wdo *wdo)
{
	int lines;

	thumbto(wdo, event->xbutton.y);
	lines = blines(wdo->wbuff);
	gotoline(wdo, lines * event->xbutton.y / wdo->vheight);

	do {
		XNextEvent(display, event);
		if (event->type == MotionNotify) {
			while (XCheckMaskEvent(display, PointerMotionMask,
					       event))
				;
			thumbto(wdo, event->xmotion.y);
			gotoline(wdo, lines * event->xmotion.y / wdo->vheight);
		}
	} while (event->type != ButtonRelease);
}

/* Called by Refresh() */
/*SAM Optimize could make use of window changed? */
void updatescrollbars()
{
	unsigned line;
	int lines = blines(Curwdo->wbuff);

	if (lines < Curwdo->last) {
		/* thumb fills entire window */
		thumbto(Curwdo, 0);
		thumbsize(Curwdo, Curwdo->vheight);
	} else {
		blocation(&line);
		thumbto(Curwdo, line * Curwdo->vheight / lines);
		thumbsize(Curwdo, Curwdo->last * Curwdo->vheight / lines);
	}
}

#ifdef HSCROLL

int Hshift;

/* Move thumb to pixel value */
static void hthumbto(struct wdo *wdo, int x)
{
	if (x < 0 || x >= Xcol[Colmax] - 3)
		return;

	/* SAM HACK for now */
	XMoveWindow(display, wdo->hthumb, x, 0);
}

/* We received a ButtonPress event in a hscroll window */
static void hscrollevent(XEvent *event, struct wdo *wdo)
{
	hthumbto(wdo, event->xbutton.x);

	do {
		XNextEvent(display, event);
		if (event->type == MotionNotify) {
			while (XCheckMaskEvent(display, PointerMotionMask,
					       event))
				;
			hthumbto(wdo, event->xmotion.x);
		}
	} while (event->type != ButtonRelease);
}

#endif
#endif
