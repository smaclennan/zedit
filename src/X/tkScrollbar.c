#ifdef BORDER3D

/* 
 * tkScrollbar.c --
 *
 *	This module implements a scrollbar widgets for the Tk
 *	toolkit.  A scrollbar displays a slider and two arrows;
 *	mouse clicks on features within the scrollbar cause
 *	scrolling commands to be invoked.
 *
 * Copyright (c) 1990-1994 The Regents of the University of California.
 * Copyright (c) 1994-1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/* Very modified for use in Zedit. See tk3d.c for disclaimers in full. */

#include <X11/Xlib.h>
#define XWINDOWS 1
#include "global.h"
#include "xwind.h"
#include "buff.h"

#define Tk_Depth(win)		DefaultDepth(display, screen)
#define Tk_WindowId(win)	(win)
#define Tk_Width(win)		wwidth
#define Tk_Height(win)		wheight

/*
 * Minimum slider length, in pixels (designed to make sure that the slider
 * is always easy to grab with the mouse).
 */

#define MIN_SLIDER_LENGTH	5

/*
 *--------------------------------------------------------------
 *
 * DisplayScrollbar --
 *
 *	This procedure redraws the contents of a scrollbar window.
 *	It is invoked as a do-when-idle handler, so it only runs
 *	when there's nothing else for the application to do.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Information appears on the screen.
 *
 *--------------------------------------------------------------
 */

void DisplayScrollbar (Scrollbar * scrollPtr)
{
	register        Window tkwin = scrollPtr -> win;
	XPoint points[7];
	Border *border;
	int     relief,
	        width,
	        elementBorderWidth;
	Pixmap pixmap;
	int     wwidth,
	        wheight;

	if (scrollPtr -> vertical)
	{
		wheight = scrollPtr -> height;
		wwidth = SCROLLBAR_WIDTH;
		width = Tk_Width (tkwin) - 2 * scrollPtr -> inset;
	}
	else
	{
		wheight = SCROLLBAR_WIDTH;
		wwidth = scrollPtr -> height;
		width = Tk_Height (tkwin) - 2 * scrollPtr -> inset;
	}
	elementBorderWidth = scrollPtr -> elementBorderWidth;
	if (elementBorderWidth < 0)
	{
		elementBorderWidth = scrollPtr -> borderWidth;
	}

 /* 
  * In order to avoid screen flashes, this procedure redraws
  * the scrollbar in a pixmap, then copies the pixmap to the
  * screen in a single operation.  This means that there's no
  * point in time where the on-sreen image has been cleared.
  */

	pixmap = XCreatePixmap (display, Tk_WindowId (tkwin),
			Tk_Width (tkwin), Tk_Height (tkwin), Tk_Depth (tkwin));

	Tk_Draw3DRectangle (tkwin, pixmap, scrollPtr -> trough,
			scrollPtr -> highlightWidth, scrollPtr -> highlightWidth,
			Tk_Width (tkwin) - 2 * scrollPtr -> highlightWidth,
			Tk_Height (tkwin) - 2 * scrollPtr -> highlightWidth,
			scrollPtr -> borderWidth, TK_RELIEF_SUNKEN);
	XFillRectangle (display, pixmap, scrollPtr -> trough->bgGC,
			scrollPtr -> inset, scrollPtr -> inset,
			(unsigned) (Tk_Width (tkwin) - 2 * scrollPtr -> inset),
			(unsigned) (Tk_Height (tkwin) - 2 * scrollPtr -> inset));

 /* 
  * Draw the top or left arrow.  The coordinates of the polygon
  * points probably seem odd, but they were carefully chosen with
  * respect to X's rules for filling polygons.  These point choices
  * cause the arrows to just fill the narrow dimension of the
  * scrollbar and be properly centered.
  */

	relief = scrollPtr -> activeField == TOP_ARROW ?
		TK_RELIEF_SUNKEN : TK_RELIEF_RAISED;
	border = scrollPtr -> bgBorder;

	if (scrollPtr -> vertical)
	{
		points[0].x = scrollPtr -> inset - 1;
		points[0].y = scrollPtr -> arrowLength + scrollPtr -> inset - 1;
		points[1].x = width + scrollPtr -> inset;
		points[1].y = points[0].y;
		points[2].x = width / 2 + scrollPtr -> inset;
		points[2].y = scrollPtr -> inset - 1;
		Tk_Fill3DPolygon (tkwin, pixmap, border, points, 3,
				elementBorderWidth, relief);
	}
	else
	{
		points[0].x = scrollPtr -> arrowLength + scrollPtr -> inset - 1;
		points[0].y = scrollPtr -> inset - 1;
		points[1].x = scrollPtr -> inset;
		points[1].y = width / 2 + scrollPtr -> inset;
		points[2].x = points[0].x;
		points[2].y = width + scrollPtr -> inset;
		Tk_Fill3DPolygon (tkwin, pixmap, border, points, 3,
				elementBorderWidth, relief);
	}

 /* 
  * Display the bottom or right arrow.
  */

	relief = scrollPtr -> activeField == BOTTOM_ARROW ?
		TK_RELIEF_SUNKEN : TK_RELIEF_RAISED;
	border = scrollPtr -> bgBorder;

	if (scrollPtr -> vertical)
	{
		points[0].x = scrollPtr -> inset;
		points[0].y = Tk_Height (tkwin) - scrollPtr -> arrowLength
			- scrollPtr -> inset + 1;
		points[1].x = width / 2 + scrollPtr -> inset;
		points[1].y = Tk_Height (tkwin) - scrollPtr -> inset;
		points[2].x = width + scrollPtr -> inset;
		points[2].y = points[0].y;
		Tk_Fill3DPolygon (tkwin, pixmap, border,
				points, 3, elementBorderWidth, relief);
	}
	else
	{
		points[0].x = Tk_Width (tkwin) - scrollPtr -> arrowLength
			- scrollPtr -> inset + 1;
		points[0].y = scrollPtr -> inset - 1;
		points[1].x = points[0].x;
		points[1].y = width + scrollPtr -> inset;
		points[2].x = Tk_Width (tkwin) - scrollPtr -> inset;
		points[2].y = width / 2 + scrollPtr -> inset;
		Tk_Fill3DPolygon (tkwin, pixmap, border,
				points, 3, elementBorderWidth, relief);
	}

 /* 
  * Display the slider.
  */

	relief = scrollPtr -> activeField == SLIDER ?
		TK_RELIEF_SUNKEN : TK_RELIEF_RAISED;
	border = scrollPtr -> bgBorder;

	if (scrollPtr -> vertical)
	{
		Tk_Fill3DRectangle (tkwin, pixmap, border,
				scrollPtr -> inset, scrollPtr -> sliderFirst,
				width, scrollPtr -> sliderLast - scrollPtr -> sliderFirst,
				elementBorderWidth, relief);
	}
	else
	{
		Tk_Fill3DRectangle (tkwin, pixmap, border,
				scrollPtr -> sliderFirst, scrollPtr -> inset,
				scrollPtr -> sliderLast - scrollPtr -> sliderFirst, width,
				elementBorderWidth, relief);
	}

 /* 
  * Copy the information from the off-screen pixmap onto the screen,
  * then delete the pixmap.
  */

	XCopyArea (display, pixmap, Tk_WindowId (tkwin),
			scrollPtr -> copyGC, 0, 0, (unsigned) Tk_Width (tkwin),
			(unsigned) Tk_Height (tkwin), 0, 0);
	XFreePixmap (display, pixmap);
}


/*
 *----------------------------------------------------------------------
 *
 * ComputeScrollbarGeometry --
 *
 *	After changes in a scrollbar's size or configuration, this
 *	procedure recomputes various geometry information used in
 *	displaying the scrollbar.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The scrollbar will be displayed differently.
 *
 *----------------------------------------------------------------------
 */

void ComputeScrollbarGeometry (scrollPtr)
register        Scrollbar * scrollPtr;
{
	int     width,
	        fieldLength;
	int     wwidth,
	        wheight;

	if (scrollPtr -> vertical)
	{
		wheight = scrollPtr -> height;
		wwidth = SCROLLBAR_WIDTH;
	}
	else
	{
		wheight = SCROLLBAR_WIDTH;
		wwidth = scrollPtr -> height;
	}

	if (scrollPtr -> highlightWidth < 0)
	{
		scrollPtr -> highlightWidth = 0;
	}
	scrollPtr -> inset = scrollPtr -> highlightWidth + scrollPtr -> borderWidth;
	width = (scrollPtr -> vertical) ? Tk_Width (scrollPtr -> tkwin)
		: Tk_Height (scrollPtr -> tkwin);
	scrollPtr -> arrowLength = width - 2 * scrollPtr -> inset + 1;
	fieldLength = (scrollPtr -> vertical ? Tk_Height (scrollPtr -> tkwin)
			: Tk_Width (scrollPtr -> tkwin))
		- 2 * (scrollPtr -> arrowLength + scrollPtr -> inset);
	if (fieldLength < 0)
	{
		fieldLength = 0;
	}
	scrollPtr -> sliderFirst = fieldLength * scrollPtr -> firstFraction;
	scrollPtr -> sliderLast = fieldLength * scrollPtr -> lastFraction;

 /* 
  * Adjust the slider so that some piece of it is always
  * displayed in the scrollbar and so that it has at least
  * a minimal width (so it can be grabbed with the mouse).
  */

	if (scrollPtr -> sliderFirst > (fieldLength - 2 * scrollPtr -> borderWidth))
	{
		scrollPtr -> sliderFirst = fieldLength - 2 * scrollPtr -> borderWidth;
	}
	if (scrollPtr -> sliderFirst < 0)
	{
		scrollPtr -> sliderFirst = 0;
	}
	if (scrollPtr -> sliderLast < (scrollPtr -> sliderFirst
				+ MIN_SLIDER_LENGTH))
	{
		scrollPtr -> sliderLast = scrollPtr -> sliderFirst + MIN_SLIDER_LENGTH;
	}
	if (scrollPtr -> sliderLast > fieldLength)
	{
		scrollPtr -> sliderLast = fieldLength;
	}
	scrollPtr -> sliderFirst += scrollPtr -> arrowLength + scrollPtr -> inset;
	scrollPtr -> sliderLast += scrollPtr -> arrowLength + scrollPtr -> inset;

}

/*
 *--------------------------------------------------------------
 *
 * ScrollbarPosition --
 *
 *	Determine the scrollbar element corresponding to a
 *	given position.
 *
 * Results:
 *	One of TOP_ARROW, TOP_GAP, etc., indicating which element
 *	of the scrollbar covers the position given by (x, y).  If
 *	(x,y) is outside the scrollbar entirely, then OUTSIDE is
 *	returned.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

int
                ScrollbarPosition (scrollPtr, x, y)
register        Scrollbar * scrollPtr;/* Scrollbar widget record. */
int     x,
        y;			/* Coordinates within scrollPtr's *
				   window. */
{
	int     length,
	        width;

	length = scrollPtr->height;
	width = SCROLLBAR_WIDTH;

	if (scrollPtr -> vertical == 0)
	{
		int tmp = x;
		x = y;
		y = tmp;
	}

	if ((x < scrollPtr -> inset) || (x >= (width - scrollPtr -> inset))
			|| (y < scrollPtr -> inset) || (y >= (length - scrollPtr -> inset)))
	{
		return OUTSIDE;
	}

 /* 
  * All of the calculations in this procedure mirror those in
  * DisplayScrollbar.  Be sure to keep the two consistent.
  */

	if (y < (scrollPtr -> inset + scrollPtr -> arrowLength))
	{
		return TOP_ARROW;
	}
	if (y < scrollPtr -> sliderFirst)
	{
		return TOP_GAP;
	}
	if (y < scrollPtr -> sliderLast)
	{
		return SLIDER;
	}
	if (y >= (length - (scrollPtr -> arrowLength + scrollPtr -> inset)))
	{
		return BOTTOM_ARROW;
	}
	return BOTTOM_GAP;
}

#endif
