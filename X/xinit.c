/* xinit.c - Zedit X initialization
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#include "xwind.h"

/* This file contains the routines that work with the databases */

Display *display;
int screen;
static XrmDatabase xDB;

/* Set in load_font */
int fontwidth, fontheight, fontbase, fontid, boldid;
int Xrow[ROWMAX + 1], Xcol[COLMAX + 1];

/* command line options table */
static XrmOptionDescRec opTable[] = {
	{ "-bw",	".borderWidth",	 XrmoptionSepArg, (caddr_t)NULL },
	{ "-display",	".display",	 XrmoptionSepArg, (caddr_t)NULL },
	{ "-font",	"*font",	 XrmoptionSepArg, (caddr_t)NULL },
	{ "-bold",	"*boldfont",	 XrmoptionSepArg, (caddr_t)NULL },

	{ "-fg",	"*foreground",	 XrmoptionSepArg, (caddr_t)NULL },
	{ "-bg",	"*background",	 XrmoptionSepArg, (caddr_t)NULL },
	{ "-cr",	"*cursorColor",	 XrmoptionSepArg, (caddr_t)NULL },
	{ "-ms",	"*markColor",	 XrmoptionSepArg, (caddr_t)NULL },
	{ "-cc",	"*commentColor", XrmoptionSepArg, (caddr_t)NULL },

	{ "-geometry",	".geometry",	 XrmoptionSepArg, (caddr_t)NULL },
	{ "-geo",	".geometry",	 XrmoptionSepArg, (caddr_t)NULL },
	{ "-iconic",	".iconic",	 XrmoptionNoArg,  (caddr_t)"on" },
	{ "-geom",	"*iconGeometry", XrmoptionSepArg, (caddr_t)NULL },
	{ "-ps",	"*pointerShape", XrmoptionSepArg, (caddr_t)NULL },
};
#define OPTABLESZ	(sizeof(opTable) / sizeof(XrmOptionDescRec))

static char *Appname;

static void MergeDatabaseFile(char *fname)
{
	if (fname) {
		XrmDatabase DB = XrmGetFileDatabase(fname);
		if (DB)
			XrmMergeDatabases(DB, &xDB);
	}
}


void Xinit(char *app,
	   int  *argc,		/* returns updated */
	   char **argv)
{
	XrmDatabase cmdDB = NULL;
	char fname[1024], *p;
	char *env;
	char *displayname;

	/* for GetResource - must be lowercase */
	Appname = strdup(app);
	for (p = Appname; *p; ++p)
		if (isupper(*p))
			*p = tolower(*p);
	*Appname = toupper(*Appname);

	/* se we can use resource manager data merge functions */
	XrmInitialize();

	/* parse the command line, store any options in a database */
	XrmParseCommand(&cmdDB, opTable, OPTABLESZ, Appname, argc, argv);

	/* get and open display now since we need it to get other databases */
	xDB = cmdDB;
	displayname = GetResource(".display", ".Display");
	xDB = NULL;
	display = XOpenDisplay(displayname);
	if (!display) {
		fprintf(stderr, "%s: Cannot connect to X server %s\n",
			argv[0], XDisplayName(displayname));
		exit(1);
	}
	displayname = XDisplayName(displayname);

	screen = DefaultScreen(display);

	/* get the applications defaults file, if any */
	sprintf(fname, "/usr/lib/X11/app-defaults/%s", Appname);
	MergeDatabaseFile(fname);

#ifdef SAM_WHAT_TO_DO
	/* Merge server defaults (created by xrdb) loaded as a
	 * property of the root window when the server initializes, and
	 * loaded into the display structure on XOpenDisplay.
	 */
	MergeDatabaseFile(display->xdefaults);
#endif

	env = (char *)getenv("HOME");
	if (!env)
		env = ".";
	sprintf(fname, "%s/.Xdefaults", env);
	MergeDatabaseFile(fname);

	/* open XENVIRONMENT file, or if not defined, the .Xdefaults */
	env = (char *)getenv("XENVIRONMENT");
	if (env == NULL) {
		int len = strlen(fname);
		gethostname(fname + len, 1024 - len);
		env = fname;
	}
	MergeDatabaseFile(env);

	/* command line options take precedence over everything */
	XrmMergeDatabases(cmdDB, &xDB);

	env = GetResource(".borderwidth", ".BorderWidth");
	if (env)
		border_width = atoi(env);
	env = GetResource(".highlight", ".HighLight");
	if (env)
		highlight = atoi(env);

	initSockets(ConnectionNumber(display));
}


/* Given a resource database, the resource name, and the resource class,
 * return the resouce value as a string.  Appname is always prepended.
 * It returns NULL or a statically allocated name.
 */
char *GetResource(char *name, char *class)
{
	XrmValue value;
	char *type;
	char fullname[128], fullclass[128];

	sprintf(fullclass, "%s%s", Appname, class);
	sprintf(fullname,  "%s%s", Appname, name);
	if (XrmGetResource(xDB, fullname, fullclass, &type, &value) == True)
		return value.addr;
	return NULL;
}

int GetXColor(char *color_name, XColor *color)
{
	Colormap cmap = DefaultColormap(display, screen);
	XColor def;

	if (XAllocNamedColor(display, cmap, color_name, &def, color))
		return 1;

	printf("Unable to allocate color %s\n", color_name);
	return 0;
}

int GetColor(char *color_name, int *pixel)
{
	XColor color;

	if (GetXColor(color_name, &color)) {
		*pixel = color.pixel;
		return 1;
	}
	return 0;
}

int ColorResource(char *name, char *class, int *pixel)
{
	char *color_name;

	color_name = GetResource(name, class);
	if (color_name == NULL)
		return 0;
	return GetColor(color_name, pixel);
}

/* Load a font and setup the global variables needed by Zedit.
 * Take care not to destroy old values if fontname is invalid.
 */
XFontStruct *LoadFontByName(char *fontname)
{
	static XFontStruct *font_info;
	XFontStruct *info;
	int i;

	info = XLoadQueryFont(display, fontname);
	if (info == NULL)
		return NULL;

	fontwidth  = info->max_bounds.width;
	fontheight = info->descent + info->ascent;
	fontbase   = info->ascent;
	fontid	   = info->fid;

	if (font_info) {
		/* we are changing the font */
		/* free the old font information */
		XFreeFont(display, font_info);

		/* change the fontid for all character gcs
		 * note that curgc just points to another gc
		 */
		XSetFont(display, normgc,	fontid);
		XSetFont(display, revgc,	fontid);
		XSetFont(display, boldgc,	fontid);
		XSetFont(display, commentgc,	fontid);
		XSetFont(display, cppgc,	fontid);
		XSetFont(display, cppifgc,	fontid);
		XSetFont(display, cursorgc,	fontid);
		XSetFont(display, markgc,	fontid);
		XSetFont(display, modegc,	fontid);

		/* resize the root window */
		win_width  = Colmax * fontwidth;
		win_height = Rowmax * fontheight;
#ifdef SCROLLBARS
		win_width += SCROLLBAR_WIDTH;
# ifdef HSCROLL
		win_height += SCROLLBAR_WIDTH;
# endif
#endif
		XResizeWindow(display, Zroot, win_width, win_height);
	}

	/* Precalc the row/col to pixel conversions. */
	Xrow[0] = Xcol[0] = 0;
	for (i = 1; i <= ROWMAX; ++i) {
		Xrow[i] = Xrow[i-1] + fontheight;
		Xcol[i] = Xcol[i-1] + fontwidth;
	}
	for ( ; i <= COLMAX; ++i)
		Xcol[i] = Xcol[i-1] + fontwidth;

	return font_info = info;
}

XFontStruct *LoadFonts(void)
{
	XFontStruct *font_info;
	char *fontname;

	/* SAM ignore bold font for now. */
	boldid = 0;

	if (VARSTR(VFONT))
		fontname = VARSTR(VFONT);
	else {
		fontname = GetResource(".font", ".Font");
		if (fontname == NULL)
			fontname = "fixed";
	}

	font_info = LoadFontByName(fontname);
	if (!font_info) {
		printf("Unable to load font %s\n", fontname);
		exit(1);
	}

	VARSTR(VFONT) = strdup(fontname);

	return font_info;
}

void xusage(void)
{
	puts("X11 options:");
	puts("\t-display	set display name");
	puts("\t-geometry	set geometry");
	puts("\t-geo		set geometry");
	puts("\t-font		font");
	puts("\t-bold		boldfont");
	puts("\t-fg		foreground");
	puts("\t-bg		background");
	puts("\t-cr		cursorColor");
	puts("\t-ms		markColor");
	puts("\t-cc		commentColor");
	puts("\t-iconic		start iconic");
	puts("\t-geom		icon geometry");
	puts("\t-ps		pointerShape");
	puts("\t-bw		borderWidth");
}
