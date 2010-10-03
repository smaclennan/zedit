/* xwind.c - Zedit main X routines
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

#include "../z.h"

#ifdef XWINDOWS
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include "xwind.h"
#include "../keys.h"

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
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static struct {
	char *name;
	int cursor;
} Xcursors[] = {
	{ "X_cursor",			XC_X_cursor },
	{ "arrow",			XC_arrow },
	{ "based_arrow_down",		XC_based_arrow_down },
	{ "based_arrow_up",		XC_based_arrow_up },
	{ "boat",			XC_boat },
	{ "bogosity",			XC_bogosity },
	{ "bottom_left_corner",		XC_bottom_left_corner },
	{ "bottom_right_corner",	XC_bottom_right_corner },
	{ "bottom_side",		XC_bottom_side },
	{ "bottom_tee",			XC_bottom_tee },
	{ "box_spiral",			XC_box_spiral },
	{ "center_ptr",			XC_center_ptr },
	{ "circle",			XC_circle },
	{ "clock",			XC_clock },
	{ "coffee_mug",			XC_coffee_mug },
	{ "cross",			XC_cross },
	{ "cross_reverse",		XC_cross_reverse },
	{ "crosshair",			XC_crosshair },
	{ "diamond_cross",		XC_diamond_cross },
	{ "dot",			XC_dot },
	{ "dotbox",			XC_dotbox },
	{ "double_arrow",		XC_double_arrow },
	{ "draft_large",		XC_draft_large },
	{ "draft_small",		XC_draft_small },
	{ "draped_box",			XC_draped_box },
	{ "exchange",			XC_exchange },
	{ "fleur",			XC_fleur },
	{ "gobbler",			XC_gobbler },
	{ "gumby",			XC_gumby },
	{ "hand1",			XC_hand1 },
	{ "hand2",			XC_hand2 },
	{ "heart",			XC_heart },
	{ "icon",			XC_icon },
	{ "iron_cross",			XC_iron_cross },
	{ "left_ptr",			XC_left_ptr },
	{ "left_side",			XC_left_side },
	{ "left_tee",			XC_left_tee },
	{ "leftbutton",			XC_leftbutton },
	{ "ll_angle",			XC_ll_angle },
	{ "lr_angle",			XC_lr_angle },
	{ "man",			XC_man },
	{ "middlebutton",		XC_middlebutton },
	{ "mouse",			XC_mouse },
	{ "pencil",			XC_pencil },
	{ "pirate",			XC_pirate },
	{ "plus",			XC_plus },
	{ "question_arrow",		XC_question_arrow },
	{ "right_ptr",			XC_right_ptr },
	{ "right_side",			XC_right_side },
	{ "right_tee",			XC_right_tee },
	{ "rightbutton",		XC_rightbutton },
	{ "rtl_logo",			XC_rtl_logo },
	{ "sailboat",			XC_sailboat },
	{ "sb_down_arrow",		XC_sb_down_arrow },
	{ "sb_h_double_arrow",		XC_sb_h_double_arrow },
	{ "sb_left_arrow",		XC_sb_left_arrow },
	{ "sb_right_arrow",		XC_sb_right_arrow },
	{ "sb_up_arrow",		XC_sb_up_arrow },
	{ "sb_v_double_arrow",		XC_sb_v_double_arrow },
	{ "shuttle",			XC_shuttle },
	{ "sizing",			XC_sizing },
	{ "spider",			XC_spider },
	{ "spraycan",			XC_spraycan },
	{ "star",			XC_star },
	{ "target",			XC_target },
	{ "tcross",			XC_tcross },
	{ "top_left_arrow",		XC_top_left_arrow },
	{ "top_left_corner",		XC_top_left_corner },
	{ "top_right_corner",		XC_top_right_corner },
	{ "top_side",			XC_top_side },
	{ "top_tee",			XC_top_tee },
	{ "trek",			XC_trek },
	{ "ul_angle",			XC_ul_angle },
	{ "umbrella",			XC_umbrella },
	{ "ur_angle",			XC_ur_angle },
	{ "watch",			XC_watch },
	{ "xterm",			XC_xterm },
};
#define XCURSORS	(sizeof(Xcursors) / (sizeof(char *) + sizeof(int)))

char *KeyNames[] = {
	"Home", "Left", "Up", "Right", "Down", "Prior",
	"Next", "End", "Begin", "Select", "Print", "Execute",
	"Insert", "???", "Undo", "Redo", "Menu", "Find",
	"Cancel", "Help", "Break",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
	"F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17",
	"F18", "F19", "F20", "F21", "F22", "F23", "F24", "F25",
	"F26", "F27", "F28", "F29", "F30", "F31", "F32", "F33",
	"F34", "F35", "Ctrl Home", "Ctrl Left", "Ctrl Up", "Ctrl Right",
	"Ctrl Down", "Ctrl Prior", "Ctrl Next", "Ctrl End", "Ctrl Begin"
};

#define DOUBLE_CLICK		500		/* 1/2 second */

static Window CreateRootWindow(int argc, char **argv, int *, int *);
static void GetGeometry(XSizeHints *sh, int bw);
static void ExposeWindow(void), SetupSpecial(void);
static void SetTimer(XEvent *event, unsigned timeout);

static void Paste(Atom paste);

Window Zroot, zwindow;

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

static Atom WM_PROTOCOLS;	/* message sent by window manager (twm) */
static Atom WM_DELETE_WINDOW;	/* subtype of WM_PROTOCOLS */

char *PromptString;

void tinit(int argc, char **argv)
{
	XGCValues values;
	uint valuemask;
	int ccolor;
	int width, height;
	XEvent event;

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
	if (HasColor) {
		/* override with these values if available */
		if (!ColorResource(".text.foreground", ".text.Foreground",
				   &foreground))
			ColorResource(".foreground", ".Foreground",
				      &foreground);
		if (!ColorResource(".text.background", ".text.Background",
				   &background))
			ColorResource(".background", ".Background",
				      &background);
	}

	LoadFonts();

	/* Create root window */
	CreateRootWindow(argc, argv, &width, &height);

	/* only root window exists */
	zwindow = Zroot;

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
	if (HasColor && ColorResource(".boldColor", ".boldColor", &ccolor)) {
		/* user specified bold color */
		values.background = ccolor;
		boldgc = XCreateGC(display, zwindow, valuemask, &values);
	} else if (boldid) {
		/* user specified bold font */
		values.font = boldid;
		boldgc = XCreateGC(display, zwindow, valuemask, &values);
		values.font = fontid;
	} else
		boldgc = normgc;

	/* create GC for cursor */
	if (HasColor) {
		/* try to make a color cursor */
		if (ColorResource(".cursorColor", ".CursorColor", &ccolor)) {
			values.background = ccolor;
			values.foreground = foreground;
		} else {
			values.background = foreground;
			values.foreground = background;
		}
		values.function = GXcopy;
	} else
		/* just invert the character */
		values.function = GXinvert;
	valuemask |= GCFunction;
	cursorgc = XCreateGC(display, zwindow, valuemask, &values);

	/* create GC for mark */
	if (HasColor) {
		if (ColorResource(".markColor", ".markColor", &ccolor)) {
			values.background = ccolor;
			values.foreground = foreground;
			values.function = GXcopy;
			markgc = XCreateGC(display, zwindow,
					   valuemask, &values);
		} else
			markgc = revgc;
	} else
		markgc = normgc;

	/* Allow user to set comment color but default to blue if possible. */
	if (HasColor &&
		(ColorResource(".commentColor", ".CommentColor", &ccolor) ||
		 GetColor("blue", &ccolor))) {
		values.foreground = ccolor;
		values.background = background;
		commentgc = XCreateGC(display, zwindow, valuemask, &values);
	} else
		commentgc = boldgc;

	/* Allow user to set cpp color but default to green if possible. */
	if (HasColor &&
		(ColorResource(".cppColor", ".CppColor", &ccolor) ||
		 GetColor("green", &ccolor))) {
		values.foreground = ccolor;
		values.background = background;
		cppgc = XCreateGC(display, zwindow, valuemask, &values);
	} else
		cppgc = commentgc;

	/* Allow user to set cpp color but default to green if possible. */
	if (ColorResource(".cppifColor", ".CppifColor", &ccolor)) {
		values.foreground = ccolor;
		values.background = background;
		cppifgc = XCreateGC(display, zwindow, valuemask, &values);
	} else
		cppifgc = cppgc;

	/* create GC for modeline - do this last */
	if (HasColor) {
		/* default to reverse */
		if (ColorResource(".modelinebg", ".modelineBG", &ccolor))
			values.background = ccolor;
		else
			values.background = foreground;
		if (ColorResource(".modelinefg", ".modelingFG", &ccolor))
			values.foreground = ccolor;
		else
			values.foreground = background;

		modegc = XCreateGC(display, zwindow, valuemask, &values);
	} else
		modegc = revgc;

	/* We wait for exposure so draw commands will work.
	 * We wait for StructureNotify so we can get real window size.
	 */
	do {
		XMaskEvent(display, ExposureMask|StructureNotifyMask, &event);
		if (event.type == ConfigureNotify) {
			/* window has been resized */
			win_width  = event.xconfigure.width;
			win_height = event.xconfigure.height;
		}
	} while (event.type != Expose || win_width == 0);

	initline();		/* Curwdo not defined yet */

	Srow = Scol = -1;	/* undefined */
	Pcol = Prow = 0;	/* start 'em off */

	termsize();
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
#if defined(SCROLLBARS)
	size_hints.width += SCROLLBAR_WIDTH;
#  ifdef HSCROLL
	size_hints.height += SCROLLBAR_WIDTH;
#  endif
#endif
	Zroot = XCreateWindow(display,  RootWindow(display, screen),
		size_hints.x, size_hints.y, size_hints.width, size_hints.height,
		border_width, CopyFromParent, InputOutput, CopyFromParent,
		CWBackPixel | CWEventMask | CWOverrideRedirect, &attr);

	wm_hints.flags = InputHint | StateHint | IconPixmapHint;
	wm_hints.input = True;		/* Allow keyboard input. */

	resource = GetResource(".iconic", ".Iconic");
	if (resource != NULL)
		wm_hints.initial_state = IconicState;
	else
		wm_hints.initial_state = NormalState;
	wm_hints.icon_pixmap = XCreateBitmapFromData(display, Zroot, zedit_bits,
		zedit_width, zedit_height);

	resource = GetResource("*icongeometry", "*iconGeometry");
	if (resource != NULL) {
		int flags;
		unsigned dummy;
		flags = XParseGeometry(resource,
				       &wm_hints.icon_x, &wm_hints.icon_y,
				       &dummy, &dummy);
		if (flags & (XValue | YValue))
			wm_hints.flags |= IconPositionHint;
	}

	class_hints.res_name = ZSTR;
	class_hints.res_class = ZSTR;

	if (XStringListToTextProperty(&name, 1, &tpname) == 0) {
		printf("Out of memory.\n");
		exit(1);
	}

	XSetWMProperties(display, Zroot, &tpname, &tpname, argv, argc,
			 &size_hints, &wm_hints, &class_hints);

	/* read the keyboard modifiers */
	SetupSpecial();

	/* create pointer if necessary */
	resource = GetResource(".pointershape", ".pointerShape");
	if (resource)
		for (i = 0; i < XCURSORS; ++i)
			if (strcmp(resource, Xcursors[i].name) == 0) {
				textcursor =
					XCreateFontCursor(display,
							  Xcursors[i].cursor);
				XDefineCursor(display, Zroot, textcursor);
				break;
			}
	/* create scrollpointer - default to vertical scrollbar double arrow */
	resource = GetResource(".vscrollpointer", ".vscrollPointer");
	if (!resource) {
		resource = GetResource(".scrollpointer", ".scrollPointer");
		if (!resource)
			resource = "sb_v_double_arrow";
	}
	for (i = 0; i < XCURSORS; ++i)
		if (strcmp(resource, Xcursors[i].name) == 0) {
			vscrollcursor = XCreateFontCursor(display,
							  Xcursors[i].cursor);
			break;
		}

	/* create scrollpointer - default to horizontal scrollbar
	 * double arrow */
	resource = GetResource(".hscrollpointer", ".hscrollPointer");
	if (!resource) {
		resource = GetResource(".scrollpointer", ".scrollPointer");
		if (!resource)
			resource = "sb_h_double_arrow";
	}
	for (i = 0; i < XCURSORS; ++i)
		if (strcmp(resource, Xcursors[i].name) == 0) {
			hscrollcursor = XCreateFontCursor(display,
							  Xcursors[i].cursor);
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

Window CreateWindow(Window parent, int x, int y, int width, int height,
		    long events)
{
	Window window;
	XSetWindowAttributes attr;

	attr.background_pixel = background;
	attr.event_mask = events;
	window = XCreateWindow(display,  parent, x, y, width, height, 1,
		CopyFromParent, InputOutput, CopyFromParent,
		CWBackPixel | CWEventMask, &attr);

	XMapWindow(display, window);

	return window;
}

void newtitle(char *title)
{
	if (title)
		sprintf(PawStr, "%s  %s", ZSTR, title);
	else
		strcpy(PawStr, ZSTR);
	XStoreName(display, Zroot, PawStr);
}

void tbell(void)
{
#define BELLTIME	100
	if (VAR(VSILENT) == 0)
		XBell(display, BELLTIME);
}

void tfini(void)
{
	CleanupSocket(-1);
}

void tclrwind(void)
{
	XClearWindow(display, zwindow);
}

void tcleol(void)
{
	if (Pcol < Clrcol[Prow]) {
		XClearArea(display, zwindow, Xcol[Pcol], Xrow[Prow],
			   0, fontheight, 0);
		Clrcol[Prow] = Pcol;
	}
}

static int s_row = -1, s_col = -1, s_len;
static char outline[COLMAX];		/* buffer of chars */

void tputchar(char c)
{
	if (Prow != s_row || Pcol != s_col)
		tflush();
	outline[s_len++] = c;
	++s_col;
}

void tflush(void)
{
	if (s_len) {
		XDrawImageString(display, zwindow, curgc,
			Xcol[s_col - s_len], Xrow[s_row] + fontbase,
			outline, s_len);
		s_len = 0;
	}
	s_row = Prow;
	s_col = Pcol;
}


/* SAM How do we handle T_BOLD/T_NORMAL pairs when in comment? */
void tstyle(int style)
{
	static int cur_style = -1;

	if (style != cur_style) {
		tflush();		/* must flush before switching style */
		switch (cur_style = style) {
		case T_NORMAL:
			curgc = normgc;		break;
		case T_STANDOUT:
			curgc = modegc;		break;
		case T_REVERSE:
			curgc = revgc;		break;
		case T_BOLD:
			curgc = boldgc;		break;
		case T_COMMENT:
			curgc = commentgc;	break;
		case T_CPP:
			curgc = cppgc;		break;
		case T_CPPIF:
			curgc = cppifgc;	break;
		}
	}
}


#ifdef SCROLLBARS
#define VSCROLL_EXTRA SCROLLBAR_WIDTH
#else
#define VSCROLL_EXTRA 0
#endif
#ifdef HSCROLL
#define HSCROLL_EXTRA (SCROLLBAR_WIDTH + 2)
#else
#define HSCROLL_EXTRA 0
#endif

void termsize(void)
{
	XEvent event;

	Rowmax = win_height / fontheight;
	Colmax = (win_width - VSCROLL_EXTRA) / fontwidth;

	if (Xrow[Rowmax] != win_height + HSCROLL_EXTRA) {
		win_height = Xrow[Rowmax] + HSCROLL_EXTRA;
		XResizeWindow(display, zwindow, win_width, win_height);
		XMaskEvent(display, StructureNotifyMask, &event);
	}
}

Byte tgetkb(void)
{
	return (Byte)tgetcmd();
}

static char *SelectionData;
static int SelectionSize;

static void SetSelection(void)
{
	struct buff *save = Curbuff;
	char *p;

	copytomrk(Curbuff->mark);
	SelectionSize = blength(Killbuff);
	if (SelectionData)
		free(SelectionData);

	SelectionData = p = malloc(SelectionSize + 1);
	if (!SelectionData) {
		error("Not enough memory");
		return;
	}

	bswitchto(Killbuff);
	btostart();
	for ( ; !bisend(); bmove1())
		*p++ = Buff();
	bswitchto(save);

	XSetSelectionOwner(display, XA_PRIMARY, zwindow, CurrentTime);
	echo("Copied to clipboard.");
}


static Boolean Focus = TRUE;

/* If in modeline, scroll buffer. */
static Boolean PointerScroll(int row, Boolean down)
{
	struct wdo *wdo;

	for (wdo = Whead; wdo; wdo = wdo->next)
		if (row == wdo->last) {
			/* on a modeline - scroll it */
			struct wdo *save = Curwdo;
			wswitchto(wdo);
			down ? Znextpage() : Zprevpage();
			wswitchto(save);
			refresh();
			return TRUE;
		}
	return FALSE;
}

static void MarkToPointer(int row, int col)
{
	showcursor(FALSE);
	if (!PointerScroll(row, FALSE)) {
		bswappnt(Curbuff->mark);
		pntmove(row, col);
		bswappnt(Curbuff->mark);
	}
	showcursor(TRUE);
}

static void PointToPointer(int row, int col)
{
	showcursor(FALSE);
	if (!PointerScroll(row, TRUE))
		pntmove(row, col);
	showcursor(TRUE);
}

/* Initially set in SetupSpecial. Used in tgetcmd. */
static int Special;
#define Normal	0
#define Meta	1
#define Shift	2
#define Ctrl	4
#define NumLock	8

/* Setup the Specials. We only care about the mask variable. */
static void SetupSpecial(void)
{
#ifdef sun
	Window root, child;
	int rx, ry, wx, wy;
	unsigned int mask;

	XQueryPointer(display, Zroot, &root, &child, &rx, &ry, &wx, &wy, &mask);
	if (mask & Mod3Mask)
		Special = NumLock;
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

static int AlarmFlag;
static int HomeCnt, EndCnt;

/* Dragging timeouts */
#define DRAG_TIMEOUT	100000
#define PAGE_TIMEOUT	150000
#define KEY_TIMEOUT	999999 /* 1 second - 1 usecond */

int tgetcmd(void)
{
	XEvent event;
	char c;
	KeySym key;
	int row, col;
	Atom paste = 0; /*shutup*/
	static int PushRow, PushCol;
	static Time ButtonTime;		/* time of last button click */

	/* process events until keypress */
	showcursor(TRUE);	/* only display cursor when waiting */

	for (Cmd = K_NODEF; Cmd == K_NODEF; ) {
		if (!XPending(display))
			ProcessFDs();

		XNextEvent(display, &event);
#if 0
		printf("Event %s %s\n",
			XWindowName(event.xany.window), XEventName(event.type));
#endif

		/* Events we always handle */
		switch (event.type) {
		case Expose:
			while (XCheckTypedEvent(display, Expose, &event))
				;
			ExposeWindow();
			continue;

		case ConfigureNotify:
			/* window has been resized */
			win_width  = event.xconfigure.width;
			win_height = event.xconfigure.height;
			Zredisplay();
			showcursor(FALSE);
			refresh();
			showcursor(TRUE);
			while (XCheckTypedEvent(display, Expose, &event))
				;
			continue;

		case ClientMessage:
			if (event.xclient.message_type == WM_PROTOCOLS &&
			    event.xclient.data.l[0] == WM_DELETE_WINDOW) {
				/* exit */
				Zexit();
				exit(0);
			}
#if DBG
			else {
				int i;

				Dbg("UNKNOWN CLIENT MESSAGE: %x\n"
				    "FORMAT: %d\n\t",
				    event.xclient.message_type,
				    event.xclient.format);
				for (i = 0; i < 5; ++i)
					Dbg("%x  ", event.xclient.data.l[i]);
				Dbg("\n");
			}
#endif
			continue;
		}

#ifdef SCROLLBARS
		if (event.xany.window != zwindow) {
			ScrollEvent(&event);
			continue;
		}
#endif
		switch (event.type) {
		case ButtonPress:
			/* convert x and y to row and col */
			PushCol = event.xbutton.x / fontwidth;
			PushRow = event.xbutton.y / fontheight;

			/* process the button */
			switch (event.xbutton.button) {
			case Button1:
				if (!InPaw &&
				    event.xbutton.time <
				    ButtonTime + DOUBLE_CLICK) {
					/* double click means Find Tag */
					showcursor(FALSE);
					xfindtag();
					showcursor(TRUE);
				} else {	/* first click */
					if (Special == Shift)
						MarkToPointer(PushRow, PushCol);
					else
						PointToPointer(PushRow,
							       PushCol);
					refresh();
					Dragging = START_DRAG;
				}
				ButtonTime = event.xbutton.time;
				break;
			case Button2:
				if (Special & (Shift | Ctrl))
					/* we want it */
					SetSelection();
				else { /* Paste */
					paste = XInternAtom(display,
							    "PASTEIT", False);
					showcursor(FALSE);
					XConvertSelection(display, XA_PRIMARY,
							  XA_STRING, paste,
							  zwindow, CurrentTime);
					showcursor(TRUE);
				}
				break;
			case Button3:
				MarkToPointer(PushRow, PushCol);
				break;
			}
			break;

		case MotionNotify:
			if (Dragging) {
				if (Dragging == START_DRAG)
					/* Set the mark to start of drag area */
					MarkToPointer(PushRow, PushCol);
				/* reset every itertation due to DRAGSCROLL */
				Dragging = DRAGGING;
				while (XCheckMaskEvent(display,
						       PointerMotionMask,
						       &event))
					;
				col = event.xmotion.x / fontwidth;
				row = event.xmotion.y / fontheight;

				if (event.xmotion.y <= 0 ||
				    row < Curwdo->first) {
					Arg = 1;
					Zprevline();
					refresh();
					SetTimer(&event, DRAG_TIMEOUT);
					Dragging = DRAGSCROLL;
				} else if (row >= Curwdo->last) {
					/* scroll down */
					Arg = 1;
					Znextline();
					refresh();
					SetTimer(&event, DRAG_TIMEOUT);
					Dragging = DRAGSCROLL;
				} else if (PushRow != row || PushCol != col) {
					/* move the point */
					PointToPointer(row, col);
					refresh();
					PushRow = row; PushCol = col;
				}
			}
			break;

		case ButtonRelease:
			Dragging = NO_DRAG;
			showcursor(TRUE);
			break;

		case KeyPress:
			XLookupString((XKeyEvent *)&event, &c, 1, &key, NULL);

			if ((key >= XK_space && key <= XK_asciitilde) ||
			    (key >= XK_BackSpace && key <= XK_Return) ||
			    key == XK_Escape || key == XK_Delete) {
				Cmd = c;
				if (Special & Meta)
					Cmd |= 128;
			} else if (key >= XK_Home && key <= XK_Begin) {
				if (Special & Ctrl)
					Cmd = key - XK_Home + ZXK_CHome;
#define PCSTYLE
#ifdef PCSTYLE
				else if (key == XK_Home) {
					if (AlarmFlag)
						++HomeCnt;
					else
						HomeCnt = 0;
					SetTimer(NULL, KEY_TIMEOUT);
					switch (HomeCnt) {
					case 0:
						Cmd = ZXK_Home;		break;
					case 1:
						Cmd = ZXK_SHome;	break;
					case 2:
						Cmd = ZXK_CHome;	break;
					}
				} else if (key == XK_End) {
					if (AlarmFlag)
						++EndCnt;
					else
						EndCnt = 0;
					SetTimer(NULL, KEY_TIMEOUT);
					switch (EndCnt) {
					case 0:
						Cmd = ZXK_End;	break;
					case 1:
						Cmd = ZXK_SEnd;	break;
					case 2:
						Cmd = ZXK_CEnd;	break;
					}
				}
#endif
				else
					Cmd = key - XK_Home + ZXK_Home;
			} else if (key >= XK_Select && key <= XK_Break)
				Cmd = key - XK_Select + ZXK_Select;
			else if (key >= XK_F1 && key <= XK_F35)
				Cmd = key - XK_F1 + ZXK_F1;
			else
				switch (key) {
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
			XLookupString((XKeyEvent *)&event, &c, 1, &key, NULL);
			switch (key) {
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
			Focus = TRUE;
			showcursor(TRUE);
			break;
		case FocusOut:
			/* We get a NotifyInferior when switching to the PAW.
			 * Just ignore it.
			 */
			if (event.xfocus.detail != NotifyInferior) {
				Focus = FALSE;
				showcursor(FALSE);
			}
			break;

		case SelectionNotify:
			if (event.xselection.property == paste)
				Paste(paste);
			break;

		case SelectionRequest:
		{
			/* someone wants to paste */
			XSelectionEvent reply;

			if (SelectionData)
				XChangeProperty(
					event.xselectionrequest.display,
					event.xselectionrequest.requestor,
					event.xselectionrequest.property,
					event.xselectionrequest.type,
					8,
					PropModeReplace,
					(Byte *)SelectionData,
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
				   (XEvent *)&reply);
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
			Dbg("Got unexpected event %s\n",
			    XEventName(event.type));
			break;
#endif
		}
	}
	showcursor(FALSE);

	return Cmd;
}

int tkbrdy(void)
{
	XEvent event;

	if (XCheckMaskEvent(display, KeyPressMask, &event)) {
		XPutBackEvent(display, &event);
		return 1;
	}
	return 0;
}

/* Optimized for monochrome */
void showcursor(Boolean set)
{
	Window window;
	static int point_x, point_y;
	static char wasch = ' '; /* color only */
	static GC wasgc;

	window = zwindow;
	if (set) {
		point_x = Xcol[Pcol];
		point_y = Xrow[Prow];
	}
	if (!HasColor)
		XFillRectangle(display, window, cursorgc, point_x, point_y,
				fontwidth, fontheight);
	else if (set) {
#if COMMENTBOLD
		checkcomment();
#endif
		wasgc = curgc;
		wasch = (bisend() || ISNL(Buff()) ||
			 Buff() == '\t') ? ' ' : Buff();
		XDrawImageString(display, window, cursorgc, point_x,
			point_y + fontbase, &wasch, 1);
	} else {
		XDrawImageString(display, window, wasgc, point_x,
			point_y + fontbase, &wasch, 1);
		if (!Focus)
			XDrawRectangle(display, window, cursorgc,
				       point_x, point_y,
				       fontwidth - 1, fontheight - 1);
	}
}

void setmark(Boolean prntchar)
{
	if (HasColor) {
		GC save = curgc;
		tflush();
		curgc = markgc;
		tprntchar(prntchar ? Buff() : ' ');
		tflush();
		curgc = save;
	} else {
		int mark_x = Xcol[Pcol];
		int mark_y = Xrow[Prow];
		tprntchar(prntchar ? Buff() : ' ');
		tflush();
		XDrawRectangle(display, zwindow, markgc, mark_x, mark_y,
			fontwidth - 1, fontheight - 1);
	}
}


/* must be called after load_font */
static void GetGeometry(XSizeHints *sh, int bw)
{
	char *gstr;
	int flags = 0;

	/* set defaults */
	sh->flags = PPosition | PSize | PMinSize | PMaxSize | PWinGravity;
	sh->x = 0;
	sh->y = 0;
	sh->width = 80;
	sh->height = 30;
	sh->min_width = Xcol[10];
	sh->min_height = Xrow[5];
	sh->max_width = Xcol[COLMAX];
	sh->max_height = Xrow[ROWMAX];

	/* now check database */
	gstr = GetResource(".geometry", ".Geometry");
	if (gstr != NULL) {
		flags = XParseGeometry(gstr, &sh->x, &sh->y,
				       (unsigned *)&sh->width,
				       (unsigned *)&sh->height);
		if (flags & (XValue | YValue))
			sh->flags |= USPosition;
		if (flags & (WidthValue | HeightValue))
			sh->flags |= USSize;
	}
#ifdef SAM_DOES_NOT_ALLOW_TWM_TO_SET
	else if (QueryPointer(display, &sh->x, &sh->y))
		sh->flags |= USPosition;
#endif

	/* convert from chars to pixels */
	sh->width	*= fontwidth;
	sh->height	*= fontheight;

	/* handle negative positions */
	if (flags & XNegative) {
		sh->x = DisplayWidth(display, screen) -
			sh->width - (2 * bw) - sh->x;
		if (sh->x < 0)
			sh->x = 0;
	}
	if (flags & YNegative) {
		sh->y = DisplayHeight(display, screen) -
			sh->height - (2 * bw) - sh->y;
		if (sh->y < 0)
			sh->y = 0;
		sh->win_gravity = SouthWestGravity;
	} else
		sh->win_gravity = NorthWestGravity;
}

static void Paste(Atom paste)
{
	Atom actual_type;
	int actual_format;
	unsigned long nitems, leftover;
	char *string, *p;
	unsigned long offset = 0;

	do {
		if (XGetWindowProperty(display, zwindow, paste, offset,
				       0x100000L, False, AnyPropertyType,
				       &actual_type, &actual_format, &nitems,
				       &leftover, (Byte **)&string) != Success)
				return;
		switch (actual_format) {
		case 16:
			nitems *= 2; break;
		case 32:
			nitems *= 4; break;
		}
		offset += nitems;
		for (p = string; nitems-- > 0l; ++p)
			binsert(*p);
		XFree(string);
	} while (leftover);
	refresh();
}

/* This is called before the windows are created */
void initline(void)
{
	if (VAR(VSHOWCWD))
		newtitle(Cwd);
}

/* SAM HACK IT AS STATIC FOR NOW */
static char modeline[COLMAX + 1];

/* Redraw the modeline except for flags. */
static void Modeline(struct wdo *wdo, int y)
{
	memset(modeline, ' ', COLMAX);
	sprintf(modeline, ZFMT, ZSTR, VERSION,
		setmodes(wdo->wbuff), wdo->wbuff->bname);
	if (wdo->wbuff->fname) {
		int len = (VAR(VLINES) ? 13 : 3) + strlen(modeline);
		strcat(modeline, limit(wdo->wbuff->fname, len));
	}
	wdo->modecol = strlen(modeline) + 1;

	modeline[wdo->modecol - 1] = ' ';	/* replace null */

#ifdef SCROLLBARS
	XDrawImageString(display, Zroot, modegc, 0, y, modeline, Colmax + 3);
#else
	XDrawImageString(display, Zroot, modegc, 0, y, modeline, Colmax);
#endif
}


void modeflags(struct wdo *wdo)
{
	unsigned line, col, mask;
#ifdef HSCROLL
	int y = Xrow[wdo->last] + fontbase + SCROLLBAR_WIDTH + 2;
#else
	int y = Xrow[wdo->last] + fontbase;
#endif

	if (wdo->modeflags == INVALID)
		Modeline(wdo, y);

	mask = delcmd() | (wdo->wbuff->bmodf ? 2 : 0);
	if (!InPaw && wdo->modeflags != mask) {
		char flags[2];

		flags[0] = mask & 2 ? '*' : ' ';
		flags[1] = mask & 1 ? '+' : ' ';
		XDrawImageString(display, Zroot, modegc,
			Xcol[wdo->modecol], y, flags, 2);

		wdo->modeflags = mask;
	}

	if (VAR(VLINES) == 2) {
		/* show as % */
		struct buff *was = Curbuff;

		/* SAM this could be optimized */
		bswitchto(wdo->wbuff);
		blocation(&line);
		col = blines(Curbuff);

		sprintf(PawStr, "%3u%%", (line * 100 + 5) / col);

		XDrawImageString(display, Zroot, modegc,
			Xcol[Colmax - 5], y,
			PawStr, 4);

		bswitchto(was);
	} else if (VAR(VLINES)) {
		/* show as line/col */
		struct buff *was = Curbuff;

		bswitchto(wdo->wbuff);
		blocation(&line);
		col = bgetcol(FALSE, 0) + 1;
		if (col > 999)
			sprintf(PawStr, "%5u:???", line);
		else
			sprintf(PawStr, "%5u:%-3u", line, col);
		PawStr[9] = '\0';

		XDrawImageString(display, Zroot, modegc,
			Xcol[Colmax - 10], y,
			PawStr, 9);

		bswitchto(was);
	}
}

void clrecho(void)
{
	XClearArea(display, Zroot, 0, Xrow[Rowmax - 1], 0, fontheight, 0);
}

/* Put a string into the PAW.
 * type is:	0 for echo	Echo() macro
 *		1 for error	Error() macro
 *		2 for save pos
 *
 * For XWINDOWS we don't want to wait for key.
 */
void putpaw(char *str, int type)
{
	if (type == 1)
		tbell();
	if (!InPaw) {
		clrecho();
		XDrawString(display, Zroot, normgc, 0,
			    Xrow[Rowmax - 1] + fontbase,
			    str, strlen(str));
	}
}

/* We cannot call Refresh here since it makes too many assumptions
 * about Curbuff, Prow, Pcol, Inpaw, etc.
 */
static void ExposeWindow(void)
{
	int i;
	struct mark *psave = bcremrk();
	int inpaw = InPaw;
	int prow = Prow, pcol = Pcol;
	struct buff *bsave = Curbuff;
	struct wdo *wdo;

	/* invalidate the windows */
	for (i = 0; i < tmaxrow() - 1; ++i)
		Scrnmarks[i].modf = TRUE;
	Tlrow = -1;

	/* Update all the windows */
	InPaw = FALSE;

	mrktomrk(Curwdo->wstart, Sstart);
	for (wdo = Whead; wdo; wdo = wdo->next) {
		bswitchto(wdo->wbuff);
		settabsize(Curbuff->bmode);
		bpnttomrk(wdo->wstart);
		innerdsp(wdo->first, wdo->last, NULL);
		wdo->modeflags = INVALID;
		modeflags(wdo);
	}
	bswitchto(bsave);
	settabsize(Curbuff->bmode);
	bpnttomrk(psave);
	unmark(psave);

	/* Update paw if necessary */
	InPaw = inpaw;
	if (InPaw) {
		XDrawImageString(display, Zroot, normgc,
			0, Xrow[Rowmax - 1] + fontbase,
			PromptString, strlen(PromptString));

		/* We may have changed these above */
		Curcmds = 2;
		Keys[CR] = ZNEWLINE;

		refresh();
	}

	XFlush(display);

	tsetpoint(prow, pcol);
	showcursor(TRUE);
}

void Xflush(void)
{
	XFlush(display);
}

static XEvent motion;

static void scrollit(int signum)
{	/* mimic a MotionEvent that will cause a scroll */
	if (Dragging == DRAGSCROLL) {
		XSendEvent(display, Zroot, True, PointerMotionMask, &motion);
		XFlush(display);	/* needed */
	}
#ifdef sun
	else if (Dragging == PAGING) {
		XSendEvent(display, Zroot, True, ButtonPressMask, &motion);
		XFlush(display);	/* needed */
	}
#endif
}

static void setit(int signum)
{
	AlarmFlag = 0;
}

static void SetTimer(XEvent *event, unsigned timeout)
{
	struct itimerval value;

	if (event) {
		memcpy(&motion, event, sizeof(XEvent));
		signal(SIGALRM, scrollit);
	} else {
		signal(SIGALRM, setit);
		AlarmFlag = 1;
	}

	/* one shot */
	value.it_value.tv_usec		= timeout;
	value.it_value.tv_sec		= 0;
	value.it_interval.tv_usec	= 0;
	value.it_interval.tv_sec	= 0;
	setitimer(ITIMER_REAL, &value, NULL);
}

void addwindowsizes(char *str)
{
	sprintf(str, "   Display: %dx%dx%d   Window: %dx%d",
		DisplayWidth(display, screen), DisplayHeight(display, screen),
		DefaultDepth(display, screen), win_width, win_height);
}
#endif
