#ifdef SCROLLBARS

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../z.h"
#include "xwind.h"

#define THUMB_SIZE	26

static int thumbSize = THUMB_SIZE;

extern int win_width;

#define THUMB_WIDTH		(SCROLLBAR_WIDTH - 2)

#ifdef HSCROLL
static void HscrollEvent(XEvent *event, struct wdo *wdo);
#endif
static void VscrollEvent(XEvent *event, struct wdo *wdo);

/* Called by Wcreate to create scrollbars from line first to last */
void CreateScrollBars(struct wdo *wdo)
{
	static int set = 0, thumbColor, troughColor;

	if(!set)
	{
		if(!ColorResource(".thumbColor",".thumbColor", &thumbColor))
			thumbColor = foreground;
		if(!ColorResource(".troughColor", ".troughColor", &troughColor))
			troughColor = background;
		set = 1;
	}

	wdo->vheight = Xrow[wdo->last] - Xrow[wdo->first] - 2;

	wdo->vscroll = CreateWindow(Zroot,
		win_width - SCROLLBAR_WIDTH - 2, Xrow[wdo->first],
		SCROLLBAR_WIDTH, wdo->vheight,
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

	wdo->vthumb = CreateWindow(wdo->vscroll,
		0, 0,
		THUMB_WIDTH, THUMB_SIZE, 0);

	XDefineCursor(display, wdo->vscroll, vscrollcursor);

	XSetWindowBackground(display, wdo->vscroll, troughColor);
	XSetWindowBackground(display, wdo->vthumb, thumbColor);
	XClearWindow(display, wdo->vscroll);	/* force update */
	XClearWindow(display, wdo->vthumb);		/* force update */

#ifdef HSCROLL
	wdo->hscroll = CreateWindow(Zroot,
		0, Xrow[wdo->last],
		Xcol[Colmax] - 3, SCROLLBAR_WIDTH,
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

	wdo->hthumb = CreateWindow(wdo->hscroll,
		0, 0,
		Xcol[Colmax] - 4, THUMB_WIDTH, 0);

	XDefineCursor(display, wdo->hscroll, hscrollcursor);

	XSetWindowBackground(display, wdo->hscroll, troughColor);
	XSetWindowBackground(display, wdo->hthumb, thumbColor);
	XClearWindow(display, wdo->hscroll);	/* force update */
	XClearWindow(display, wdo->hthumb);		/* force update */
#endif

	XFlush(display);						/* necessary */
}


/* Called by Wfree */
void DeleteScrollBars(struct wdo *wdo)
{
	XDestroyWindow(display, wdo->vscroll);	/* this will destroy thumb */
#ifdef HSCROLL
	XDestroyWindow(display, wdo->hscroll);	/* this will destroy thumb */
#endif
}


void ResizeScrollBars(struct wdo *wdo)
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
	UpdateScrollbars();
}


/* Move thumb to pixel value */
static void ThumbTo(struct wdo *wdo, int y)
{
	if(y < thumbSize) y = 0;
	else if(y > wdo->vheight - thumbSize) y = wdo->vheight - thumbSize;
	else y -= thumbSize / 2;
	XMoveWindow(display, wdo->vthumb, 0, y);
}

static void ThumbSize(struct wdo *wdo, int height)
{
	if(thumbSize != height)
	{
		if(height < 5) height = 5;	/* minimum size */
		XResizeWindow(display, wdo->vthumb, THUMB_WIDTH, height);
		thumbSize = height;
	}
}

/* Note that GotoLine is an implicit Wswitchto */
static void GotoLine(struct wdo *wdo, int line)
{
	Argp = TRUE;
	Arg  = line;
	ShowCursor(FALSE);
	Wswitchto(wdo);
	Zlgoto();
	Refresh();
	ShowCursor(TRUE);
}


void ScrollEvent(event)
XEvent *event;
{
	struct wdo *wdo;
	Window window = event->xany.window;

	if(event->type != ButtonPress) return;

	/* find the wdo associated with this window */
	for(wdo = Whead; wdo; wdo = wdo->next)
		if(window == wdo->vscroll)
		{
			VscrollEvent(event, wdo);
			return;
		}
#ifdef HSCROLL
		else if(window == wdo->hscroll)
		{
			HscrollEvent(event, wdo);
			return;
		}
#endif
}

/* We received a ButtonPress event in a vscroll window */
static void VscrollEvent(XEvent *event, struct wdo *wdo)
{
	int lines;

	ThumbTo(wdo, event->xbutton.y);
	lines = Blines(wdo->wbuff);
	GotoLine(wdo, lines * event->xbutton.y / wdo->vheight);

	do
	{
		XNextEvent(display, event);
		if(event->type == MotionNotify)
		{
			while(XCheckMaskEvent(display, PointerMotionMask, event)) ;
			ThumbTo(wdo, event->xmotion.y);
			GotoLine(wdo, lines * event->xmotion.y / wdo->vheight);
		}
	}
	while(event->type != ButtonRelease);
}

/* Called by Refresh() */
/*SAM Optimize could make use of window changed? */
void UpdateScrollbars()
{
	unsigned line;
	int lines = Blines(Curwdo->wbuff);

	if(lines < Curwdo->last)
	{	/* thumb fills entire window */
		ThumbTo(Curwdo, 0);
		ThumbSize(Curwdo, Curwdo->vheight);
	}
	else
	{
		Blocation(&line);
		ThumbTo(Curwdo, line * Curwdo->vheight / lines);
		ThumbSize(Curwdo, Curwdo->last * Curwdo->vheight / lines);
	}
}

#ifdef HSCROLL
/********************************************************************\
 *																	*
 *					Handle the horizontal scrollbars				*
 *																	*
\********************************************************************/

int Hshift = 0;

/* Move thumb to pixel value */
static void HThumbTo(struct wdo *wdo, int x)
{
	if(x < 0 || x >= Xcol[Colmax] - 3) return;

	/* SAM HACK for now */
	XMoveWindow(display, wdo->hthumb, x, 0);
}

/* We received a ButtonPress event in a hscroll window */
static void HscrollEvent(XEvent *event, struct wdo *wdo)
{
	HThumbTo(wdo, event->xbutton.x);

	do
	{
		XNextEvent(display, event);
		if(event->type == MotionNotify)
		{
			while(XCheckMaskEvent(display, PointerMotionMask, event)) ;
			HThumbTo(wdo, event->xmotion.x);
		}
	}
	while(event->type != ButtonRelease);
}
#endif
#endif
