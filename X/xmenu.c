#ifdef BORDER3D

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../z.h"
#include "xwind.h"
#include "xkeys.h"

extern Border Zborder;

/* SAM Hardcode for now */
static int MenuBW = 2;

typedef struct Entry {
	char *name;
	Proc (*func)();
	int arg;
	Window *win;
	struct Entry *next;
} Entry;

typedef struct {
	char *name;
	Entry *head;
	Entry *tail;
	int maxlen;
	int nentry;
} Menu;

#define NUMMENUS		2
static Menu Menus[NUMMENUS];
static int maxlen = 0;

static void NewEntry(Menu *menu, char *name, Proc (*func)(), int arg);

static void Docmd(int cmd)
{
	extern unsigned Cmd;
	extern Byte Keys[];
	
	/* For Menu Open we want the popup */
	if(cmd == ZFINDFILE) Argp = 1;

	for(Cmd = 0; Cmd < NUMKEYS; ++Cmd)
		if(Keys[Cmd] == cmd)
			return;

	/* Command is not bound to any key! */
	Cmd = ZABORT;
}

static void MenuFindfile()
{
	Argp = 1;
	Zfindfile();
}

/* The menubar mimics the TextEditor from CDE */
void InitMenuBar()
{
	int n = 0;

	memset(Menus, 0, sizeof(Menus));
	
	/* File menu */
	Menus[n].name  = strdup("File  ");
/* SAM	NewEntry(&Menus[n], 0, Tearoff); */
	NewEntry(&Menus[n], "New",				Docmd, ZNOTIMPL);
	NewEntry(&Menus[n], "Open...",			MenuFindfile, 0);
	NewEntry(&Menus[n], "Include...",		Docmd, ZFILEREAD);
	NewEntry(&Menus[n], "", 0, 0);	
	NewEntry(&Menus[n], "Save",				Docmd, ZFILESAVE);
	NewEntry(&Menus[n], "Save As...",  		Docmd, ZFILEWRITE);
	NewEntry(&Menus[n], "Print...",  		Docmd, ZNOTIMPL);
	NewEntry(&Menus[n], "", 0, 0);	
	NewEntry(&Menus[n], "Close     Alt+F4",	Zexit, 0);
	
	/* Find menu */
	++n;
	Menus[n].name = strdup("Find");
	NewEntry(&Menus[n], "Forward ...",		Docmd, ZSEARCH);
	NewEntry(&Menus[n], "Reverse ...",		Docmd, ZRSEARCH);
	NewEntry(&Menus[n], "Expression ...",	Docmd, ZRESRCH);
	NewEntry(&Menus[n],	"Incremental ...",	Docmd, ZINCSRCH);
	NewEntry(&Menus[n], "Global ...",		Docmd, ZGSEARCH);
	NewEntry(&Menus[n], "", 0, 0);
	NewEntry(&Menus[n], "Again",			Docmd, ZAGAIN);
	
	/* Find the longest menu name */
	for(maxlen = n = 0; n < NUMMENUS; ++n)
	{
		if(strlen(Menus[n].name) > maxlen) maxlen = strlen(Menus[n].name);
		Menus[n].maxlen *= fontwidth;
	}
	maxlen = maxlen * fontwidth + highlight * 2;
}

void DrawMenuBar(x, y, w, h)
int x, y, w, h;
{
	extern GC modegc;
	int i;

	Tk_Fill3DRectangle(Zroot, Zroot, &Zborder, x, y, w, h, MenuBW,
		TK_RELIEF_FLAT);

	x += MenuBW + highlight;
	y += MenuBW + highlight + fontbase;
	for(i = 0; i < NUMMENUS; ++i)
	{	
		XDrawImageString(display, Zroot, modegc, x, y,
			Menus[i].name, strlen(Menus[i].name));
		x += maxlen;
	}
}

static void MenuPopup(menu, x)
Menu *menu;
int x;		/* really just an offset */
{
	extern GC modegc;
	Window popup;
	XEvent event; 
	int y, w, h, i;
	Entry *e;

	/* create the popup window */
	x = x + MenuBW * 2;
	y = H_MENUBAR;
	w = MenuBW * 4 + menu->maxlen;
	h = ((MenuBW * 2 + fontheight) * menu->nentry) + MenuBW * 2;
	popup = CreateWindow(Zroot, x, y, w, h, 0);
	
	/* And outline it */
	Tk_Draw3DRectangle (popup, popup, &Zborder, 0, 0, w, h, MenuBW,
		TK_RELIEF_GROOVE);

	/* Create the entries */
	x = y = MenuBW;
	h = fontheight + MenuBW * 2;
	w -= MenuBW * 2;

#ifdef SAM_NOT_READY_YET
{	/* Draw tear-off line. */
    XPoint points[2];
    int margin, width, maxX;

    margin = fontheight / 2;
    points[0].x = 0;
    points[0].y = h / 2;
    points[1].y = points[0].y;
    width = 6;
    maxX  = w - 1;

    while(points[0].x < maxX)
	{
		points[1].x = points[0].x + width;
		if(points[1].x > maxX) points[1].x = maxX;
		Tk_Draw3DPolygon(popup, popup, &Zborder, points, 2, 1,
			TK_RELIEF_RAISED);
		points[0].x += 2 * width;
    }
	
	y += h;
}
#endif

	for(e = menu->head; e; e = e->next, y += h)
	{
		e->win = CreateWindow(popup, x, y, w, h, e->func == 0 ? 0 :
			ButtonReleaseMask | EnterWindowMask | LeaveWindowMask);

		/* fill in the entry */
		Tk_Fill3DRectangle(e->win, e->win, &Zborder, 0, 0, w, h, MenuBW,
			TK_RELIEF_FLAT);

		if(e->func == 0)
		{	/* seperator */
		    XPoint points[2];

		    points[0].x = 0;
		    points[0].y = h / 2;
		    points[1].x = w - 1;
		    points[1].y = points[0].y;
			Tk_Draw3DPolygon(e->win, e->win, &Zborder, points, 2, 1,
				TK_RELIEF_FLAT);
		}
		else
			/* menu entry */
			XDrawString(display, e->win, modegc, x, x + fontbase,
						e->name, strlen(e->name));
	}
	
	/* The ButtonPress in the menubar did an implicit grab of the pointer
	 * and buttons. We must ungrab to allow the popup to get events
	 * properly.
	 */
	XUngrabPointer(display, CurrentTime);

	do
	{
		XNextEvent(display, &event);
		switch(event.type)
		{
			case EnterNotify:
				Tk_Draw3DRectangle(event.xany.window, event.xany.window,
					&Zborder, 0, 0, w, h, MenuBW, TK_RELIEF_RAISED);
				break;
			case LeaveNotify:
				Tk_Draw3DRectangle(event.xany.window, event.xany.window,
					&Zborder, 0, 0, w, h, MenuBW, TK_RELIEF_FLAT);
				break;
		}
	}
	while(event.type != ButtonRelease);

	/* process the choosen item if any */
	ShowCursor(FALSE);
	for(e = menu->head; e; e = e->next)
		if(event.xany.window == e->win)
		{
			Argp = Arg = 0;
			(*e->func)(e->arg);
		}

	XDestroyWindow(display, popup);

	ShowCursor(TRUE);

}

int ProcessMenuBar(event)
XEvent *event;
{
	int i;
	int x, y;

/* SAM	if(event->type != ButtonPress) return 0; */

	x = event->xbutton.x; y = event->xbutton.y;

	/* If it is not on menu bar, we do not handle it! */
	if(y > H_MENUBAR) return 0;

	/* If it is in the border - pretend we handled it! */
	x -= MenuBW * 2;
	y -= MenuBW * 2;
	if(x < 0 || y < 0 || y > fontheight + highlight * 2) return 1;

	/* Check if it is on a menu item */
	for(i = 0; i < NUMMENUS; ++i)
		if(x < maxlen * (i + 1))
		{	/* got one! */
			if(event->type == ButtonPress)
				MenuPopup(&Menus[i], maxlen * i);
			return 1;
		}

	/* pretend we handled it */
	return 1;
}


static void NewEntry(menu, name, func, arg)
Menu *menu;
char *name;
Proc (*func)();
int arg;
{
	Entry *new;
	int len;

	if((new = (Entry *)malloc(sizeof(Entry))) == 0)
	{
		printf("OUT OF MEMORY!\n");
		exit(2);
	}
	
	new->name = strdup(name);
	new->func = func;
	new->arg = arg;
	new->next = 0;
	
	if(menu->head)
	{
		menu->tail->next = new;
		menu->tail = new;
	}
	else
		menu->head = menu->tail = new;
	
	++menu->nentry;
	if((len = strlen(name)) > menu->maxlen) menu->maxlen = len;
}

#endif
