#include "z.h"

#if XWINDOWS
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "xwind.h"

static int GetWindowPosition ARGS((Window window,
		int *absx, int *absy, int *relx, int *rely));

/* Calculate the height lost by the WM decoarations.
 * We assume the window is layed out with a header on top and
 * a uniform border on right, left, and bottom
 */
#define WM_WASTE	((absy - rely) + (absx - relx))

Proc Zzoom ()
{
	int absx, absy, relx, rely;
	/* keep track of the last unzoomed position */
	static int was_x, was_y, was_w, was_h = 0;

	GetWindowPosition(Zroot, &absx, &absy, &relx, &rely);

	if (was_h == 0)
	{	/* zoom */
		was_x = relx;
		was_y = rely;
		was_w = win_width;
		was_h = win_height;

		win_height = DisplayHeight(display, 0) - WM_WASTE;
		rely = 0;
	}
	else
	{	/* unzoom */
		relx = was_x;
		rely = was_y;
		win_width = was_w;
		win_height = was_h;
		was_h = 0;	/* mark as unzoomed */
	}

	/* move/resize/redisplay */
	XMoveResizeWindow (display, Zroot, relx, rely, win_width, win_height);
	Zredisplay ();
}

/* This routine returns the absolure and relative positions of the window.
 * The absolute is the top-left corner of the Zroot window.
 * The relative is the top-left corner including the WM decorations.
 * A negative value means the window is off the screen.
 *
 * This routine will display an error message if it fails and return 0.
 */
static int GetWindowPosition (window, absx, absy, relx, rely)
Window window;
int *absx, *absy, *relx, *rely;
{
	XWindowAttributes win_attr;
	XVisualInfo vistemplate, *vinfo;
	XSizeHints hints;
	int rx, ry, xright, ybelow;
	Window wmframe;
	int  junk;
	long longjunk;
	Window junkwin;
	int dw = DisplayWidth (display, screen);
	int dh = DisplayHeight(display, screen);

	if(!XGetWindowAttributes (display, window, &win_attr))
	{
		Error ("Can't get window attributes.");
		return 0;
	}
	vistemplate.visualid = XVisualIDFromVisual (win_attr.visual);
	vinfo = XGetVisualInfo (display, VisualIDMask, &vistemplate, &junk);
	XTranslateCoordinates (display, window, win_attr.root,
			-win_attr.border_width,
			-win_attr.border_width,
			&rx, &ry, &junkwin);

	xright = dw - rx - win_attr.border_width * 2 - win_attr.width;
	ybelow = dh - ry - win_attr.border_width * 2 - win_attr.height;

	/* save the absolute values */
	*absx = rx;
	*absy = ry;

	XGetWMNormalHints (display, window, &hints, &longjunk);

	/* Note: Zedit uses NorthWestGravity or SouthWestGravity */
	if (!(hints.flags & PWinGravity))
		hints.win_gravity = NorthWestGravity;/* per ICCCM */

	/* find our window manager frame, if any */
	wmframe = window;
	while(True)
	{
		Window root, parent;
		Window * childlist;
		unsigned int ujunk;

		if(!XQueryTree(display, wmframe, &root, &parent, &childlist,
			&ujunk) || parent == root || !parent)
				break;
		wmframe = parent;
		if(childlist)
			XFree ((char *) childlist);
	}
	if (wmframe != window)
	{	/* WM reparented, so find edges of the frame. *
		 * Only works for ICCCM-compliant WMs, and then only if the
		 * window has corner gravity.  We would need to know the
		 * original width of the window to correctly handle the other
		 * gravities. */
		XWindowAttributes frame_attr;

		if (!XGetWindowAttributes (display, wmframe, &frame_attr))
		{
			Error("Can't get frame attributes.");
			return 0;
		}
		switch (hints.win_gravity)
		{
			case NorthWestGravity: 
			case SouthWestGravity: 
			case NorthEastGravity: 
			case SouthEastGravity: 
			case WestGravity: 
				rx = frame_attr.x;
		}
		switch (hints.win_gravity)
		{
			case NorthWestGravity: 
			case SouthWestGravity: 
			case NorthEastGravity: 
			case SouthEastGravity: 
			case NorthGravity: 
				ry = frame_attr.y;
		}
	}

	*relx = rx;
	*rely = ry;
	return 1;
}
#endif
