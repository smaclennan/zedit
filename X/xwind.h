#include "../z.h"

#if XWINDOWS
#ifndef XWIND_H
#define XWIND_H

#include <X11/Xlib.h>

#ifdef sun4
typedef unsigned long ulong;
#endif

/* defined in xinit.c */
extern Display *display;
extern int screen;
extern int fontheight, fontwidth, fontbase, fontid, boldid;
extern int Xrow[], Xcol[];

/* defined in xwind.c */
extern Window Zroot, zwindow, textwindow, PAWwindow;
extern GC curgc, normgc;
extern Boolean HasColor;
extern int foreground, background;
extern int border_width, highlight;
extern int win_width, win_height;
extern Cursor textcursor, vscrollcursor, hscrollcursor;

extern int Hshift;

extern char *KeyNames[];

extern char *PromptString;

Window CreateWindow(Window parent, int x, int y, int w, int h, long ev);
char *GetResource(char *name, char *class);

/* defined in xscroll.c */
#ifdef SCROLLBARS
void ScrollEvent(XEvent *event);
#define SCROLLBAR_WIDTH		11
#else
#define SCROLLBAR_WIDTH		0
#endif

#if 0
/* some handy height macros */
#define H_MENUBAR		(fontheight + border_width * 2)
#define H_MODELINE		H_MENUBAR
#define H_PAW			(fontheight + border_width * 2)
#define Y_PAW(h)		((h) - border_width * 2 - fontheight)
#endif

void AddWindowSizes(char *str);
void CreateScrollBars(struct wdo *wdo);
void DeleteScrollBars(struct wdo *wdo);
void ResizeScrollBars(struct wdo *wdo);
void Xflush();
XFontStruct *LoadFontByName(char *fontname);

/* Keep these around in case we bring back the tcl code */
#define initSockets(x)
#define closeSockets()
#define CleanupSocket(i)
#define ProcessFDs()
#define XAddBuffer(b)
#define XSwitchto(b)
#define XDeleteBuffer(b)

#endif
#endif
