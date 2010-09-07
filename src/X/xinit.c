#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#include "../global.h"
#include "../config.h"
#include "xwind.h"
#include "../vars.h"

/* This file contains the routines that work with the databases */

Display *display;
int screen;
static XrmDatabase xDB;

char *GetResource();

/* Set in load_font */
int fontwidth, fontheight, fontbase, fontid, boldid;
int Xrow[ROWMAX + 1], Xcol[COLMAX + 1];

/* command line options table */
static XrmOptionDescRec opTable[] = {
{"-bw",				".borderWidth",		XrmoptionSepArg,	(caddr_t) NULL},
{"-display",		".display",			XrmoptionSepArg,	(caddr_t) NULL},
{"-font",			"*font",			XrmoptionSepArg,	(caddr_t) NULL},
{"-bold",			"*boldfont",		XrmoptionSepArg,	(caddr_t) NULL},
	
/* colors */
{"-fg",				"*foreground",		XrmoptionSepArg,	(caddr_t) NULL},
{"-bg",				"*background",		XrmoptionSepArg,	(caddr_t) NULL},
{"-cr",				"*cursorColor",		XrmoptionSepArg,	(caddr_t) NULL},
{"-ms",				"*markColor",		XrmoptionSepArg,	(caddr_t) NULL},
{"-cc",				"*commentColor",	XrmoptionSepArg,	(caddr_t) NULL},

{"-geometry",		".geometry",		XrmoptionSepArg,	(caddr_t) NULL},
{"-geo",			".geometry",		XrmoptionSepArg,	(caddr_t) NULL},
{"-iconic",			".iconic",			XrmoptionNoArg,		(caddr_t) "on"},
{"-geom",			"*iconGeometry",	XrmoptionSepArg,	(caddr_t) NULL},
{"-ps",				"*pointerShape",	XrmoptionSepArg,	(caddr_t) NULL},
};
#define OPTABLESZ	(sizeof(opTable) / sizeof(XrmOptionDescRec))

static char *Appname;

static void MergeDatabaseFile(fname)
char *fname;
{
	XrmDatabase DB;

	if(fname && (DB = XrmGetFileDatabase(fname)) != NULL)
		XrmMergeDatabases(DB, &xDB);
}


void Xinit(app, argc, argv)
char *app;
int  *argc;		/* returns updated */
char **argv;
{
	extern int gethostname();
	extern int border_width, highlight;
	XrmDatabase cmdDB = NULL;
	char fname[1024], *p;
	char *env;
	char *displayname;

	/* for GetResource - must be lowercase */
	Appname = strdup(app);
	for(p = Appname; *p; ++p) if(isupper(*p)) *p = tolower(*p);
	*Appname = toupper(*Appname);

	/* se we can use resource manager data merge functions */
	XrmInitialize();

	/* parse the command line, store any options in a database */
	XrmParseCommand(&cmdDB, opTable, OPTABLESZ, Appname, argc, argv);

	/* get and open display now since we need it to get other databases */
	xDB = cmdDB;
	displayname = GetResource(".display", ".Display");
	xDB = NULL;
	if((display = XOpenDisplay(displayname)) == NULL)
	{
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

	if((env = (char *)getenv("HOME")) == NULL) env = ".";
	sprintf(fname, "%s/.Xdefaults", env);
	MergeDatabaseFile(fname);

	/* open XENVIRONMENT file, or if not defined, the .Xdefaults */
	if((env = (char *)getenv("XENVIRONMENT")) == NULL)
	{
		int len;
		
		len = strlen(fname);
		gethostname(fname + len, 1024 - len);
		env = fname;
	}
	MergeDatabaseFile(env);
	
	/* command line options take precedence over everything */
	XrmMergeDatabases(cmdDB, &xDB);
	
	if((env = GetResource(".borderwidth", ".BorderWidth")) != NULL)
		border_width = atoi(env);
	if((env = GetResource(".highlight", ".HighLight")) != NULL)
		highlight = atoi(env);
		
	initSockets(ConnectionNumber(display));
}


/* Given a resource database, the resource name, and the resource class,
 * return the resouce value as a string.  Appname is always prepended.
 * It returns NULL or a statically allocated name.
 */
char *GetResource(name, class)
char *name, *class;
{
	XrmValue value;
	char *type;
	char fullname[128], fullclass[128];

	sprintf(fullclass, "%s%s", Appname, class);
	sprintf(fullname,  "%s%s", Appname, name);
	if(XrmGetResource(xDB, fullname, fullclass, &type, &value) == True)
		return value.addr;
	return NULL;
}

int GetXColor(color_name, color)
char *color_name;
XColor *color;
{		
	Colormap cmap = DefaultColormap(display, screen);

#if 0
	if(XParseColor(display, cmap, color_name, color) &&
	   XAllocColor(display, cmap, color))
		return 1;
#else
	XColor def;

	if(XAllocNamedColor(display, cmap, color_name, &def, color))
		return 1;
#endif
	printf("Unable to allocate color %s\n", color_name);
	return 0;
}

int GetColor(color_name, pixel)
char *color_name;
int *pixel;
{		
	XColor color;
	
	if(GetXColor(color_name, &color))
	{
		*pixel= color.pixel;
		return 1;
	}
	return 0;
}

int ColorResource(name, class, pixel)
char *name, *class;
int *pixel;
{
	char *color_name;

	if((color_name = GetResource(name, class)) == NULL)
		return 0;
	return GetColor(color_name, pixel);
}


#if XWINDOWS
/* Load a font and setup the global variables needed by Zedit.
 * Take care not to destroy old values if fontname is invalid.
 */
XFontStruct *LoadFontByName(char *fontname)
{
	static XFontStruct *font_info = 0;
	XFontStruct *info;
	int i;

	if((info = XLoadQueryFont(display, fontname)) == NULL)
		return 0;

	fontwidth  = info->max_bounds.width;
	fontheight = info->descent + info->ascent;
	fontbase   = info->ascent;
	fontid	   = info->fid;

	if(font_info)
	{	/* we are changing the font */
		extern GC normgc, revgc, boldgc, commentgc, cppgc, cppifgc;
		extern GC cursorgc, markgc, modegc;
		extern int win_width, win_height;
		extern size_t Colmax, Rowmax;

		/* free the old font information */
		XFreeFont(display, font_info);
		
		/* change the fontid for all character gcs
		 * note that curgc just points to another gc
		 */
		XSetFont(display, normgc,	fontid);
		XSetFont(display, revgc,	fontid);
		XSetFont(display, boldgc,	fontid);
		XSetFont(display, commentgc,fontid);
		XSetFont(display, cppgc,	fontid);
		XSetFont(display, cppifgc,	fontid);
		XSetFont(display, cursorgc,	fontid);
		XSetFont(display, markgc,	fontid);
		XSetFont(display, modegc,	fontid);

		/* resize the root window */
#ifdef BORDER3D
 		win_width  = (Colmax  * fontwidth) + (border_width * 4);
		win_height = (win_height * fontheight) + (border_width * 4);
#else
		win_width  = Colmax * fontwidth;
		win_height = Rowmax * fontheight;
# ifdef SCROLLBARS
		win_width += SCROLLBAR_WIDTH;
#  ifdef HSCROLL
		win_height += SCROLLBAR_WIDTH;
#  endif
# endif
#endif
		XResizeWindow(display, Zroot, win_width, win_height);
	}

	/* Precalc the row/col to pixel conversions. */
	Xrow[0] = Xcol[0] = 0;
	for(i = 1; i <= ROWMAX; ++i)
	{
		Xrow[i] = Xrow[i-1] + fontheight;
		Xcol[i] = Xcol[i-1] + fontwidth;
	}
	for( ; i <= COLMAX; ++i) Xcol[i] = Xcol[i-1] + fontwidth;

	return font_info = info;
}

XFontStruct *LoadFonts()
{
	XFontStruct *font_info;
	char *fontname;

	/* SAM ignore bold font for now. */
	boldid = 0;	

	if(Vars[VFONT].val)
		fontname = (char *)Vars[VFONT].val;
	else if((fontname = GetResource(".font", ".Font")) == NULL)
		fontname = "fixed";
		
	if((font_info = LoadFontByName(fontname)) == 0)
	{
		printf("Unable to load font %s\n", fontname);
		exit(1);
	}
	
	Vars[VFONT].val = (Word)strdup(fontname);

	return font_info;
}
#else
/* loads the fonts */
XFontStruct *LoadFonts()
{
	XFontStruct *font_info;
	char *fontname;
	int i;
	
	if((fontname = GetResource(".font", ".Font")) == NULL)
		fontname = "fixed";
	if((font_info = XLoadQueryFont(display, fontname)) == NULL)
	{
		fprintf(stderr, "Cannot open %s font.\n", fontname);
		exit(1);
	}
	
	fontwidth  = font_info->max_bounds.width;
	fontheight = font_info->descent + font_info->ascent;
	fontbase   = font_info->ascent;
	fontid	   = font_info->fid;
	/* If a bold font is specified, it must match the size of the normal
	 * font. We allow the bold font to be 1 pixel higher.
	 */
	boldid = 0;
	if((fontname = GetResource(".boldfont", ".BoldFont")) != NULL)
		if((font_info = XLoadQueryFont(display, fontname)) != NULL)
			if(	fontwidth  == font_info->max_bounds.width &&
				fontbase   == font_info->ascent &&
				(fontheight == font_info->descent + fontbase ||
				 fontheight == font_info->descent + fontbase - 1))
			{
				boldid = font_info->fid;
				fontheight = font_info->descent + font_info->ascent;
			}
#if DBGy
			else
			{
				Dbg("Mismatch Font W %d H %d B %d\n",
					fontwidth, fontheight, fontbase);
				Dbg("         Bold W %d H %d B %d\n",
					font_info->max_bounds.width, font_info->descent + fontbase,
					font_info->ascent);
			}
		else Dbg("Unable to load font %s\n", fontname);
#endif
		
	/* Precalc the row/col to pixel conversions. */
	Xrow[0] = Xcol[0] = 0;
	for(i = 1; i <= ROWMAX; ++i)
	{
		Xrow[i] = Xrow[i-1] + fontheight;
		Xcol[i] = Xcol[i-1] + fontwidth;
	}
	for( ; i <= COLMAX; ++i) Xcol[i] = Xcol[i-1] + fontwidth;

	return font_info;
}
#endif

void xusage()
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
