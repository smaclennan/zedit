#if XWINDOWS
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
#ifdef BORDER3D
extern GC PAWgc;
#endif
extern Boolean HasColor;
extern int foreground, background;
extern int border_width, highlight;
extern int win_width, win_height;

extern char *KeyNames[];

Window CreateWindow ARGS((Window parent, int x, int y, int w, int h, long ev));
char *GetResource ARGS((char *name, char *class));
void initSockets ARGS((int xfd));

/* defined in xscroll.c */
#ifdef SCROLLBARS
void ScrollEvent ARGS((XEvent *event));
#ifdef _buff_h
void VscrollEvent ARGS((XEvent *event, WDO *wdo));
void HscrollEvent ARGS((XEvent *event, WDO *wdo));
#endif
# ifdef BORDER3D
extern int ScrollBarWidth;
extern int SBborderwidth, SBhighlight;
#  define SCROLLBAR_WIDTH		ScrollBarWidth
# else
#  define SCROLLBAR_WIDTH		11
# endif
#else
#define SCROLLBAR_WIDTH		0
#endif

/* some handy height macros */
#define H_MENUBAR		(fontheight + border_width * 2)
#define H_MODELINE		H_MENUBAR
#define H_PAW			(fontheight + border_width * 2)
#define Y_PAW(h)		((h) - border_width * 2 - fontheight)

/* Struct for 3d borders. */

typedef struct
{
	XColor *bgColorPtr;		/* Background (middle) color. */
	GC bgGC;
	XColor *darkColorPtr;	/* Color for darker areas. */
	GC darkGC;
	XColor * lightColorPtr;	/* Color used for lighter areas. */
	GC lightGC;
	Pixmap shadow;			/* Stipple pattern to use for drawing
							 * shadows areas.  Used for displays with
							 * <= 64 colors or where colormap has
							 * filled up.
							 */
} Border;

#define TK_RELIEF_RAISED	1
#define TK_RELIEF_FLAT		2
#define TK_RELIEF_SUNKEN	4
#define TK_RELIEF_GROOVE	8
#define TK_RELIEF_RIDGE		16

/* from tk3d.c */
void Tk_Fill3DPolygon(Window tkwin, Drawable drawable, Border *borderPtr, 
    XPoint *pointPtr, int numPoints, int borderWidth, int leftRelief);
void Tk_Fill3DRectangle(Window tkwin, Drawable drawable, Border *borderPtr,
    int x, int y, int width, int height, int borderWidth, int relief);
void Tk_Draw3DRectangle(Window tkwin, Drawable drawable, Border *borderPtr,
    int x, int y, int width, int height, int borderWidth, int relief);

/* Struct for scrollbars */

typedef struct {
    int vertical;		/* Non-zero means vertical orientation */
	Window win;			/* window to draw in */
	int height;
    int borderWidth;	/* Width of 3-D borders. */
    Border *bgBorder;	/* For drawing all flat surfaces except trough. */
	Border *trough;		/* For drawing trough */
    GC copyGC;			/* Used for copying from pixmap onto screen. */
    int highlightWidth;	/* Width in pixels of highlight to draw. */
    int inset;			/* Total width of all borders */
    int elementBorderWidth;	/* Width of border to draw around elements
				 * inside scrollbar (arrows and slider).
				 * -1 means use borderWidth. */
    int arrowLength;	/* Length of arrows along long dimension of
						 * scrollbar, including space for a small gap
						 * between the arrow and the slider.
						 * Recomputed on window size changes. */
    int sliderFirst;
    int sliderLast;
    int activeField;	/* Names field to be displayed as active (pushed) */

	double firstFraction;	/* first visible thing in window.
							 * fraction between 0 and 1.0.
							 */
	double lastFraction;	/* last visible thing */
} Scrollbar;

/*
 * Legal values for "activeField" field of Scrollbar structures.  These
 * are also the return values from the ScrollbarPosition procedure.
 */

#define OUTSIDE		0
#define TOP_ARROW	1
#define TOP_GAP		2
#define SLIDER		3
#define BOTTOM_GAP	4
#define BOTTOM_ARROW	5

/* routines from tkScrollbar.c */
void ComputeScrollbarGeometry(Scrollbar *scrollPtr);
void DisplayScrollbar(Scrollbar *scrollPtr);
int ScrollbarPosition(Scrollbar *scrollPtr, int x, int y);

#endif
