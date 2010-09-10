/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

#ifndef BORDER3D

extern Mark Scrnmarks[], *Sstart;
extern int Sendp;

WDO *Whead = NULL, *Curwdo = NULL;

#define MINWDO		5		/* minimum window size */

static void Wfree ARGS((WDO *));
static Boolean Wdelete ARGS((WDO *));
static Boolean Wsplit ARGS((void));


/* Create a new window pointer - screen info invalid */
static WDO *Wcreate(first, last)
int first, last;
{
	WDO *new;

	if((new = (WDO *)malloc(sizeof(WDO))) != NULL)
	{
		memset((char *)new, '\0', sizeof(WDO));
		new->wbuff		= Curbuff;
		new->wpnt		= Bcremrk();
		new->wmrk		= Bcremrk();
		new->wstart		= Bcremrk();
		new->modeflags	= INVALID;
		new->first		= first;
		new->last		= last;
		Mrktomrk(new->wmrk, Curbuff->mark);
#ifdef SCROLLBARS
		CreateScrollBars(new);
#endif
	}
	return(new);
}


/* free wdo - may invalidate Curwdo and Whead */
static void Wfree(wdo)
WDO *wdo;
{
	Unmark(wdo->wpnt);
	Unmark(wdo->wmrk);
	Unmark(wdo->wstart);
#ifdef SCROLLBARS
	DeleteScrollBars(wdo);
#endif
	free((char *)wdo);
}


static Boolean Wdelete(wdo)
WDO *wdo;
{
	WDO *new;

	/* can't delete last window */
	if(!Whead->next) return FALSE;
	
	/* give more space to the smaller of the 2 windows (favour next) */
	if((wdo->next ? wdo->next->last - wdo->next->first : ROWMAX + 1) <=
	   (wdo->prev ? wdo->prev->last - wdo->prev->first : ROWMAX + 1))
	{	/* give it to the next window */
		new = wdo->next;
		new->first = wdo->first;
		new->prev = wdo->prev;
		if(wdo->prev)
			wdo->prev->next = new;
		else
			Whead = new;
	}
	else if(wdo->prev)
	{	/* give it to the previous window */
		new = wdo->prev;
		new->last = wdo->last;
		new->next = wdo->next;
		if(wdo->next) wdo->next->prev = new;
		new->modeflags = INVALID;
	}
	else
		return(FALSE);
	
	Winvalid(wdo);
	
	if(wdo == Curwdo)
	{
		Wswitchto(new);
		Reframe();	/*SAM*/
	}
	Wfree(wdo);
#ifdef SCROLLBARS
	ResizeScrollBars(new);
#endif
	return(TRUE);
}


/* 
 * Split the current window in 2, putting the same buffer in both windows.
 * Leaves the user in the new window.
 */
static Boolean Wsplit()
{
	WDO *new;
	int first, last;
	
	if(Wheight() < MINWDO) return FALSE;
	
	/* Create the new window. */
	first = Curwdo->first + (Wheight() / 2) + 1;
	last = Curwdo->last;
	if((new = Wcreate(first, last)) == 0) return FALSE;

	/* resize the old window */
	Curwdo->last = first - 1;
	new->first = Curwdo->last + 1;
	Curwdo->modeflags = INVALID;
#ifdef SCROLLBARS
	ResizeScrollBars(Curwdo);
#endif

	/* link it into chain */
	new->prev = Curwdo;
	new->next = Curwdo->next;
	if(Curwdo->next) Curwdo->next->prev = new;
	Curwdo->next = new;
	
	/* Point may be off new screen, reframe just in case... */
	Reframe();

	/* Go to new window. */
	Wswitchto(new);
	Reframe();
	Mrktomrk(Curwdo->wstart, Sstart);
	return(TRUE);
}


/* Find the window associated with buffer */
WDO *Findwdo(buff)
Buffer *buff;
{
	WDO *wdo;
	
	for(wdo = Whead; wdo; wdo = wdo->next)
		if(wdo->wbuff == buff)
			return(wdo);
	return(NULL);
}

/* Switch to another window. */
void Wswitchto(wdo)
WDO *wdo;
{
	if(wdo != Curwdo)
	{
		if(Curwdo)
		{
			Bmrktopnt(Curwdo->wpnt);
			Mrktomrk(Curwdo->wmrk, Curbuff->mark);
			/* don't update wstart unless Sstart for this window */
			if(Sstart->mbuff == Curwdo->wbuff)
				Mrktomrk(Curwdo->wstart, Sstart);
		}
		Curwdo = wdo;
		Bswitchto(wdo->wbuff);
		Bpnttomrk(wdo->wpnt);
		Mrktomrk(Curbuff->mark, wdo->wmrk);
		Mrktomrk(Sstart, wdo->wstart);
		Sendp = FALSE;
#if XWINDOWS
		XSwitchto(wdo->wbuff->bname);
#endif
	}
	Curwdo->modeflags = INVALID;
}


/* Switch to a new buffer in the current window. */
void Cswitchto(buff)
Buffer *buff;
{
	extern Mark *Sstart;

	Bswitchto(buff);
	if(Curwdo->wbuff != Curbuff)
	{	
		Curwdo->wbuff = Curbuff;
		Bmrktopnt(Curwdo->wpnt);
		Mrktomrk(Curwdo->wmrk, Curbuff->mark);
		if(Sstart->mbuff == Curbuff)
			Mrktomrk(Curwdo->wstart, Sstart);
		else
		{	/* bring to start of buffer - just in case */
			Curwdo->wstart->mbuff = Curbuff;
			Curwdo->wstart->mpage = Curbuff->firstp;
			Curwdo->wstart->moffset = 0;
		}
		Curwdo->modeflags = INVALID;
	
		Settabsize( buff->bmode );
	}

#if XWINDOWS
		XSwitchto(buff->bname);
#endif
}


/* Local routine to change the current window by 'size' lines */
static Boolean Sizewindow(size)
int size;
{
	WDO *other;
	
	if(Wheight() + size < MINWDO) return(FALSE);
	if((other = Curwdo->next) &&
		other->last - other->first - size > MINWDO)
	{
		Curwdo->last += size;
		other->first += size;
	}
	else if((other = Curwdo->prev) &&
		other->last - other->first - size > MINWDO)
	{
		Curwdo->first -= size;
		other->last   -= size;
	}
	else
		return(FALSE);

	/* invalidate the windows */		
	Winvalid(Curwdo);
	Winvalid(other);

#ifdef SCROLLBARS
	ResizeScrollBars(Curwdo);
	ResizeScrollBars(other);
#endif
	return(TRUE);
}


int noResize = 0;

/* See if window size has changed */
void Wsize()
{
	WDO *wdo;
	int orow, d, i;
	Boolean changed = TRUE;

	if(noResize) return;

	orow = Rowmax;
	Termsize();

	/* if Rowmax changed we must update window sizes */
	if(Rowmax != orow)
	{
		if(Whead->next)
			if((d = Rowmax - orow) > 0)
			{	/* make the windows bigger starting at the top */
				while(d > 0)
					for(i = 1, wdo = Whead; wdo; wdo = wdo->next)
					{
						wdo->last += i;
						if(wdo->next) wdo->next->first += i;
						if(--d > 0) ++i;
					}
			}
			else
			{	/* make the windows smaller starting at the bottom */
				d = -d;
				while(d > 0 && changed)
				{
					changed = FALSE;
					for(i = 1, wdo = Whead; wdo; wdo = wdo->next)
					{
						if(wdo->last - wdo->first - 1 > 3 && d > 0)
						{
							wdo->last -= i;
							if(wdo->next) wdo->next->first -= i;
							if(d-- > 0) ++i;
							changed = TRUE;
						}
						else
						{
							wdo->last -= i - 1;
							if(wdo->next) wdo->next->first -= i - 1;
						}
					}
				}
				if(d > 0) Z1wind();
			}
		else
		{
			Whead->last = Rowmax - 2;
		}
	}

#ifdef SCROLLBARS
	/* We always update scrollbars since width may have changed. */
	for(wdo = Whead; wdo; wdo = wdo->next)
		ResizeScrollBars(wdo);
#endif
}



/* Resize PAW by moving bottom window by 'diff' lines, if possible. */
Boolean Resize(diff)
int diff;
{
	WDO *last;
	int i;
	
	/* find the last window */
	for(last = Whead; last->next; last = last->next) ;

	if(last->last - last->first + diff < 1) return FALSE;
	if(diff > 0)
		for(i = 0; i < diff; ++i)
			Scrnmarks[i + last->last].modf = TRUE;
	last->last += diff;
	Rowmax += diff;
	last->modeflags = INVALID;
	Clrecho();
	return TRUE;
}


/*
 * Create/Reuse a buffer in another window. Window to use:
 *	1. if buffer in a window - use window
 *	2. if >1 windows - use bottom window
 *	3. if 1 window - create 2nd 10 line window and use it
 *	4. if all else fails, use current
 * Makes new window and buffer current.
 * NOTE: Blows away previous buffer.
 */
Boolean WuseOther(bname)
char *bname;
{
	WDO *wdo, *last;
	Buffer *buff;

	for(wdo = Whead, last = NULL; wdo; last = wdo, wdo = wdo->next)
		if(strcmp(wdo->wbuff->bname, bname) == 0) break;
	if(wdo)
		Wswitchto(wdo);
	else if(last != Whead)
		Wswitchto(last);
	else
	{
		Wsplit();
		if((strcmp(bname, MAKEBUFF) == 0 || strcmp(bname, REFBUFF) == 0)
			&& Wheight() > 8)
		{	/* .make/.ref buffers are smaller */
			Curwdo->first = Curwdo->last - 8;
			Curwdo->prev->last = Curwdo->first - 1;
#ifdef SCROLLBARS
			ResizeScrollBars(Curwdo->prev);
			ResizeScrollBars(Curwdo);
#endif
		}
	}
	Winvalid(Curwdo);
	if((buff = Cmakebuff(bname, NULL)) == NULL) return(FALSE);
	buff->bmode |= SYSBUFF;
	Cswitchto(buff);
	Bempty();
	return(TRUE);
}


/*
 * Invalidate an entire window. i.e. next Refresh will do a complete update.
 * Note that the line BEFORE the window must be invalidated to make sure that
 * the window is updated correctly.
 */
void Winvalid(wdo)
WDO *wdo;
{
	int i;

	if(wdo->first > Tstart) Scrnmarks[wdo->first - 1].modf = TRUE;
	for(i = wdo->first; i <= wdo->last; ++i)
		Scrnmarks[i].modf = TRUE;
	wdo->modeflags = INVALID;
}


/****************************************************************************
 *																			*
 *					Zedit interface to window routines						*
 *																			*
 ****************************************************************************/

/* Split the current window and enter new (bottom) window */
Proc Z2wind()
{
	if(!Wsplit()) Tbell();
}


/* Tear down all but one (current) window */
Proc Z1wind()
{
	WDO *wdo;
	int i;

	while(Whead)
	{
		wdo = Whead;
		Whead = Whead->next;
		if(wdo != Curwdo) Wfree(wdo);
	}

	Curwdo->first = Tstart;
	Curwdo->last = Tmaxrow() - 2;
	Curwdo->modeflags = INVALID;
	Curwdo->prev = Curwdo->next = NULL;
	Whead = Curwdo;

	for(i = 0; i < Curwdo->last; ++i)			Scrnmarks[i].modf = TRUE;

	Tclrwind();

#ifdef SCROLLBARS
	ResizeScrollBars(Curwdo);
#endif
}


/* Delete current window if more than one */
Proc Zdelwind()
{
	if(!Wdelete(Curwdo)) Tbell();
}


/* Make previous window current */
Proc Zprevwind()
{
	WDO *wdo;
	
	if(Curwdo->prev)
		Wswitchto(Curwdo->prev);
	else
	{
		for(wdo = Whead; wdo->next; wdo = wdo->next) ;
		if(wdo != Curwdo)
			Wswitchto(wdo);
		else
			Tbell();
	}
}


/* Make next window current */
Proc Znextwind()
{
	if(Curwdo->next)
		Wswitchto(Curwdo->next);
	else if(Curwdo != Whead)
		Wswitchto(Whead);
	else
		Tbell();
}


/* Make current window bigger */
Proc Zgrowwind()
{
	Sizewindow(Arg);
	Arg = 0;
}


/* Make current window smaller */
Proc Zshrinkwind()
{
	Sizewindow(-Arg);
	Arg = 0;
}


/* Make current window an absolute size */
Proc Zsizewind()
{
	if(!Sizewindow(Arg - Wheight() + 1)) Tbell();
	Arg = 0;
}


/*
 * Find the "other" window:
 *	- if there is an Arg - go to Arg window (start at 1)
 *	- use bottom window unless current
 *	- use top window
 *	- use current if only one
 * Makes new window current.
 * Never fails.
 */
static WDO *Otherwind()
{
	WDO *wdo;
	
	if(Argp)
		for(wdo = Whead; --Arg > 0; wdo = wdo->next ? wdo->next : Whead) ;
	else
	{
		for(wdo = Whead; wdo->next; wdo = wdo->next) ;
		if(wdo == Curwdo) wdo = Whead;
	}
	Wswitchto(wdo);
	return wdo;
}


Proc Znxtothrwind()
{
	WDO *save = Curwdo;
	Otherwind();
	Znextpage();
	Wswitchto(save);
}


Proc Zprevothrwind()
{
	WDO *save = Curwdo;
	Otherwind();
	Zprevpage();
	Wswitchto(save);
}


/*
 * If buffer is in a current window, switchto that window, else put the buffer
 * in the current or other window.
 */
void Bgoto(buff)
Buffer *buff;
{
	WDO *wdo;
	
	if((wdo = Findwdo(buff)))
	{
		Wswitchto(wdo);
		return;
	}
		
	if(Vars[VUSEOTHER].val)
	{
		if(Curwdo->next)
			Wswitchto(Curwdo->next);
		else if(Curwdo->prev)
			Wswitchto(Curwdo->prev);
	}

	Cswitchto(buff);
}


/* Load into correct window based on UseOtherWdo.
 * Always munch Main
 * Never  munch .make
 */
void Loadwdo(bname)
char *bname;
{
	if(Vars[VUSEOTHER].val)
	{
		if(strcmp(Curwdo->wbuff->bname, bname) == 0)
			return;
		if(strcmp(Curwdo->wbuff->bname, MAINBUFF) == 0)
			return;
		if(Whead->next)
		{
			Znextwind();
			/* SAM never munch on .make buffer */
			if(strcmp(Curwdo->wbuff->bname, MAKEBUFF) == 0)
				Znextwind();
		}
		else
			Z2wind();
	}
}


/* These routines are ONLY callable at startup. */

static WDO *Wstart = NULL;	/* Set in Wload */

void Winit()
{
	if(Wstart == NULL)
	{	/* Create first window over entire screen. */
		Whead = Wcreate(Tstart, Rowmax - 2);
		Wswitchto(Whead);
	}
	else
	{	/* We created the window[s] with Wload[s]. */
		for(; Whead->prev; Whead = Whead->prev) ;
		Wswitchto(Wstart);
		Wsize();
	}
}


void Wload(bname, first, last, sloc, iscurrent)
char *bname;
int first, last, iscurrent;
unsigned long sloc;
{
	WDO *new;
	Buffer *buff;

#if PIPESH
	if(strcmp(bname, SHELLBUFF) == 0)
	{	/* invoke the shell */
		if((buff = Cmakebuff(SHELLBUFF, NULL)) == NULL) NoMem();
		buff->bmode |= SYSBUFF;
		Doshell();
	}
#endif
	if((buff = Cfindbuff(bname)) == NULL) buff = Cfindbuff(MAINBUFF);
	Bswitchto(buff);
	if((new = Wcreate(first, last)) == NULL) NoMem();
	Mrktomrk(buff->mark, new->wmrk);
	Boffset(sloc);
	Bmrktopnt(new->wstart);
	Bpnttomrk(new->wpnt);	/* return it */
	new->first = first;
	new->last  = last;
	if(Whead)
	{
		Whead->next = new;
		new->prev = Whead;
	}
	Whead = new;
	if(iscurrent) Wstart = new;
	Rowmax = new->last + 2;
}

#endif
