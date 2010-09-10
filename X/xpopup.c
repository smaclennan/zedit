#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef BORDER3Dy

#include <X11/keysym.h>
#include "z.h"
#include "xwind.h"


static void DrawButton(Window button, int width, int height, char *text,
	int pushed);
void QueryPointer(int *x, int *y);

extern Border outline, Zborder;
extern GC normgc;

int PopupGetarg(char *prompt, char *iarg, int max)
{
	XSizeHints size;
	Window popup, entry, button1, button2;
	XEvent event; 
	XSetWindowAttributes attr;
	unsigned long attr_mask;
	int line, plen;
	int x, y, w, h;
	Byte c;
	KeySym key;
	int i;
	char arg[PATHMAX + 1];
	
	if(max > PATHMAX) max = PATHMAX;

	/* SAM NOT NEEDED */
	attr.background_pixel  = Zborder.bgColorPtr->pixel;
	attr.event_mask = ExposureMask | ButtonPress;
	attr.override_redirect = True;
	attr_mask = CWBackPixel | CWEventMask | CWOverrideRedirect;

	/* set defaults */
	QueryPointer(&size.x, &size.y);
	/* SAM NOT NEEDED */
	size.flags		= PPosition | PSize | PMinSize | PMaxSize;
	size.min_width	=
	size.max_width	=
	size.width		= 300;
	size.min_height	=
	size.max_height	=
	size.height		= 80;

	/* create the popup window */
	popup = XCreateWindow(display,  RootWindow(display, screen),
		size.x, size.y, size.width, size.height,
		2, CopyFromParent, InputOutput, CopyFromParent,
		attr_mask, &attr);
	XMapWindow(display, popup);

	/* And outline it */
	Tk_Draw3DRectangle (popup, popup, &outline, 
		0, 0, size.width, size.height, border_width, TK_RELIEF_RIDGE);

	/* Display the prompt */
	plen = strlen(prompt);
	line = border_width * 2 + highlight + fontbase;
	XDrawString(display, popup, normgc,
		border_width + highlight, line, prompt, plen);

	/* We wait for exposure so draw commands will work. */
	XMaskEvent(display, ExposureMask, &event);

	/* Outline the entry */
	plen *= fontwidth;
	x = border_width + highlight + plen;
	y = border_width + highlight;
	w = size.width - border_width * 2 - highlight - plen;
	h = fontheight + border_width * 2;
	Tk_Draw3DRectangle (popup, popup, &Zborder,
		x, y, w, h, border_width, TK_RELIEF_SUNKEN);

	/* Create the window inside the outline */
	y += border_width;
	x += border_width;
	w -= border_width * 2;
	h -= border_width * 2;	
	attr.background_pixel  = background;
	attr.event_mask = KeyPressMask;
	entry = XCreateWindow(display,  popup,
		x, y, w, h, 2, CopyFromParent, InputOutput, CopyFromParent,
		attr_mask, &attr);
	XMapWindow(display, entry);
	
	/* Add the arg if any */
	if(*iarg)
	{
		strncpy(arg, iarg, max);
		i = strlen(arg);
		XDrawImageString(display, entry, normgc, 0, fontbase, arg, i);
	}
	else i = 0;


	attr.background_pixel  = Zborder.bgColorPtr->pixel;
	attr.event_mask = ButtonPressMask | ButtonReleaseMask;

	size.flags		= PPosition | PSize;
	size.x			= 30;
	size.y			= 40;
	size.width		= (fontwidth * 5) + 20;
	size.height		= fontheight + 16;

	button1 = XCreateWindow(display,  popup,
		size.x, size.y, size.width, size.height,
		0, CopyFromParent, InputOutput, CopyFromParent,
		CWBackPixel | CWEventMask, &attr);
	XMapWindow(display, button1);

	size.x			+= size.width + 10;

	button2 = XCreateWindow(display,  popup,
		size.x, size.y, size.width, size.height,
		0, CopyFromParent, InputOutput, CopyFromParent,
		CWBackPixel | CWEventMask, &attr);
	XMapWindow(display, button2);
	
	DrawButton(button1, size.width, size.height, "Ok",  0);
	DrawButton(button2, size.width, size.height, "Abort", 0);

	do
	{ 
		XNextEvent(display, &event);
		switch(event.type)
		{
			case ButtonPress:
				if(event.xbutton.window == button1)
					DrawButton(button1, size.width, size.height, "Ok",  1);
				else if(event.xbutton.window == button2)
					DrawButton(button2, size.width, size.height, "Abort!",  1);
				break;
			case ButtonRelease:
				if(event.xbutton.window == button1 ||
				   event.xbutton.window == button2)
				{
				}
				else event.type = ButtonPress;
			case KeyPress:
				if(event.xany.window != entry)
					continue;
				XLookupString((XKeyEvent *)&event, &c, 1, &key, 0);
				if((key >= XK_space && key <= XK_asciitilde))
				{
					if(i >= max) {Tbell(); continue;}
					arg[i++] = c;
					XDrawImageString(display, entry, normgc, 0, fontbase, arg, i);
				}
				else if(key == XK_BackSpace)
				{
					if(i == 0) {Tbell(); continue;}
					--i;
					XClearArea(display, entry, Xcol[i], 0, 0, fontheight, 0);
				}
				else if(key == XK_Return)
				{	/* mimic an Ok button release */
					event.type = ButtonRelease;
					event.xany.window = button1;
				}
				break;
		}
	}
	while(event.type != ButtonRelease);

	XDestroyWindow(display, popup);
	
	/* SAM do we need to destroy all resources??? */
	
	/* user hit abort? */
	if(event.xbutton.window == button2) return ABORT;
	
	/* null string */
	if(i == 0) return 1;
	
	/* update arg */
	arg[i] = '\0';
	strcpy(iarg, arg);
	return 0;
}

static void DrawButton(Window button, int width, int height, char *text,
	int pushed)
{
	int x, y, len;
	
	Tk_Draw3DRectangle (button, button, &Zborder, 0, 0, width, height,
			border_width, pushed ? TK_RELIEF_SUNKEN : TK_RELIEF_RAISED);
			
	/* center text in window */
	len = strlen(text);
	x = (width - (len * fontwidth)) / 2;
	y = (height - fontheight) / 2 + fontbase;
	XDrawString(display, button, normgc, x, y, text, len);
}
#endif

#if 0
int CreatePopup()
{
	extern int fontid;
	XSizeHints size;
	Window window, button1, button2;
	XEvent event; 
	GC gc, gc1, gc2;
	XGCValues values;
	uint valuemask;
	XFontStruct *font;
	XSetWindowAttributes attr;
	unsigned long attr_mask;

	if((font = XQueryFont(display, fontid)) == NULL)
	{
		fprintf(stderr, "Cannot open fixed font.\n");
		exit(1);
	}

	if(!dimgrey)
	{
		black = BlackPixel(display, screen);
		white = WhitePixel(display, screen);
		if(!GetColor("DimGrey", &dimgrey)) dimgrey = black;
		if(!GetColor("LightGrey", &lightgrey)) lightgrey = white;
	}

/* SAM
	fontwidth  = font->max_bounds.width;
	fontheight = font->descent + font->ascent;
	fontbase   = font->ascent;
	fontid	   = font->fid;
*/

	attr.background_pixel  = lightgrey;
	attr.event_mask = ExposureMask;
	attr.override_redirect = True;
	attr_mask = CWBackPixel | CWEventMask | CWOverrideRedirect;

	/* set defaults */
	QueryPointer(&size.x, &size.y);
	size.flags		= PPosition | PSize | PMinSize | PMaxSize;
	size.min_width	=
	size.max_width	=
	size.width		= 230;
	size.min_height	=
	size.max_height	=
	size.height		= 80;

	window = XCreateWindow(display,  RootWindow(display, screen),
		size.x, size.y, size.width, size.height,
		2, CopyFromParent, InputOutput, CopyFromParent,
		attr_mask, &attr);

	XMapWindow(display, window);

	valuemask = GCForeground | GCBackground | GCFont;
	values.font = font->fid;
	values.foreground = foreground;
	values.background = lightgrey;
	gc = XCreateGC(display, window, valuemask, &values);

	attr.event_mask = ButtonPressMask | ButtonReleaseMask;

	size.flags		= PPosition | PSize;
	size.x			= 10;
	size.y			= 20;
	size.width		= (font->max_bounds.width * 5) + 20;
	size.height		= font->descent + font->ascent + 16;

	button1 = XCreateWindow(display,  window,
		size.x, size.y, size.width, size.height,
		0, CopyFromParent, InputOutput, CopyFromParent,
		CWBackPixel | CWEventMask, &attr);
	XMapWindow(display, button1);

	gc1 = XCreateGC(display, button1, valuemask, &values);

	size.x			= size.width + 10 + 10;

	button2 = XCreateWindow(display,  window,
		size.x, size.y, size.width, size.height,
		0, CopyFromParent, InputOutput, CopyFromParent,
		CWBackPixel | CWEventMask, &attr);
	XMapWindow(display, button2);
	
	gc2 = XCreateGC(display, button2, valuemask, &values);

	/* We wait for exposure so draw commands will work. */
	XMaskEvent(display, ExposureMask, &event);

	XDrawImageString(display, window, gc, 10, 10, "Are you sure?", 13);

	DrawButton(button1, gc1, font, size.width, size.height, "Yup!",  1);
	DrawButton(button2, gc2, font, size.width, size.height, "Maybe", 1); 

	do
	{ 
		XNextEvent(display, &event);
		switch(event.type)
		{
			case ButtonPress:
				if(event.xbutton.window == button1)
				DrawButton(event.xbutton.window, gc1, font, size.width, size.height,
						"Yup!",  0);
				else
					DrawButton(button2, gc2, font, size.width, size.height,
						"Maybe", 0);
				break;
		}
	}
	while(event.type != ButtonRelease);

	XDestroyWindow(display, window);
	
	/* SAM do we need to destroy all resources??? */
	
	return event.xbutton.window == button1 ? 1 : 0;
}

#define MAXBUTTONS	20
typedef struct {
	Window window;
	GC gc;
	int x, y;		/* SAM We may not need this! */
	int w, h;
	char *text;
	int pushed;
	int type;
#define BT_PUSH		0
#define BT_TOGGLE	1
} aButton;

static aButton buttons[MAXBUTTONS];
int nButtons = 0;

static XFontStruct *buttonfont;

static void NewToggle(parent, x, y, w, h, text, pushed)
Window parent;
int x, y, w, h, pushed;
char *text;
{
	CreateButton(parent, x, y, w, h, text, pushed, BT_TOGGLE);
}


static void NewButton(parent, x, y, w, h, text, pushed)
Window parent;
int x, y, w, h, pushed;
char *text;
{
	CreateButton(parent, x, y, w, h, text, pushed, BT_PUSH);
}


static int CreateButton(parent, x, y, w, h, text, pushed, type)
Window parent;
int x, y, w, h, pushed, type;
char *text;
{
	XSetWindowAttributes attr;
	unsigned long mask;
	XGCValues values;

	if(nButtons >= MAXBUTTONS)
{ printf("TOO MANY BUTTONS!\n");
		return 0;
}

	mask = CWBackPixel | CWEventMask;
	attr.background_pixel = lightgrey;
	attr.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask;

	buttons[nButtons].window = XCreateWindow( display,  parent,
		x, y, w, h,
		type == BT_PUSH ? 1 : 0,
		CopyFromParent, InputOutput, CopyFromParent,
		mask, &attr);

	mask = GCForeground | GCBackground | GCFont;
	values.font = buttonfont->fid;
	values.foreground = foreground;
	values.background = lightgrey;
	buttons[nButtons].gc = XCreateGC(display, parent, mask, &values);
	
	buttons[nButtons].x = x; buttons[nButtons].y = y;
	buttons[nButtons].w = w; buttons[nButtons].h = h;
	buttons[nButtons].pushed = pushed;
	buttons[nButtons].type = type;
	buttons[nButtons].text = strdup(text);

	XMapWindow(display, buttons[nButtons].window);

	++nButtons;
	
	return 1;
}

int FindButton(window)
Window window;
{
	int n;

	for(n = 0; n < nButtons; ++n)
		if(buttons[n].window == window)
			return n;
	return -1;
}


#define S_BUTTON_W		104	/* 16 chars * fixed_font_width(6) + 8 */
#define S_BUTTON_H		21	/* fixed is 13 + 4 top + 4 bottom */
#define S_BUTTON_PAD	10

Window Search = None, TextWind, ReplaceWind;
static int SearchMapped = True;
static GC TextGC;

/* For the Query Replace popup window */
static Window Query;
static Boolean QueryMapped = False, Popped = False;

void PopupSearch()
{
	if(Search == None)
		CreateSearch();
	else if(!SearchMapped)
	{
		XMapWindow(display, Search);
		SearchMapped = True;
	}
	else
		XRaiseWindow(display, Search);
}

CreateSearch()
{
	XSizeHints size;
	Window window;
	XEvent event; 
	XGCValues values;
	uint valuemask;
	XSetWindowAttributes attr;
	unsigned long attr_mask;
	int x, y, w, h, start;
	int argc = 1;
	char *argv[1];

#if 0
	if((buttonfont = XQueryFont(display, fontid)) == NULL) return;
#else
	if((buttonfont = XLoadQueryFont(display, "fixed")) == NULL) return;
#endif

	if(!dimgrey)
	{
		black = BlackPixel(display, screen);
		white = WhitePixel(display, screen);
		if(!GetColor("DimGrey", &dimgrey)) dimgrey = black;
		if(!GetColor("LightGrey", &lightgrey)) lightgrey = white;
	}

	attr.background_pixel  = lightgrey;
	attr.event_mask = ExposureMask;
	attr_mask = CWBackPixel | CWEventMask;

	/* set defaults */
	QueryPointer(&size.x, &size.y);
	size.flags		= PPosition | PSize;
	
	/* SAM Should calc this! */
	size.width		= 362;
	size.height		= 216;

	Search = window = XCreateWindow(display,  RootWindow(display, screen),
		size.x, size.y, size.width, size.height,
		2, CopyFromParent, InputOutput, CopyFromParent,
		attr_mask, &attr);

{	/* SAM */
#define TITLE	"Search/Replace Window"
	XWMHints wm_hints;

	/* This allows keyboard input */
	wm_hints.flags = InputHint;
	wm_hints.input = True;

	argv[0] = "popup";
	XSetWMProperties(display, window, 0, 0, argv, argc, &size, &wm_hints, 0);
} /* SAM */

	XMapWindow(display, window);
#if 1
	XStoreName(display, window, "Search/Replace Window");
#endif

	/* Create the text windows */
	attr.event_mask = ExposureMask | KeyPressMask |
		EnterWindowMask | LeaveWindowMask;
	x = y = S_BUTTON_PAD;
	w = size.width - S_BUTTON_PAD - S_BUTTON_PAD;
	h = S_BUTTON_H;
	TextWind = XCreateWindow(display,  window,
		x, y, w, h,
		1, CopyFromParent, InputOutput, CopyFromParent,
		attr_mask, &attr);
	XMapWindow(display, TextWind);

	valuemask = GCForeground | GCBackground | GCFont;
	values.font = buttonfont->fid;
	values.foreground = foreground;
	values.background = lightgrey;
	TextGC = XCreateGC(display, TextWind, valuemask, &values);

	XMaskEvent(display, ExposureMask, &event);

	y += h + S_BUTTON_PAD;
	ReplaceWind = XCreateWindow(display,  window,
		x, y, w, h,
		1, CopyFromParent, InputOutput, CopyFromParent,
		attr_mask, &attr);
	XMapWindow(display, ReplaceWind);

	XMaskEvent(display, ExposureMask, &event);

	/* Create the buttons */
	y += h + S_BUTTON_PAD + S_BUTTON_PAD;
	start = y;
	x = S_BUTTON_PAD;
	NewToggle(window, x, y, S_BUTTON_W, S_BUTTON_H, "Forward Search", 1);
	y += S_BUTTON_H + S_BUTTON_PAD;	
	NewToggle(window, x, y, S_BUTTON_W, S_BUTTON_H, "Reverse Search", 0);
	y += S_BUTTON_H + S_BUTTON_PAD;	
	NewToggle(window, x, y, S_BUTTON_W, S_BUTTON_H, "Expr. Search", 0);

	y= start;
	x += S_BUTTON_W + S_BUTTON_PAD;
	NewToggle(window, x, y, S_BUTTON_W, S_BUTTON_H, "Replace All", 0);
	y += S_BUTTON_H + S_BUTTON_PAD;	
	NewToggle(window, x, y, S_BUTTON_W, S_BUTTON_H, "Query Replace", 0);
	y += S_BUTTON_H + S_BUTTON_PAD;	
	NewToggle(window, x, y, S_BUTTON_W, S_BUTTON_H, "Expr. Replace", 0);

	y= start;
	x += S_BUTTON_W + S_BUTTON_PAD;
	NewToggle(window, x, y, S_BUTTON_W, S_BUTTON_H, "NoCase", 0);
	y += S_BUTTON_H + S_BUTTON_PAD;	
	NewToggle(window, x, y, S_BUTTON_W, S_BUTTON_H, "All Buffers", 0);

	
	x = (size.width - S_BUTTON_W - S_BUTTON_W) / 3;
	y += (S_BUTTON_H + S_BUTTON_PAD) * 2 + S_BUTTON_PAD;	
	NewButton(window, x, y, S_BUTTON_W, S_BUTTON_H, "Start", 0);
	x += x + S_BUTTON_W;
	NewButton(window, x, y, S_BUTTON_W, S_BUTTON_H, "Cancel", 0);

#if 0
printf("width = %d || %d height = %d\n",
	S_BUTTON_PAD + 8 + (buttonfont->max_bounds.width * 50),
	S_BUTTON_W * 3 + S_BUTTON_PAD * 5,
	y + S_BUTTON_H + S_BUTTON_PAD);
#endif


	/* Create the replace query window */
	/* SAM INCOMPLETE! */
	attr.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask;
	size.width = S_BUTTON_PAD * 3 + S_BUTTON_W * 2;
	size.height = S_BUTTON_PAD * 3 + S_BUTTON_H * 2;
	Query = XCreateWindow(display,  RootWindow(display, screen),
		size.x + 50, size.y + 50, size.width, size.height,
		2, CopyFromParent, InputOutput, CopyFromParent,
		attr_mask, &attr);

{	/* SAM */
	XWMHints wm_hints;

	/* This allows keyboard input */
	wm_hints.flags = InputHint;
	wm_hints.input = True;

	argv[0] = "popup";
	XSetWMProperties(display, Query, 0, 0, argv, argc, &size, &wm_hints, 0);
} /* SAM */

	XStoreName(display, Query, "Query Window");

	/* Create the buttons */
	y = x = S_BUTTON_PAD;
	NewButton(Query, x, y, S_BUTTON_W, S_BUTTON_H, "Replace", 0);
	x += S_BUTTON_W + S_BUTTON_PAD;
	NewButton(Query, x, y, S_BUTTON_W, S_BUTTON_H, "Replace All", 0);
	y += S_BUTTON_H + S_BUTTON_PAD;
	x = S_BUTTON_PAD;
	NewButton(Query, x, y, S_BUTTON_W, S_BUTTON_H, "Skip", 0);
	x += S_BUTTON_W + S_BUTTON_PAD;
	NewButton(Query, x, y, S_BUTTON_W, S_BUTTON_H, "Abort", 0);

	/* We wait for exposure so draw commands will work. */
	XMaskEvent(display, ExposureMask, &event);

	DrawButtons();
}


#define ReplaceSelected()	(buttons[3].pushed || buttons[4].pushed || \
							 buttons[5].pushed)

void SearchEvent(event)
XEvent *event;
{
	static char buff[80], replace[80];
	static int bn = 0, rn = 0;
	KeySym key;
	char c;
	int n;

	switch(event->type)
	{
	case Expose:
		DrawButtons();
		break;
	case ButtonPress:
		if((n = FindButton(event->xbutton.window)) == -1) break;
		if(n < 6)
		{	/* first six buttons exclusive */	
			if(buttons[n].pushed == 0)
			{
				int i;
				
				buttons[n].pushed = 1;
				DrawButton(&buttons[n]);
				
				for(i = 0; i < 6; ++i)
					if(buttons[i].pushed && i != n)
					{
						buttons[i].pushed = 0;
						DrawButton(&buttons[i]);
					}
					
				if(n < 3) Showcursor(-3, ReplaceWind);
			}
		}
		else
		{
			buttons[n].pushed ^= 1;
			DrawButton(&buttons[n]);
		}
		break;
	case ButtonRelease:
		if((n = FindButton(event->xbutton.window)) == -1) break;
		if(buttons[n].type == BT_PUSH)
		{
			buttons[n].pushed ^= 1;
			DrawButton(&buttons[n]);
		}
		if(n == 8)	/* start */
			StartSearch(buff, bn, replace, rn);
		else if(n == 9)	/* cancel */
		{
			XUnmapWindow(display, Search);
			SearchMapped = False;
		}
		break;
	case KeyPress:
		/* SAM YUK! */
		XLookupString((XKeyEvent *)event, &c, 1, &key, 0);
		if(event->xany.window == TextWind)
		{
			if((key >= XK_space && key <= XK_asciitilde))
			{
				buff[bn++] = c;
				XDrawImageString(display, TextWind, TextGC,
					4,
					(S_BUTTON_H - buttonfont->descent -
					buttonfont->ascent) / 2 + buttonfont->ascent, buff, bn);
				Showcursor(bn, TextWind);
			}
			else if(key == XK_BackSpace)
			{
				if(bn == 0) { Tbell(); break; }
				buff[--bn] = ' ';
				XDrawImageString(display, TextWind, TextGC,
					4,
					(S_BUTTON_H - buttonfont->descent - buttonfont->ascent) / 2
					+ buttonfont->ascent, buff, bn + 1);
				Showcursor(bn, TextWind);
			}
			else if(key == XK_Return)
				StartSearch(buff, bn, replace, rn);
			else Tbell();
		}
		else if(event->xany.window == ReplaceWind)
		{
			if((key >= XK_space && key <= XK_asciitilde))
			{
				replace[rn++] = c;
				XDrawImageString(display, ReplaceWind, TextGC,
					4,
					(S_BUTTON_H - buttonfont->descent -
					buttonfont->ascent) / 2 + buttonfont->ascent, replace, rn);
				Showcursor(rn, ReplaceWind);
			}
			else if(key == XK_BackSpace)
			{
				if(rn == 0) { Tbell(); break; }
				replace[--rn] = ' ';
				XDrawImageString(display, ReplaceWind, TextGC,
					4,
					(S_BUTTON_H - buttonfont->descent - buttonfont->ascent) / 2
					+ buttonfont->ascent, replace, rn + 1);
				Showcursor(rn, ReplaceWind);
			}
#ifdef SAM_NOT_YET_CAUSES_PROBLEMS_WITH_REPLACE_ALL
			else if(key == XK_Return)
				StartSearch(buff, bn, replace, rn);
#endif
			else Tbell();
		}
		break;
	case EnterNotify:
		if(event->xany.window == TextWind || ReplaceSelected())
			Showcursor(-1, event->xany.window);
		break;
	case LeaveNotify:
		if(event->xany.window == ReplaceWind && !ReplaceSelected())
			Showcursor(-3, event->xany.window);
		else
			Showcursor(-2, event->xany.window);
		break;
	}
}

StartSearch(buff, bn, replace, rn)
char *buff, *replace;
int bn, rn;
{
	/* Note that rn may be 0 if user wants to delete all occurances! */
	if(bn == 0)
	{	/* nothing to do! */
		Tbell();
		return;
	}
	buff[bn] = '\0';
	replace[rn] = '\0';

	if(buttons[6].pushed)
		Curbuff->bmode &= ~EXACT;
	else
		Curbuff->bmode |= EXACT;

	if(buttons[0].pushed)
		Xsearch(buff, FORWARD,  buttons[7].pushed);
	else if(buttons[1].pushed)
		Xsearch(buff, BACKWARD, buttons[7].pushed);
	else if(buttons[2].pushed)
		Xsearch(buff, REGEXP,   buttons[7].pushed);
	else if(buttons[3].pushed)
		Xreplace(buff, replace, FORWARD, buttons[7].pushed);
	else if(buttons[4].pushed)
		Xreplace(buff, replace, QUERY,   buttons[7].pushed);
	else if(buttons[5].pushed)
		Xreplace(buff, replace, REGEXP,  buttons[7].pushed);
	Refresh();
}


/* n == -1	entered window, show cursor
 * n == -2  left window, show empty cursor
 * n == -3  window unselected, no cursor
 * n >=  0 cursor moving
 */
Showcursor(n, wind)
int n;
Window wind;
{
	static int was1 = 0, was2 = 0;
	int *was = wind == TextWind ? &was1 : &was2;
	
	int height = buttonfont->descent + buttonfont->ascent;
	int y = (S_BUTTON_H - height) / 2;

	if(n == -1) n = *was;
	if(n == -2 || n == -3)
	{	/* hide cursor */
		XSetForeground(display, TextGC, background);
		XFillRectangle(display, wind, TextGC,
			*was * buttonfont->max_bounds.width + 4, y,
			buttonfont->max_bounds.width, height);
		XSetForeground(display, TextGC, foreground);
		if(n == -2)
			XDrawRectangle(display, wind, TextGC,
				*was * buttonfont->max_bounds.width + 4, y,
				buttonfont->max_bounds.width - 1, height - 1);
		return;
	}
	if(*was > n)
	{
		XSetForeground(display, TextGC, background);
		XFillRectangle(display, wind, TextGC,
			*was * buttonfont->max_bounds.width + 4, y,
			buttonfont->max_bounds.width, height);
		XSetForeground(display, TextGC, foreground);
	}
	XFillRectangle(display, wind, TextGC,
		n * buttonfont->max_bounds.width + 4, y,
		buttonfont->max_bounds.width, height);
	*was = n;
}

DrawButtons()
{
	int i;
	
	for(i = 0; i < nButtons; ++i)
		DrawButton(&buttons[i]);
}

/* Outline a button. */
static void ButtonOutline(wind, gc, x, y, w, h, pushed)
Window wind;
GC gc;
int x, y, w, h, pushed;
{
	w += x;
	h += y;

	/* Draw Top bevel */
	XSetForeground(display, gc, pushed ? dimgrey : white);
	XDrawLine(display, wind, gc, x,   y,   w,   y);
	XDrawLine(display, wind, gc, x,   y+1, w-1, y+1);
	XDrawLine(display, wind, gc, x,   y+2, w-2, y+2);
	XDrawLine(display, wind, gc, x,   y,   x,   h);
	XDrawLine(display, wind, gc, x+1, y+1, x+1, h-1);
	XDrawLine(display, wind, gc, x+2, y+2, x+2, h-2);

	/* Draw Bottom Bevel */
	XSetForeground(display, gc, pushed ? white : dimgrey);
	XDrawLine(display, wind, gc, x,   h,   w,   h);
	XDrawLine(display, wind, gc, x+1, h-1, w-1, h-1);
	XDrawLine(display, wind, gc, x+2, h-2, w-2, h-2);
	XDrawLine(display, wind, gc, w,   h,   w,   y);
	XDrawLine(display, wind, gc, w-1, h-1, w-1, y+1);
	XDrawLine(display, wind, gc, w-2, h-2, w-2, y+2);
}


int DrawButton(button)
aButton *button;
{
	Window w = button->window;
	GC gc = button->gc;

	if(button->type == BT_PUSH)
	{	
		ButtonOutline(w, gc, 0, 0, button->w, button->h, button->pushed);
	
		/* Draw Text */
		XSetForeground(display, gc, black);
		CenterText(w, gc, button->w, button->h, buttonfont, button->text);
	}
	else if(button->type == BT_TOGGLE)
	{	
		int fontheight = buttonfont->ascent + buttonfont->descent;
		int y = (button->h - fontheight) / 2;

		ButtonOutline(w, gc, 0, y, fontheight, fontheight, button->pushed);

		/* Draw Text */
		XSetForeground(display, gc, black);
		XDrawImageString(display, w, gc,
			fontheight + 8, y + buttonfont->ascent,
			button->text, strlen(button->text));
	}

	if(button->pushed & 2)
	{
		int i;
		
		XSetForeground(display, gc, lightgrey);
		for(i = 1; i < button->h; i += 3)
			XDrawLine(display, w, gc, 0, i, button->w, i);
	}
}


int CenterText(w, gc, bwidth, bheight, font, text)
Window w;
GC gc;
int bwidth, bheight;
XFontStruct *font;
char *text;
{
	int	x, y, width, height;
	int len = strlen(text);

	height = font->descent + font->ascent;
	width = XTextWidth(font, text, len);

	x = (bwidth - width) / 2;
	y = (bheight - height) / 2 + font->ascent;
	XDrawImageString(display, w, gc, x, y, text, len);
}

Xsearch(buff, type, global)
char *buff;
int type, global;
{
	extern char old[];
	extern Boolean searchdir[];

	strcpy(old, buff);
	searchdir[0] = type;
	Arg = 1;
	Dosearch();
}

Xreplace(buff, replace, type, global)
char *buff, *replace;
int global;
{
	extern char old[], new[];
	
	strcpy(old, buff);
	strcpy(new, replace);
	
	Argp = global;

	Popped = True;
	Doreplace(type);
	Popped = False;
}


#ifdef __STDC__
char GetQueryCmd(char prev)
#else
char GetQueryCmd(prev)
char prev;
#endif
{
	XEvent event;
	int n;

	if(!Popped)
	{
		if(prev != '?') Echo("Replace? ");
		Refresh();
		return Tgetcmd();
	}

	if(!QueryMapped)
	{
		XMapWindow(display, Query);
		XMaskEvent(display, ExposureMask, &event);
		QueryMapped = True;
		DrawButtons();
	}
	
	Refresh();	/* Show selection */

	/* GRAB POINTER??? */
	while(1)
	{
		ShowCursor(TRUE);
		XMaskEvent(display, ButtonPressMask, &event);
		if((n = FindButton(event.xbutton.window)) == -1 || n < 10)
			continue;
		buttons[n].pushed = 1;
		DrawButton(&buttons[n]);
		XMaskEvent(display, ButtonReleaseMask, &event);
		ShowCursor(FALSE);
		buttons[n].pushed = 0;
		DrawButton(&buttons[n]);
		switch(n)
		{
			case 10: return 'y';
			case 11: return '!';
			case 12: return 'n';
			case 13: return 'q';
		}
	}
}

void QueryDone()
{
	if(QueryMapped)
	{
		XUnmapWindow(display, Query);
		QueryMapped = 0;
	}
}
#endif

void QueryPointer(x, y)
int	*x, *y;
{
	extern Display *display;
	Window	rootwindow, childwindow;
	int		rootx, rooty;
	int		childx, childy;
	unsigned int button_state;

	if(XQueryPointer(display, RootWindow(display, DefaultScreen(display)),
			&rootwindow, &childwindow,
			&rootx, &rooty,
			&childx, &childy,
			&button_state) == True)
	{
		*x = rootx;
		*y = rooty;
	}
	else
		*x = *y = 0;
}
