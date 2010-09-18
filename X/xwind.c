#include "../z.h"

#if XWINDOWS
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include "xcursors.h"

#define zedit_width 64
#define zedit_height 64
static char zedit_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xf8, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x1f, 0x38, 0x08, 0x31, 0x82, 0xff, 0xff, 0xff, 0x1f,
   0xf8, 0xed, 0x6d, 0xef, 0xff, 0xff, 0xff, 0x1f, 0xf8, 0xce, 0x6d, 0xef,
   0xff, 0xff, 0xff, 0x1f, 0x78, 0xef, 0x6d, 0xef, 0xff, 0xff, 0xff, 0x1f,
   0x38, 0x08, 0x31, 0xee, 0xff, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x1f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#include "xwind.h"
#include "../keys.h"

#if XWINDOWS
char *KeyNames[] = {
	"Home", "Left", "Up", "Right", "Down", "Prior",
	"Next", "End", "Begin", "Select", "Print", "Execute",
	"Insert", "???", "Undo", "Redo", "Menu", "Find",
 	"Cancel", "Help", "Break",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
 	"F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17",
	"F18", "F19", "F20", "F21", "F22", "F23", "F24", "F25",
	"F26", "F27", "F28", "F29", "F30", "F31", "F32", "F33",
 	"F34", "F35", "Ctrl Home","Ctrl Left","Ctrl Up", "Ctrl Right",
 	"Ctrl Down","Ctrl Prior", "Ctrl Next","Ctrl End",	"Ctrl Begin"
};
#endif

#define DOUBLE_CLICK		500		/* 1/2 second */

void processpipe(XClientMessageEvent *event);
static Window CreateRootWindow(int argc, char **argv, int *, int *);
static void GetGeometry(XSizeHints *sh, int bw);
static void ExposeWindow(), SetupSpecial();
static void SetTimer(XEvent *event, unsigned timeout);

static void Paste(Atom paste);

/* set in Xinit */
extern Display *display;
extern int screen;
extern  int fontheight, fontwidth, fontbase, fontid, boldid;

Window Zroot, zwindow;
#ifdef BORDER3D
Window textwindow, PAWwindow;
GC PAWgc;
#endif

/* Note: LoadFontByName needs to know all the gcs */
GC curgc;
GC normgc, revgc, boldgc, commentgc, cppgc, cppifgc;
GC cursorgc, markgc;
GC modegc;

Cursor textcursor, vscrollcursor, hscrollcursor;

int win_width = 0, win_height = 0;
int border_width = 2, highlight = 2;

Boolean HasColor = FALSE;
int foreground, background;
#ifdef BORDER3D
static int oldfg, oldbg;
#endif

Atom WM_PROTOCOLS;			/* message sent by window manager (twm) */
Atom WM_DELETE_WINDOW;		/* subtype of WM_PROTOCOLS */

char *Term;

void Tinit(argc, argv)
int argc;
char **argv;
{
	extern char *getenv();
	XGCValues values;
	uint valuemask;
	int ccolor;
	int width, height;
#ifdef BORDER3D
	char *env;
#endif
	XEvent event;

	if((Term = getenv("TERM")) == NULL) Term = "xterm";

	screen = DefaultScreen(display);
	
	HasColor = DisplayPlanes(display, screen) > 7;

	/* For the 3d version of Zedit, the foreground and background
	 * are for the general windows and text.* controls the text
	 * window.
	 * To make Zedit and Zedit3D compatible, Zedit will first check
	 * for the .text.* values and then the normal ones.
	 */
	foreground = BlackPixel(display, screen);
	background = WhitePixel(display, screen);
#ifdef BORDER3D
	if(HasColor)
	{
		ColorResource(".foreground", ".Foreground", &foreground);
		ColorResource(".background", ".Background", &background);
	}
#else
	if(HasColor)
	{	/* override with these values if available */
		if(!ColorResource(".text.foreground", ".text.Foreground", &foreground))
			ColorResource(".foreground", ".Foreground", &foreground);
		if(!ColorResource(".text.background", ".text.Background", &background))
			ColorResource(".background", ".Background", &background);
	}
#endif

	LoadFonts();

#ifdef BORDER3D
	/* These are needed in CreateRootWindow to correctly size the window. */
	if((env = GetResource(".scrollbar.width", ".scrollbar.Width")))
		ScrollBarWidth = atoi(env);
	if((env = GetResource(".scrollbar.borderwidth", ".scrollbar.BorderWidth")))
		SBborderwidth = atoi(env);
	if((env = GetResource(".scrollbar.highlight", ".scrollbar.Highlight")))
		SBhighlight = atoi(env);
printf("Scrollbar width %d border %d highlight  %d\n",
	ScrollBarWidth, SBborderwidth, SBhighlight);
#endif

	/* Create root window */
	CreateRootWindow(argc, argv, &width, &height);

#ifdef BORDER3D
	PAWwindow  = CreateWindow(Zroot,
					border_width, Y_PAW(height),
					width, fontheight,
					KeyPressMask    | KeyReleaseMask    |
					ButtonPressMask | ButtonReleaseMask |
					PointerMotionMask);

	/* GC for paw text */
	values.font = fontid;
	values.foreground = foreground;
	values.background = background;
	valuemask = GCForeground | GCBackground | GCFont;
	PAWgc = XCreateGC(display, PAWwindow, valuemask, &values);
	
	/* For the text window and text GCs we use text.* as fore/background */
	oldbg = background;
	oldfg = foreground;
	if(HasColor)
	{
		ColorResource(".text.foreground", ".text.Foreground", &foreground);
		ColorResource(".text.background", ".text.Background", &background);
	}

	textwindow = zwindow = CreateWindow(Zroot, 0, H_MENUBAR, width, height,
			KeyPressMask    | KeyReleaseMask    |
			ButtonPressMask | ButtonReleaseMask |
			PointerMotionMask);

	XSetWindowBorderWidth(display, textwindow, 1);
	XSetWindowBorder(display, textwindow, oldfg);

	InitMenuBar();
#else
	/* only root window exists */
	zwindow = Zroot;
#endif

	/* GC for reverse text */
	values.font = fontid;
	values.foreground = background;
	values.background = foreground;
	valuemask = GCForeground | GCBackground | GCFont;
	revgc = XCreateGC(display, zwindow, valuemask, &values);
	
	/* GC for normal text */
	values.foreground = foreground;
	values.background = background;
	curgc = normgc = XCreateGC(display, zwindow, valuemask, &values);

	/* GC for bold text */
	if(HasColor && ColorResource(".boldColor", ".boldColor", &ccolor))
	{	/* user specified bold color */
		values.background = ccolor;
		boldgc = XCreateGC(display, zwindow, valuemask, &values);
	}
	else if(boldid)
	{	/* user specified bold font */
		values.font = boldid;
		boldgc = XCreateGC(display, zwindow, valuemask, &values);
		values.font = fontid;
	}
	else boldgc = normgc;

	/* create GC for cursor */
	if(HasColor)
	{	/* try to make a color cursor */
		if(ColorResource(".cursorColor", ".CursorColor", &ccolor))
		{
			values.background = ccolor;
			values.foreground = foreground;
		}
		else
		{
			values.background = foreground;
			values.foreground = background;
		}
		values.function = GXcopy;
	}
	else
		/* just invert the character */
		values.function = GXinvert;
	valuemask |= GCFunction;
	cursorgc = XCreateGC(display, zwindow, valuemask, &values);

	/* create GC for mark */
	if(HasColor)
	{
		if(ColorResource(".markColor", ".markColor", &ccolor))
		{
			values.background = ccolor;
			values.foreground = foreground;
			values.function = GXcopy;
			markgc = XCreateGC(display, zwindow, valuemask, &values);
		}
		else markgc = revgc;
	}
	else markgc = normgc;
	
	/* Allow user to set comment color but default to blue if possible. */
	if(HasColor &&
		(ColorResource(".commentColor", ".CommentColor", &ccolor) ||
		 GetColor("blue", &ccolor)))
	{
		values.foreground = ccolor;
		values.background = background;
		commentgc = XCreateGC(display, zwindow, valuemask, &values);
	}
	else commentgc = boldgc;

	/* Allow user to set cpp color but default to green if possible. */
	if(HasColor &&
		(ColorResource(".cppColor", ".CppColor", &ccolor) ||
		 GetColor("green", &ccolor)))
	{
		values.foreground = ccolor;
		values.background = background;
		cppgc = XCreateGC(display, zwindow, valuemask, &values);
	}
	else cppgc = commentgc;

	/* Allow user to set cpp color but default to green if possible. */
	if(ColorResource(".cppifColor", ".CppifColor", &ccolor))
	{
		values.foreground = ccolor;
		values.background = background;
		cppifgc = XCreateGC(display, zwindow, valuemask, &values);
	}
	else cppifgc = cppgc;

	/* create GC for modeline - do this last */
	if(HasColor)
	{	/* default to reverse */
		if(ColorResource(".modelinebg", ".modelineBG", &ccolor))
			values.background = ccolor;
		else
			values.background = foreground;
		if(ColorResource(".modelinefg", ".modelingFG", &ccolor))
			values.foreground = ccolor;
		else
			values.foreground = background;
		
		modegc = XCreateGC(display, zwindow, valuemask, &values);
	}
	else modegc = revgc;

	/* We wait for exposure so draw commands will work.
	 * We wait for StructureNotify so we can get real window size.
	 */
	do
	{
		XMaskEvent(display, ExposureMask|StructureNotifyMask, &event);
		if(event.type == ConfigureNotify)
		{	/* window has been resized */
			win_width  = event.xconfigure.width;
			win_height = event.xconfigure.height;
		}
	}
	while(event.type != Expose || win_width == 0);
	
	Initline();		/* Curwdo not defined yet */

	Srow = Scol = -1;	/* undefined */
	Pcol = Prow = 0;	/* start 'em off */

#ifdef BORDER3D
	/* Draw the border around the window */
	BorderInit();
#endif

#if 1
	Termsize();
#endif
}


/* Create the root window for the editor.
 * Note that width and height are the sizes
 * specified by the user for the source window.
 */
static Window CreateRootWindow(int argc, char **argv, int *width, int *height)
{
	XSizeHints size_hints;
	XWMHints wm_hints;
	XClassHint class_hints;
	XSetWindowAttributes attr;
	char *resource;
	char *name = ZSTR;
	XTextProperty tpname;
	int i;

	attr.background_pixel = background;
	attr.event_mask =
			KeyPressMask | KeyReleaseMask | FocusChangeMask |
			ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
			ExposureMask | StructureNotifyMask;
	attr.override_redirect = False;

	GetGeometry(&size_hints, border_width);

	*width  = size_hints.width;
	*height = size_hints.height;
#ifdef BORDER3D
	size_hints.width += SCROLLBAR_WIDTH + SBhighlight + 2;
	size_hints.height += H_MENUBAR + H_PAW + H_MODELINE +
		SCROLLBAR_WIDTH + SBhighlight + 2;
#elif defined(SCROLLBARS)
	size_hints.width += SCROLLBAR_WIDTH;
#  ifdef HSCROLL
	size_hints.height += SCROLLBAR_WIDTH;
#  endif
#endif
	Zroot = XCreateWindow(display,  RootWindow(display, screen),
		size_hints.x, size_hints.y, size_hints.width, size_hints.height,
#ifdef BORDER3D
		0, CopyFromParent, InputOutput, CopyFromParent,
#else
		border_width, CopyFromParent, InputOutput, CopyFromParent,
#endif
		CWBackPixel | CWEventMask | CWOverrideRedirect, &attr);

	wm_hints.flags = InputHint | StateHint | IconPixmapHint;
	wm_hints.input = True;		/* Allow keyboard input. */

	if((resource = GetResource(".iconic", ".Iconic")) != NULL)
		wm_hints.initial_state = IconicState;
	else
		wm_hints.initial_state = NormalState;
	wm_hints.icon_pixmap = XCreateBitmapFromData(display, Zroot, zedit_bits,
		zedit_width, zedit_height);

	if((resource = GetResource("*icongeometry", "*iconGeometry")) != NULL)
	{
		int flags, dummy;
		flags = XParseGeometry(resource, &wm_hints.icon_x, &wm_hints.icon_y,
			&dummy, &dummy);
		if(flags & (XValue | YValue)) wm_hints.flags |= IconPositionHint;
	}

	class_hints.res_name = ZSTR;
	class_hints.res_class = ZSTR;

	if(XStringListToTextProperty(&name, 1, &tpname) == 0)
	{
		printf("Out of memory.\n");
		exit(1);
	}
	
	XSetWMProperties(display, Zroot, &tpname, &tpname, argv, argc,
		&size_hints, &wm_hints, &class_hints);

	/* read the keyboard modifiers */
	SetupSpecial();

	/* create pointer if necessary */
	if((resource = GetResource(".pointershape", ".pointerShape")) != 0)
		for(i = 0; i < XCURSORS; ++i)
			if(strcmp(resource, Xcursors[i].name) == 0)
			{
				textcursor = XCreateFontCursor(display, Xcursors[i].cursor);
				XDefineCursor(display, Zroot, textcursor);
				break;
			}
	/* create scrollpointer - default to vertical scrollbar double arrow */
	if((resource = GetResource(".vscrollpointer", ".vscrollPointer")) == 0 &&
	   (resource = GetResource(".scrollpointer", ".scrollPointer")) == 0)
			resource = "sb_v_double_arrow";
	for(i = 0; i < XCURSORS; ++i)
		if(strcmp(resource, Xcursors[i].name) == 0)
		{
			vscrollcursor = XCreateFontCursor(display, Xcursors[i].cursor);
			break;
		}
	
	/* create scrollpointer - default to horizontal scrollbar double arrow */
	if((resource = GetResource(".hscrollpointer", ".hscrollPointer")) == 0 &&
	   (resource = GetResource(".scrollpointer", ".scrollPointer")) == 0)
			resource = "sb_h_double_arrow";
	for(i = 0; i < XCURSORS; ++i)
		if(strcmp(resource, Xcursors[i].name) == 0)
		{
			hscrollcursor = XCreateFontCursor(display, Xcursors[i].cursor);
			break;
		}
	
	WM_PROTOCOLS = XInternAtom(display, "WM_PROTOCOLS", True);
    WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(display, Zroot, &WM_DELETE_WINDOW, 1);

	/* get the current keyboard modifiers */
	SetupSpecial();

	/* display window */
	XMapWindow(display, Zroot);
	
	return Zroot;
}


Window CreateWindow(parent, x, y, width, height, events)
Window parent;
int x, y, width, height;
long events;
{
	Window window;
	XSetWindowAttributes attr;

	attr.background_pixel = background;
	attr.event_mask = 	events;
	window = XCreateWindow(display,  parent, x, y, width, height,
#ifdef BORDER3D
		0,
#else
		1,
#endif
		CopyFromParent, InputOutput, CopyFromParent,
		CWBackPixel | CWEventMask, &attr);

	XMapWindow(display, window);

	return window;
}

void Newtitle(title)
char *title;
{
	if(title)
		sprintf(PawStr, "%s  %s", ZSTR, title);
	else
		strcpy(PawStr, ZSTR);
	XStoreName(display, Zroot, PawStr);
}


void Tbell()
{
#define BELLTIME	100
	if(VAR(VSILENT) == 0)
		XBell(display, BELLTIME);
}


void Tfini()
{
	CleanupSocket(-1);
}


void Tclrwind()
{
	XClearWindow(display, zwindow);
}


void Tcleol()
{
#ifndef BORDER3D
	if(Pcol < Clrcol[Prow])
#endif
	{
		XClearArea(display, zwindow, Xcol[Pcol], Xrow[Prow], 0, fontheight, 0);
		Clrcol[Prow] = Pcol;
	}
}

static int s_row = -1, s_col = -1, s_len = 0;
static char outline[COLMAX];		/* buffer of chars */

#ifdef __STDC__
void Tputchar(char c)
#else
void Tputchar(c)
char c;
#endif
{
	if(Prow != s_row || Pcol != s_col)
		Tflush();
	outline[s_len++] = c;
	++s_col;
}

void Tflush()
{
	if(s_len)
	{
		XDrawImageString(display, zwindow, curgc,
			Xcol[s_col - s_len], Xrow[s_row] + fontbase,
			outline, s_len);
		s_len = 0;
	}
	s_row = Prow;
	s_col = Pcol;
}


/* SAM How do we handle T_BOLD/T_NORMAL pairs when in comment? */
void Tstyle(style)
int style;
{
	static int cur_style = -1;
	
	if(style != cur_style)
	{
		Tflush();		/* must flush before switching style */	
		switch(cur_style = style)
		{
			case T_NORMAL: 		curgc = normgc;		break;
			case T_STANDOUT:	curgc = modegc;		break;
			case T_REVERSE:		curgc = revgc;		break;
			case T_BOLD:		curgc = boldgc;		break;
			case T_COMMENT:		curgc = commentgc;	break;
			case T_CPP:			curgc = cppgc;		break;
			case T_CPPIF:		curgc = cppifgc;	break;
		}
	}
}


#ifndef BORDER3D
void Termsize()
{
	XEvent event;

	Rowmax = win_height / fontheight;
# ifdef SCROLLBARS
	Colmax = (win_width - SCROLLBAR_WIDTH) / fontwidth;
# else
	Colmax = win_width / fontwidth;
# endif

#ifdef HSCROLL
	if(Xrow[Rowmax] != win_height + SCROLLBAR_WIDTH + 2)
	{
		win_height = Xrow[Rowmax] + SCROLLBAR_WIDTH + 2;
#else
	if(Xrow[Rowmax] != win_height)
	{
		win_height = Xrow[Rowmax];
#endif
		XResizeWindow(display, zwindow, win_width, win_height);
		XMaskEvent(display, StructureNotifyMask, &event);
	}
}
#endif


Byte Tgetkb()
{
	return (Byte)Tgetcmd();
}


static char *SelectionData = 0;
static int SelectionSize;

static void SetSelection()
{
	extern Buffer *Killbuff;
	Buffer *save = Curbuff;
	register char *p;
	
	Copytomrk(Curbuff->mark);
	SelectionSize = Blength(Killbuff);
	if(SelectionData) free(SelectionData);

	if((SelectionData = p = malloc(SelectionSize + 1)) == 0)
	{
		Error("Not enough memory");
		return;
	}
	
	Bswitchto(Killbuff);
	Btostart();
	for(; !Bisend(); Bmove1()) *p++ = Buff();
	Bswitchto(save);

	XSetSelectionOwner(display, XA_PRIMARY, zwindow, CurrentTime);
	Echo("Copied to clipboard.");
}


Boolean Focus = TRUE;

/* If in modeline, scroll buffer. */
static Boolean PointerScroll(row, down)
int row;
Boolean down;
{
#ifdef BORDER3D
	if(row == Curwdo->last)
	{	/* on a modeline - scroll it */
		down ? Znextpage() : Zprevpage();
		Refresh();
		return TRUE;
	}
#else
	WDO *wdo;

	for(wdo = Whead; wdo; wdo = wdo->next)
		if(row == wdo->last)
		{	/* on a modeline - scroll it */
			WDO *save = Curwdo;
			Wswitchto(wdo);
			down ? Znextpage() : Zprevpage();
			Wswitchto(save);
			Refresh();
			return TRUE;
		}
#endif
	return FALSE;
}

static void MarkToPointer(row, col)
int row, col;
{
	ShowCursor(FALSE);
	if(!PointerScroll(row, FALSE))
	{
		Bswappnt(Curbuff->mark);
		Pntmove(row, col);
		Bswappnt(Curbuff->mark);
	}
	ShowCursor(TRUE);
}

static void PointToPointer(row, col)
int row, col;
{
	ShowCursor(FALSE);
	if(!PointerScroll(row, TRUE))
		Pntmove(row, col);
	ShowCursor(TRUE);
}

/* Initially set in SetupSpecial. Used in Tgetcmd. */
static int Special = 0;
#define Normal	0
#define Meta	1
#define Shift	2
#define Ctrl	4
#define NumLock	8

/* Setup the Specials. We only care about the mask variable. */
static void SetupSpecial()
{
#ifdef sun
	Window root, child;
	int rx, ry, wx, wy;
	unsigned int mask;

	XQueryPointer(display, Zroot, &root, &child, &rx, &ry, &wx, &wy, &mask);
	if(mask & Mod3Mask) Special = NumLock;
#endif
}


#define NO_DRAG			0		/* not dragging */
#define START_DRAG		1		/* set on ButtonPress */
#define DRAGGING		2		/* set on first MotionEvent */
#define DRAGSCROLL		3		/* set if scrolling */
#ifdef sun
#define PAGING			4		/* set for pageup pagedown */
#endif
static int Dragging = NO_DRAG;

static int AlarmFlag = 0;
static int HomeCnt = 0, EndCnt = 0;

/* Dragging timeouts */
#define DRAG_TIMEOUT	100000
#define PAGE_TIMEOUT	150000
#define KEY_TIMEOUT 	999999	/* 1 second - 1 usecond */

int Tgetcmd()	
{
	extern unsigned Cmd;
	XEvent event;
	Byte c;
	KeySym key;
	int row, col;
	Atom paste;
	static int PushRow = 0, PushCol = 0;
	static Time ButtonTime = 0;		/* time of last button click */

	/* process events until keypress */
	ShowCursor(TRUE);	/* only display cursor when waiting */

	for(Cmd = K_NODEF; Cmd == K_NODEF; )
	{
		if(!XPending(display))
			ProcessFDs();

		XNextEvent(display, &event);
#if 0
		printf("Event %s %s\n",
			XWindowName(event.xany.window), XEventName(event.type));
#endif

		/* Events we always handle */
		switch(event.type)
		{
			case Expose:
				while(XCheckTypedEvent(display, Expose, &event));
				ExposeWindow();
				continue;
				
			case ConfigureNotify:
				/* window has been resized */
				win_width  = event.xconfigure.width;
				win_height = event.xconfigure.height;
				Zredisplay();
				ShowCursor(FALSE);
				Refresh();
				ShowCursor(TRUE);
				while(XCheckTypedEvent(display, Expose, &event));
				continue;

			case ClientMessage:
				if(event.xclient.message_type == WM_PROTOCOLS &&
						event.xclient.data.l[0] == WM_DELETE_WINDOW)
				{	/* exit */
					Zexit();
					exit(0);
				}
#if DBG
				else
				{
					int i;
	
					Dbg("GOT UNKNOWN CLIENT MESSAGE: %x\nFORMAT: %d\n\t",
						event.xclient.message_type, event.xclient.format);
					for(i = 0; i < 5; ++i)
						Dbg("%x  ", event.xclient.data.l[i]);
					Dbg("\n");
				}
#endif
				continue;
		}

#ifdef BORDER3D
		if(event.xany.window == Zroot && ProcessMenuBar(&event))
		{
			ExposeWindow();
			continue;
		}
#endif
#ifdef SCROLLBARS
# ifdef BORDER3D
		if(event.xany.window != textwindow &&
		   event.xany.window != PAWwindow  &&
		   event.xany.window != Zroot)
# else
		if(event.xany.window != zwindow)
# endif
		{
			ScrollEvent(&event);
			continue;
		}
#endif
		switch(event.type)
		{
			case ButtonPress:
				/* convert x and y to row and col */
				PushCol = event.xbutton.x / fontwidth;
				PushRow = event.xbutton.y / fontheight;

				/* process the button */
				switch(event.xbutton.button)
				{
					case Button1:
						if(!InPaw &&
							event.xbutton.time < ButtonTime + DOUBLE_CLICK)
						{	/* double click means Find Tag */
							ShowCursor(FALSE);
							Xfindtag();
							ShowCursor(TRUE);
						}
						else
						{	/* first click */
							if(Special == Shift)
								MarkToPointer(PushRow, PushCol);
							else
								PointToPointer(PushRow, PushCol);
							Refresh();
							Dragging = START_DRAG;
						}
						ButtonTime = event.xbutton.time;
						break;
					case Button2:
						if(Special & (Shift | Ctrl))
							/* we want it */
							SetSelection();
						else
						{	/* Paste */
							paste = XInternAtom(display, "PASTEIT", False);
							ShowCursor(FALSE);						
							XConvertSelection(display, XA_PRIMARY, XA_STRING,
								paste, zwindow, CurrentTime);
							ShowCursor(TRUE);
						}
						break;
					case Button3:
						MarkToPointer(PushRow, PushCol);
						break;
				}
				break;
	
			case MotionNotify:
				if(Dragging)
				{
					if(Dragging == START_DRAG)
						/* Set the mark to start of drag area */
						MarkToPointer(PushRow, PushCol);
					/* reset every itertation due to DRAGSCROLL */
					Dragging = DRAGGING;
					while(XCheckMaskEvent(display, PointerMotionMask, &event));
					col = event.xmotion.x / fontwidth;
					row = event.xmotion.y / fontheight;

					if(event.xmotion.y <= 0 || row < Curwdo->first)
					{
						Arg = 1;
						Zprevline();
						Refresh();
						SetTimer(&event, DRAG_TIMEOUT);
						Dragging = DRAGSCROLL;
					}
					else if(row >= Curwdo->last)
					{	/* scroll down */
						Arg = 1;
						Znextline();
						Refresh();
						SetTimer(&event, DRAG_TIMEOUT);
						Dragging = DRAGSCROLL;
					}
					else if(PushRow != row || PushCol != col)
					{	/* move the point */
						PointToPointer(row, col);
						Refresh();
						PushRow = row; PushCol = col;
					}
				}
				break;

			case ButtonRelease:
				Dragging = NO_DRAG;
				ShowCursor(TRUE);
				break;

			case KeyPress:
				XLookupString((XKeyEvent *)&event, &c, 1, &key, 0);

#ifdef sun
				/* Hack to get keypad working as a keypad when NumLock is
				 * on. 
				 *
				 * Notes: cursor keys will not work will NumLock on
				 *        hitting shift/meta/ctrl turns off NumLock
				 */
				if((Special & NumLock) == NumLock)
				{
					switch(key)
					{
						case 0xff63: c = '0'; key = XK_0; break;
						case 0xffde: c = '1'; key = XK_1; break;
						case 0xff54: c = '2'; key = XK_2; break;
						case 0xffe0: c = '3'; key = XK_3; break;
						case 0xff51: c = '4'; key = XK_4; break;
						case 0xffdc: c = '5'; key = XK_5; break;
						case 0xff53: c = '6'; key = XK_6; break;
						case 0xffd8: c = '7'; key = XK_7; break;
						case 0xff52: c = '8'; key = XK_8; break;
						case 0xffda: c = '9'; key = XK_9; break;

						case 0xffab: c = '+'; key = XK_plus; break;
						case 0xffd5: c = '-'; key = XK_minus; break;
						case 0xffd6: c = '/'; key = XK_slash; break;
						case 0xffd7: c = '*'; key = XK_asterisk; break;
						case 0xffff: c = '.'; key = XK_period; break;
						case 0xff8d: c = '\n'; key = XK_Return; break;
					}
				}
				
				if(key == 0x1005ff78)
				{	/* Sun5 keyboard sound off/on key */
					Vars[VSILENT].val ^= 1;
					continue;
				}

				/* Sun page keys do not autorepeat */
				if(key == XK_Page_Up || key == XK_Page_Down)
				{
					if(Dragging == PAGING)
						SetTimer(&event, PAGE_TIMEOUT);
					else
					{	/* make first timeout longer */
						SetTimer(&event, PAGE_TIMEOUT * 5);
						Dragging = PAGING;
					}
				}
#endif
				if((key >= XK_space && key <= XK_asciitilde) ||
				   (key >= XK_BackSpace && key <= XK_Return) ||
				    key == XK_Escape || key == XK_Delete)
				{
					Cmd = c;
					if(Special & Meta) Cmd |= 128;
				}
				else if(key >= XK_Home && key <= XK_Begin)
				{
					if(Special & Ctrl)
						Cmd = key - XK_Home + ZXK_CHome;
#define PCSTYLE
#ifdef PCSTYLE
					else if(key == XK_Home)
					{
						if(AlarmFlag)
							++HomeCnt;
						else
							HomeCnt = 0;
						SetTimer(0, KEY_TIMEOUT);
						switch(HomeCnt)
						{
							case 0:	Cmd = ZXK_Home;		break;
							case 1: Cmd = ZXK_SHome;	break;
							case 2: Cmd = ZXK_CHome;	break;
							default: break;
						}
					}
					else if(key == XK_End)
					{
						if(AlarmFlag)
							++EndCnt;
						else
							EndCnt = 0;
						SetTimer(0, KEY_TIMEOUT);
						switch(EndCnt)
						{
							case 0:	Cmd = ZXK_End;	break;
							case 1: Cmd = ZXK_SEnd;	break;
							case 2: Cmd = ZXK_CEnd;	break;
							default: break;
						}
					}
#endif
					else
						Cmd = key - XK_Home + ZXK_Home;
				}
				else if(key >= XK_Select && key <= XK_Break)
					Cmd = key - XK_Select + ZXK_Select;
				else if(key >= XK_F1 && key <= XK_F35)
					Cmd = key - XK_F1 + ZXK_F1;
				else	
					switch(key)
					{
						case XK_Shift_L:
						case XK_Shift_R:
							Special |= Shift;		break;
						case XK_Control_L:
						case XK_Control_R:
							Special |= Ctrl;		break;
						case XK_Meta_L:
						case XK_Meta_R:
						case XK_Alt_L:
						case XK_Alt_R:
							Special |= Meta;		break;
						case XK_Num_Lock:
							Special |= NumLock;		break;
					}
				break;
				
			case KeyRelease:
				/* We only care about Specials here. */
				XLookupString((XKeyEvent *)&event, &c, 1, &key, 0);
				switch(key)
				{
					case XK_Shift_L:
					case XK_Shift_R:
						Special &= ~Shift;		break;
					case XK_Control_L:
					case XK_Control_R:
						Special &= ~Ctrl;		break;
					case XK_Meta_L:
					case XK_Meta_R:
					case XK_Alt_L:
					case XK_Alt_R:
						Special &= ~Meta;		break;
					case XK_Num_Lock:
						Special &= ~NumLock;	break;
				}
#ifdef sun
				/* turn PAGING off */
				Dragging = NO_DRAG;
#endif
				break;
			
			case FocusIn:
#ifdef BORDER3D
				GrabKeyboard(InPaw ? PAWwindow : zwindow);
				XSetWindowBorder(display, textwindow, oldfg);
#else
				GrabKeyboard(zwindow);
#endif
				Focus = TRUE;
				ShowCursor(TRUE);
				break;
			case FocusOut:
				/* We get a NotifyInferior when switching to the PAW.
				 * Just ignore it.
				 */
				if(event.xfocus.detail != NotifyInferior)
				{
					Focus = FALSE;
					ShowCursor(FALSE);
#ifdef BORDER3D
					XSetWindowBorder(display, textwindow, oldbg);
#endif
				}
				break;

			case SelectionNotify:
				if(event.xselection.property == paste)
					Paste(paste);
				break;
				
			case SelectionRequest:
			{	/* someone wants to paste */
				XSelectionEvent reply;

				if(SelectionData)
					XChangeProperty(
						event.xselectionrequest.display,
						event.xselectionrequest.requestor,
						event.xselectionrequest.property,
						event.xselectionrequest.type,
						8,
						PropModeReplace,
						SelectionData,
						SelectionSize);
				else
					event.xselectionrequest.property = None;

				reply.type		= SelectionNotify;
				reply.display	= event.xselectionrequest.display;
				reply.requestor = event.xselectionrequest.requestor;
				reply.selection	= event.xselectionrequest.selection;
				reply.target	= event.xselectionrequest.target;
				reply.property	= event.xselectionrequest.property;
				reply.time		= event.xselectionrequest.time;

				XSendEvent(reply.display, reply.requestor, False, 0, 
					(XEvent*)&reply);
				break;
			}
			
			case SelectionClear:
				/* This usually clears the selection highlight */
				break;
				
			/* Where are these coming from? Just ignore them. */
			case MapNotify:
			case UnmapNotify:
				break;
			
#if DBG
			default:
				Dbg("Got unexpected event %s\n", XEventName(event.type));
				break;
#endif
		}
	}
	ShowCursor(FALSE);

	return Cmd;
}


int Tkbrdy()
{
	XEvent event;
	
	if(XCheckMaskEvent(display, KeyPressMask, &event))
	{
		XPutBackEvent(display, &event);
		return 1;
	}
	return 0;
}


/* Optimized for monochrome */
void ShowCursor(set)
Boolean set;
{
	Window window;
	static int point_x, point_y;
	static Byte wasch = ' ';		/* color only */
	static GC wasgc;

#ifdef BORDER3D
	window = InPaw ? PAWwindow : zwindow;
#else
	window = zwindow;
#endif
	if(set)
	{
		point_x = Xcol[Pcol];
		point_y = Xrow[Prow];
	}
	if(!HasColor)
		XFillRectangle(display, window, cursorgc, point_x, point_y,
				fontwidth, fontheight);
	else if(set)
	{
#if COMMENTBOLD
		CheckComment();
#endif
		wasgc = curgc;
		wasch = (Bisend() || ISNL(Buff()) || Buff() == '\t') ? ' ' : Buff();
		XDrawImageString(display, window, cursorgc, point_x,
			point_y + fontbase, &wasch, 1);
	}
	else
	{
		XDrawImageString(display, window, wasgc, point_x,
			point_y + fontbase, &wasch, 1);	
		if(!Focus)
			XDrawRectangle(display, window, cursorgc, point_x, point_y,
				fontwidth - 1, fontheight - 1);
	}
}


void SetMark(prntchar)
Boolean prntchar;
{
	if(HasColor)
	{
		GC save = curgc;
		Tflush();
		curgc = markgc;
		Tprntchar(prntchar ? Buff() : ' ');
		Tflush();
		curgc = save;
	}
	else
	{
		int mark_x = Xcol[Pcol];
		int mark_y = Xrow[Prow];
		Tprntchar(prntchar ? Buff() : ' ');
		Tflush();
		XDrawRectangle(display, zwindow, markgc, mark_x, mark_y,
			fontwidth - 1, fontheight - 1);
	}
}


void BoldWord(row, col, s, len)
int row, col, len;
char *s;
{
	Tflush();
	XDrawString(display, zwindow, curgc, Xcol[col] + 1,
				Xrow[row] + fontbase, s, len);
}


/* must be called after load_font */
static void GetGeometry(sh, bw)
XSizeHints *sh;
int bw;
{
	char *gstr;
	int flags = 0;

	/* set defaults */
	sh->flags		= PPosition | PSize | PMinSize | PMaxSize | PWinGravity;
	sh->x			= 0;
	sh->y			= 0;
	sh->width		= 80;
	sh->height		= 30;
	sh->min_width	= Xcol[10];
	sh->min_height	= Xrow[5];
	sh->max_width	= Xcol[COLMAX];
	sh->max_height	= Xrow[ROWMAX];

	/* now check database */
	if((gstr = GetResource(".geometry", ".Geometry")) != NULL)
	{
		flags = XParseGeometry(gstr, &sh->x, &sh->y, &sh->width, &sh->height);
		if(flags & (XValue | YValue))			sh->flags |= USPosition;
		if(flags & (WidthValue | HeightValue))	sh->flags |= USSize;
	}
#ifdef SAM_DOES_NOT_ALLOW_TWM_TO_SET
	else if(QueryPointer(display, &sh->x, &sh->y))
		sh->flags |= USPosition;
#endif
	
	/* convert from chars to pixels */
	sh->width	*= fontwidth;
	sh->height	*= fontheight;
	
	/* handle negative positions */
	if(flags & XNegative)
	{
		sh->x = DisplayWidth(display, screen) - sh->width - (2 * bw) - sh->x;
		if(sh->x < 0) sh->x = 0;
	}
	if(flags & YNegative)
	{
		sh->y = DisplayHeight(display, screen) - sh->height - (2 * bw) - sh->y;
		if(sh->y < 0) sh->y = 0;
		sh->win_gravity = SouthWestGravity;
	}
	else
		sh->win_gravity = NorthWestGravity;
}


static void Paste(paste)
Atom paste;
{
	Atom actual_type;
	int actual_format;
	unsigned long nitems, leftover;
	char *string, *p;
	unsigned long offset = 0;

	do
	{
		if(XGetWindowProperty(display, zwindow, paste, offset, 0x100000L,
			False, AnyPropertyType, &actual_type, &actual_format,
			&nitems, &leftover, (Byte **)&string) != Success)
				return;
		switch(actual_format)
		{
			case 16: nitems *= 2; break;
			case 32: nitems *= 4; break;
		}
		offset += nitems;
		for(p = string; nitems-- > 0l; ++p) Binsert(*p);
		XFree(string);
	}
	while(leftover);
	Refresh();
}


/********************************************************************
 *
 *					  MODELINE/PAW OUTPUT ROUTINES
 *
 ********************************************************************/
 
/* This is called before the windows are created */
void Initline()
{
	extern char *Cwd;

	if(VAR(VSHOWCWD)) Newtitle(Cwd);

#if 0
#ifndef BORDER3D
	int i;

	sprintf(PawStr, "%s %s  Initializing", ZSTR, VERSION);
	Tclrwind();
	Tgoto(Rowmax - 2, 0);
	Tstyle(T_STANDOUT);
	Tprntstr(PawStr);
	for(i = strlen(PawStr) + 1; i < Colmax + 3; ++i) Tprntchar(' ');
	Tstyle(T_NORMAL);
	Tflush();
#endif
#endif
}

/* SAM HACK IT AS STATIC FOR NOW */
static char modeline[COLMAX + 1];

/* Redraw the modeline except for flags. */
static void Modeline(wdo, y)
WDO *wdo;
int y;
{
	memset(modeline, ' ', COLMAX);
	sprintf(modeline, ZFMT, ZSTR, VERSION,
		Setmodes(wdo->wbuff), wdo->wbuff->bname);
	if(wdo->wbuff->fname)
	{
		int len = (VAR(VLINES) ? 13 : 3) + strlen(modeline);
		strcat(modeline, Limit(wdo->wbuff->fname, len));
	}
	wdo->modecol = strlen(modeline) + 1;

	modeline[wdo->modecol - 1] = ' ';	/* replace null */

#ifdef BORDER3D
	XDrawImageString(display, Zroot, modegc, 0, y, modeline,
		COLMAX);
#elif defined(SCROLLBARS)
	XDrawImageString(display, Zroot, modegc, 0, y, modeline, Colmax + 3);
#else
	XDrawImageString(display, Zroot, modegc, 0, y, modeline, Colmax);
#endif
}


void Modeflags(wdo)
WDO *wdo;
{
	unsigned line, col, mask;
#ifdef BORDER3D
	int y = win_height - H_PAW - fontheight - border_width + fontbase;
#elif defined(HSCROLL)
	int y = Xrow[wdo->last] + fontbase + SCROLLBAR_WIDTH + 2;
#else
	int y = Xrow[wdo->last] + fontbase;
#endif

	if(wdo->modeflags == INVALID) Modeline(wdo, y);

	mask = Delcmd() | (wdo->wbuff->bmodf ? 2 : 0);
	if(!InPaw && wdo->modeflags != mask)
	{
		char flags[2];

		flags[0] = mask & 2 ? '*' : ' ';
		flags[1] = mask & 1 ? '+' : ' ';
		XDrawImageString(display, Zroot, modegc,
			Xcol[wdo->modecol], y, flags, 2);

		wdo->modeflags = mask;
	}

	if(VAR(VLINES) == 2)
	{	/* show as % */
		Buffer *was = Curbuff;

		/* SAM this could be optimized */
		Bswitchto(wdo->wbuff);
		Blocation(&line);
		col = Blines(Curbuff);

		sprintf(PawStr, "%3u%%", (line * 100 + 5) / col);

		XDrawImageString(display, Zroot, modegc,
			Xcol[Colmax - 5], y,
			PawStr, 4);

		Bswitchto(was);
	}
	else if(VAR(VLINES))
	{	/* show as line/col */
		Buffer *was = Curbuff;
		
		Bswitchto(wdo->wbuff);
		Blocation(&line);
		if((col = Bgetcol(FALSE, 0) + 1) > 999)
			sprintf(PawStr, "%5u:???", line);
		else
			sprintf(PawStr, "%5u:%-3u", line, col);
		PawStr[9] = '\0';

		XDrawImageString(display, Zroot, modegc,
			Xcol[Colmax - 10], y,
			PawStr, 9);

		Bswitchto(was);
	}
}


void Clrecho()
{
#ifdef BORDER3D
	XClearWindow(display, PAWwindow);
	Xflush();
#else
	XClearArea(display, Zroot, 0, Xrow[Rowmax - 1], 0, fontheight, 0);	
#endif
}


/* Put a string into the PAW.
 * type is:	0 for echo			Echo()		macro
 * 			1 for error			Error()		macro
 * 			2 for save pos
 *
 * For XWINDOWS we don't want to wait for key.
 */
void PutPaw(str, type)
char *str;
int type;
{
	if(type == 1) Tbell();
	if(!InPaw)
	{
		Clrecho();
#ifdef BORDER3D
		XDrawString(display, PAWwindow, PAWgc, 0, fontbase, str, strlen(str));
#else
		XDrawString(display, Zroot, normgc, 0, Xrow[Rowmax - 1] + fontbase,
			str, strlen(str));
#endif
	}
}


/* We cannot call Refresh here since it makes too many assumptions
 * about Curbuff, Prow, Pcol, Inpaw, etc.
 */
static void ExposeWindow()
{
	int i;
	Mark *psave = Bcremrk();
	int inpaw = InPaw;
	int prow = Prow, pcol = Pcol;
#ifndef BORDER3D
	Buffer *bsave = Curbuff;
	WDO *wdo;
#else

	/* Draw the border around the window */
	DrawBorders();
#endif

	/* invalidate the windows */
	for(i = 0; i < Tmaxrow() - 1; ++i) Scrnmarks[i].modf = TRUE;
	Tlrow = -1;

	/* Update all the windows */
	InPaw = FALSE;

	Mrktomrk(Curwdo->wstart, Sstart);
#ifdef BORDER3D
	Bpnttomrk(Curwdo->wstart);
	Innerdsp(Curwdo->first, Curwdo->last, NULL);
	Curwdo->modeflags = INVALID;
	Modeflags(Curwdo);
#else
	for(wdo = Whead; wdo; wdo = wdo->next)
	{
		Bswitchto(wdo->wbuff);
		Settabsize(Curbuff->bmode);
		Bpnttomrk(wdo->wstart);
		Innerdsp(wdo->first, wdo->last, NULL);
		wdo->modeflags = INVALID;
		Modeflags(wdo);
	}
	Bswitchto(bsave);
	Settabsize(Curbuff->bmode);
#endif
	Bpnttomrk(psave);
	Unmark(psave);

	/* Update paw if necessary */
	if((InPaw = inpaw))
	{
		extern char *PromptString;

#ifdef BORDER3D
		XDrawImageString(display, PAWwindow, PAWgc,
			0, fontbase, PromptString, strlen(PromptString));
#else
		XDrawImageString(display, Zroot, normgc,
			0, Xrow[Rowmax - 1] + fontbase,
			PromptString, strlen(PromptString));
#endif

		/* We may have changed these above */
		Funcs = Pawcmds;	
		Keys[CR] = ZNEWLINE;

		Refresh();
	}
	
	XFlush(display);

	Tsetpoint(prow, pcol);
	ShowCursor(TRUE);
}

/* Change the foreground of the zwindow. Called from the game of life. */
void Foreground(unsigned long foreground)
{
	XSetForeground(display, curgc, foreground);
}


void Xflush()
{
	extern Display *display;
	XFlush(display);
}


void GrabKeyboard(Window window)
{
#ifdef BORDER3D
	/* XGrabKeyboard too drastic here. */
	XSetInputFocus(display, window, RevertToPointerRoot, CurrentTime);
#endif
}


static XEvent motion;

void scrollit()
{	/* mimic a MotionEvent that will cause a scroll */
	if(Dragging == DRAGSCROLL)
	{
		XSendEvent(display, Zroot, True, PointerMotionMask, &motion);
		XFlush(display);	/* needed */
	}
#ifdef sun
	else if(Dragging == PAGING)
	{
		XSendEvent(display, Zroot, True, ButtonPressMask, &motion);
		XFlush(display);	/* needed */
	}
#endif
}

void setit()
{
	AlarmFlag = 0;
}

static void SetTimer(XEvent *event, unsigned timeout)
{
	struct itimerval value;

	if(event)
	{
		memcpy(&motion, event, sizeof(XEvent));
		signal(SIGALRM, scrollit);
	}
	else
	{
		signal(SIGALRM, setit);
		AlarmFlag = 1;
	}
	
	/* one shot */
	value.it_value.tv_usec		= timeout;
	value.it_value.tv_sec		= 0;
	value.it_interval.tv_usec	= 0;
	value.it_interval.tv_sec	= 0;
	setitimer(ITIMER_REAL, &value, 0);
}

void AddWindowSizes(char *str)
{
	sprintf(str, "   Display: %dx%dx%d   Window: %dx%d",
		DisplayWidth(display, screen), DisplayHeight(display, screen),
		DefaultDepth(display, screen), win_width, win_height);
}
#endif
