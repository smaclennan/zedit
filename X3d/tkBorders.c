#ifdef BORDER3D

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../z.h"
#include "xwind.h"

Border Zborder;
int ScrollBarWidth = 13;
int SBborderwidth = 1, SBhighlight = 3;

static Scrollbar Vscroll, Hscroll;

/* Called from Tinit to draw borders the first time */
void BorderInit()
{
	/* SAM This really needs to be fixed up, we have
	 * already allocated these colors only by pixel value!
	 *
	 * GetColor and and GetXColor should keep track of already
	 * "got" colors!
	 */
	static XColor ccolor;
	char *color;
	XGCValues values;
	int mask = GCFont;

	if((color = GetResource(".background", ".background")) &&
		GetXColor(color, &ccolor))
	{
		values.foreground = ccolor.pixel;
		mask |= GCForeground;

		/* default these in case .modelinefg not set */
		values.background = ccolor.pixel;
		mask |= GCBackground;
	 	Zborder.bgColorPtr = &ccolor;
	}
	if((color = GetResource(".modelinefg", ".modelinefg")) &&
		GetXColor(color, &ccolor))
	{
		values.background = ccolor.pixel;
		mask |= GCBackground;
	 	Zborder.bgColorPtr = &ccolor;
	}


	Zborder.darkGC = Zborder.lightGC = None;
	Zborder.shadow = None;
	values.font = fontid;
	Zborder.bgGC = XCreateGC(display, Zroot, mask, &values);
	
	CreateScrollBars();
}

	
/* Window Layout with all frames.

  +--------------------------------+
  |+------------------------------+|
  ||      Menu Bar                ||
  |+------------------------------+|
  +---------------------------+-+--+
  |                           | |  |
  |                           | |  |
  |                           | |V |
  |                           | |s |
  |     Text                  | |c |
  |     (textwindow)          | |r |
  |                           | |o |
  |                           | |l |
  |                           | |l |
  |                           | |  |
  +---------------------------+ +--+
  +---------------------------+
  |          Hscroll          |
  +---------------------------+----+
  |+------------------------------+|
  ||      Modeline                ||
  |+------------------------------+|
  +--------------------------------+
  |+------------------------------+|
  ||      PAW (PAWwindow)		  ||
  |+------------------------------+|
  +--------------------------------+
  
  Notes:
  	0.	All frames are of size border_width.
	1.  Text window has a one pixel border.
	2.	Padding to left of Vscroll and
		above Hscroll.
 */

void DrawBorders()
{
	extern int win_width, win_height;
	extern size_t Rowmax;
	int x, y;
	int width, height;

	/* Draw the menu bar */
	DrawMenuBar(0, 0, win_width, H_MENUBAR);

	/* Now start at the bottom and go up.
	 * First (last?) the PAW.
	 */
	x = 0;
	y = win_height - H_PAW;
	height = H_PAW;
	width = win_width;
#if 1
	Tk_Draw3DRectangle (Zroot, Zroot, &Zborder,
			x, y,
			width, height,
			border_width, TK_RELIEF_SUNKEN);
#endif

#if 1
	/* Now the horizontal scrollbar */
	DisplayScrollbar(&Hscroll);
#endif

#if 1
	/* Put in the vertical scrollbar */
	DisplayScrollbar(&Vscroll);
#endif
}

void Termsize()
{
	extern int SBhighlight;
	static int cur_width = 0, cur_height = 0;
	int width, height;

	/* calculate the size of the text window */
	height = win_height - H_MENUBAR - 2 - SBhighlight - SCROLLBAR_WIDTH -
		H_MODELINE - H_PAW;
	width = win_width - SCROLLBAR_WIDTH - SBhighlight - 2;
	Rowmax = height / fontheight + 2;
	Colmax = width / fontwidth;

	/* now resize window to fit rows and cols exactly */
	win_height -= height - Xrow[Rowmax - 2];
	win_width  -= width - Xcol[Colmax];

	/* This test is very important under Solaris CDE 1.0.
	 * Most X implementations (including OpenWindows) will recognize
	 * we are not really changing the window size but CDE dosen't.
	 */
	if(win_height == cur_height && win_width == cur_width) return;
	cur_height = win_height;
	cur_width  = win_width;

	XResizeWindow(display, Zroot, win_width, win_height);
	
	XResizeWindow(display, textwindow, Xcol[Colmax], Xrow[Rowmax - 2]);

	XMoveResizeWindow(display, PAWwindow,
		border_width, win_height - fontheight - border_width,
		win_width - border_width * 2, fontheight);

	Vscroll.height = Xrow[Rowmax - 2] + 2;
	XMoveResizeWindow(display, Vscroll.win,
		win_width - SCROLLBAR_WIDTH, H_MENUBAR,
		SCROLLBAR_WIDTH, Vscroll.height);

	Hscroll.height = Xcol[Colmax] + 2;
	XMoveResizeWindow(display, Hscroll.win,
		0, win_height - H_PAW - H_MODELINE - SCROLLBAR_WIDTH,
		Hscroll.height, SCROLLBAR_WIDTH);

	ComputeScrollbarGeometry(&Vscroll);
	ComputeScrollbarGeometry(&Hscroll);

	DisplayScrollbar(&Vscroll);
	DisplayScrollbar(&Hscroll);

	DrawBorders();
}

static Border *CreateBorder(Window win, char *resource, Border *bdefault);

void CreateScrollBars()
{
	extern Border Zborder;
	int x, y;
	XGCValues gcValues;
	char *env;

	Vscroll.vertical = 1;
	Vscroll.borderWidth    = SBborderwidth;
	Vscroll.highlightWidth = 0;
	Vscroll.height = win_height - H_MENUBAR - H_MODELINE - H_PAW - SBhighlight;
	Vscroll.activeField = OUTSIDE;
	Vscroll.elementBorderWidth = -1;
	Vscroll.firstFraction = 0.0;
	Vscroll.lastFraction  = 1.0;

	x = win_width - SCROLLBAR_WIDTH;
	y = H_MENUBAR;
	Vscroll.win = CreateWindow(Zroot, x, y,
			SCROLLBAR_WIDTH, Vscroll.height,
			ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

	Vscroll.bgBorder = CreateBorder(Vscroll.win, ".thumbColor",  &Zborder);
	Vscroll.trough   = CreateBorder(Vscroll.win, ".troughColor", &Zborder);

	gcValues.graphics_exposures = False;
	Vscroll.copyGC = XCreateGC(display, Vscroll.win, GCGraphicsExposures,
			&gcValues);

	Hscroll.vertical = 0;
	Hscroll.borderWidth    = SBborderwidth;
	Hscroll.highlightWidth = 0;
	Hscroll.height = win_width - SCROLLBAR_WIDTH - SBhighlight;
	Hscroll.activeField = OUTSIDE;
	Hscroll.elementBorderWidth = -1;
	Hscroll.firstFraction = 0.0;
	Hscroll.lastFraction  = 1.0;
	x = 0;
	y = win_height - H_PAW - H_MODELINE - SCROLLBAR_WIDTH;
	Hscroll.win = CreateWindow(Zroot, x, y,
			Hscroll.height, SCROLLBAR_WIDTH,
			ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

	Hscroll.bgBorder = CreateBorder(Hscroll.win, ".thumbColor",  &Zborder);
	Hscroll.trough   = CreateBorder(Hscroll.win, ".troughColor", &Zborder);

	gcValues.graphics_exposures = False;
	Hscroll.copyGC = XCreateGC(display, Hscroll.win, GCGraphicsExposures,
			&gcValues);

	ComputeScrollbarGeometry(&Vscroll);
	ComputeScrollbarGeometry(&Hscroll);
}


/* Called by refresh() */
void UpdateScrollbars()
{
	extern int Prow;
	long lines = Blines(Curwdo->wbuff);
	long line;
	double first, last;

	if(lines < Curwdo->last)
	{
		first = 0.0;
		last  = 1.0;
	}
	else
	{
		Blocation(&line);
		line -= Prow + 1;
		first = (double)line / (double)lines;
		last  = (double)(line + Curwdo->last) / (double)lines;
	}

	if(first != Vscroll.firstFraction || last != Vscroll.lastFraction)
	{
		Vscroll.firstFraction = first;
		Vscroll.lastFraction  = last;
		ComputeScrollbarGeometry(&Vscroll);
		DisplayScrollbar(&Vscroll);
	}
}


void ScrollEvent(event)
XEvent *event;
{
	struct wdo *wdo;
	Window window = event->xany.window;

	if(event->type != ButtonPress) return;

	if(window == Vscroll.win)
		VscrollEvent(event);
#ifdef HSCROLL
	else if(window == Hscroll.win)
		HscrollEvent(event);
#endif
}

/* We received an event in a scroll window */
void VscrollEvent(XEvent *event)
{
	extern int Arg, Argp;
	long lines;

	if(event->type != ButtonPress || event->xany.window != Vscroll.win) return;

	Vscroll.activeField = ScrollbarPosition(&Vscroll,
								event->xbutton.x, event->xbutton.y);
	lines = Blines(Curbuff);
	ShowCursor(FALSE);
	DisplayScrollbar(&Vscroll);

	/* Process Motion events until ButtonRelease */
	do
	{
		XNextEvent(display, event);
		if(event->type == MotionNotify)
		{
			while(XCheckMaskEvent(display, PointerMotionMask, event)) ;
			if(Vscroll.activeField == SLIDER)
			{
				double percent;

				percent =	(double)event->xmotion.y /
							(double)Vscroll.height;

				Argp = 1;
				Arg = (int)(percent * (double)lines);

				Zlgoto();
				Refresh();
				DisplayScrollbar(&Vscroll);
			}
		}
	}
	while(event->type != ButtonRelease);

	switch(ScrollbarPosition(&Vscroll, event->xbutton.x, event->xbutton.y))
	{
	case TOP_ARROW:
		if(Vscroll.activeField == TOP_ARROW) Zprevpage();
		break;
	case BOTTOM_ARROW:
		if(Vscroll.activeField == BOTTOM_ARROW) Znextpage();
		break;
	}

	Vscroll.activeField = OUTSIDE;
	Refresh();
	ShowCursor(TRUE);
	DisplayScrollbar(&Vscroll);
}

int Hshift = 0;

/* We received a ButtonPress event in a hscroll window */
void HscrollEvent(event, wdo)
XEvent *event;
struct wdo *wdo;
{
	do
	{
		XNextEvent(display, event);
		if(event->type == MotionNotify)
		{
			while(XCheckMaskEvent(display, PointerMotionMask, event)) ;
		}
	}
	while(event->type != ButtonRelease);
	
	Hshift += 10;		/* SAM HACK */
}

static Border *CreateBorder(Window win, char *resource, Border *bdefault)
{
	char *color;
	XColor ccolor;

	if((color = GetResource(resource, resource)) &&
		GetXColor(color, &ccolor))
	{	/* user specified a valid color - use it! */
		Border *border;
	    XGCValues gcValues;
			
		if((border = (Border *)malloc(sizeof(Border))) == 0 ||
		   (border->bgColorPtr = (XColor *)malloc(sizeof(XColor))) == 0)
		{	/* no memory! */
			if(border) free(border);
printf("No memory, DEFAULT BORDER\n");/*SAM*/
			return bdefault;
		}

	 	memcpy(border->bgColorPtr, &ccolor, sizeof(XColor));

		gcValues.foreground = ccolor.pixel;
		border->bgGC=XCreateGC(display, win, GCForeground, &gcValues);

		border->darkGC = border->lightGC = None;
		border->shadow = None;

		return border;
	}

printf("DEFAULT BORDER\n");	/*SAM*/
	return bdefault;
}

#endif
