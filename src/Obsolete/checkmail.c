#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "z.h"
#include "xwind.h"
#include <sys/stat.h>
#include <signal.h>

#ifdef CHECKMAIL
extern Display *display;
extern int screen;
extern int foreground, background;

Window NewMail = 0;

/* SAM These should be global! */
static unsigned long lightgrey, white, black;
static GC gc;
static int x, y, width, height;
static XFontStruct *font;

static void ButtonOutline();

void CreateNewMailWindow()
{
	extern int fontid;

	XGCValues values;
	uint valuemask;
	XSetWindowAttributes attr;
	unsigned long attr_mask;

	if((font = XQueryFont(display, fontid)) == NULL)
	{
		fprintf(stderr, "Cannot open font.\n");
		exit(1);
	}
	
	black = BlackPixel(display, screen);
	white = WhitePixel(display, screen);
	if(!GetColor("LightGrey", &lightgrey)) lightgrey = white;

	attr.background_pixel  = lightgrey;
	attr.event_mask =	ExposureMask |
						ButtonPressMask | ButtonReleaseMask |
						EnterWindowMask | LeaveWindowMask;
	attr.override_redirect = True;
	attr_mask = CWBackPixel | CWEventMask | CWOverrideRedirect;

	/* set defaults */
	x = y = 100;
	width = 100;
	height = 50;
	
	NewMail = XCreateWindow(display,  RootWindow(display, screen),
		x, y, width, height,
		1, CopyFromParent, InputOutput, CopyFromParent,
		attr_mask, &attr);

	valuemask = GCForeground | GCBackground | GCFont;
	values.font = font->fid;
	values.foreground = foreground;
	values.background = lightgrey;
	gc = XCreateGC(display, NewMail, valuemask, &values);
}


static int popped = 0;

void SetupItimer()
{
	extern void checkmail();
	static struct itimerval value;

	signal(SIGALRM, checkmail);
	
	value.it_value.tv_usec		= 0;
	value.it_value.tv_sec		= 2;
	value.it_interval.tv_usec	= 0;
	value.it_interval.tv_sec	= 2;
	setitimer(ITIMER_REAL, &value, 0);
}

void NewMailEvent(XEvent *event)
{
	static int inwindow = 0;

	switch(event->type)
	{
		case Expose:
			ButtonOutline(NewMail, gc, 0, 0, width, height, 0);
			break;

		case ButtonPress:
			ButtonOutline(NewMail, gc, 0, 0, width, height, 1);
			break;

		case ButtonRelease:
			ButtonOutline(NewMail, gc, 0, 0, width, height, 0);
			/* only unmap if mouse still in window */
			if(inwindow)
			{
				popped = 0;
				XUnmapWindow(display, NewMail);
			}
			break;

		case EnterNotify:
			inwindow = 1;
			break;

		case LeaveNotify:
			inwindow = 0;
			break;
	}
}


static void ButtonOutline(wind, gc, x, y, w, h, pushed)
Window wind;
GC gc;
int x, y, w, h, pushed;
{
	w += x;
	h += y;

	/* Draw Top bevel */
	XSetForeground(display, gc, pushed ? black : white);
	XDrawLine(display, wind, gc, x,   y,   w,   y);
	XDrawLine(display, wind, gc, x,   y+1, w-1, y+1);
	XDrawLine(display, wind, gc, x,   y+2, w-2, y+2);
	XDrawLine(display, wind, gc, x,   y,   x,   h);
	XDrawLine(display, wind, gc, x+1, y+1, x+1, h-1);
	XDrawLine(display, wind, gc, x+2, y+2, x+2, h-2);

	/* Draw Bottom Bevel */
	XSetForeground(display, gc, pushed ? white : black);
	XDrawLine(display, wind, gc, x,   h,   w,   h);
	XDrawLine(display, wind, gc, x+1, h-1, w-1, h-1);
	XDrawLine(display, wind, gc, x+2, h-2, w-2, h-2);
	XDrawLine(display, wind, gc, w,   h,   w,   y);
	XDrawLine(display, wind, gc, w-1, h-1, w-1, y+1);
	XDrawLine(display, wind, gc, w-2, h-2, w-2, y+2);

	XSetForeground(display, gc, black);
	CenterText(wind, gc, width, height, font, "New Mail!");
}


void checkmail()
{
	extern char mailbox[];
	extern long lastmail;
	struct stat sbuf;
	
	signal(SIGALRM, checkmail);

	if(popped) return;

	if(stat(mailbox, &sbuf) == 0 && sbuf.st_mtime > lastmail)
	{

		popped = 1;
		lastmail = sbuf.st_mtime;
		XMapWindow(display, NewMail);		
		Tbell();
	}
}


/* SAM Also in xpopup.c */
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
#endif
