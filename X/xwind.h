/* xwind.h - Zedit main X include file
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
#ifndef XWIND_H
#define XWIND_H

#include <X11/Xlib.h>

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

extern GC normgc, revgc, boldgc, commentgc, cppgc, cppifgc;
extern GC cursorgc, markgc, modegc;

extern int Hshift;

extern char *KeyNames[];

extern char *PromptString;

Window createwindow(Window parent, int x, int y, int w, int h, long ev);
char *getresource(char *name, char *class);

/* defined in xscroll.c */
#ifdef SCROLLBARS
void scrollevent(XEvent *event);
#define SCROLLBAR_WIDTH		11
#else
#define SCROLLBAR_WIDTH		0
#endif

void addwindowsizes(char *str);
void createscrollbars(struct wdo *wdo);
void deletescrollbars(struct wdo *wdo);
void resizescrollbars(struct wdo *wdo);
void xflush();

char *xeventname(int type);
char *xwindowname(Window window);

char *KeyToName(int, char *);
void showcursor(Boolean);
void showmark(Boolean);
int colour_resource(char *name, char *class, int *pixel);
void tputchar(char);
int getcolor(char *, int *);
XFontStruct *load_fonts(void);
void xinit(char *app, int *argc, char **argv);
XFontStruct *load_font_by_name(char *fontname);

void xusage(void);

void audioExit(void);

void xfindtag(void);

/* Keep these around in case we bring back the tcl code */
#define initsockets(x)
#define closesockets()
#define cleanupsocket(i)
#define processFDs()
#define xaddbuffer(b)
#define xswitchto(b)
#define xdeletebuffer(b)

#endif
#endif
