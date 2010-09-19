/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#ifdef BORDER3D

#include "../z.h"

extern Mark Scrnmarks[], *Sstart;
extern int Sendp;

WDO *Curwdo = NULL;

#define MINWDO		5		/* minimum window size */

/* Create a new window pointer - screen info invalid */
static WDO *Wcreate(first, last)
int first, last;
{
	if(Curwdo) return;

	if((Curwdo = (WDO *)malloc(sizeof(WDO))) != NULL)
	{
		memset((char *)Curwdo, '\0', sizeof(WDO));
		Curwdo->wbuff		= Curbuff;
		Curwdo->wpnt		= Bcremrk();
		Curwdo->wmrk		= Bcremrk();
		Curwdo->wstart		= Bcremrk();
		Curwdo->modeflags	= INVALID;
		Curwdo->first		= first;
		Curwdo->last		= last;
		Mrktomrk(Curwdo->wmrk, Curbuff->mark);
	}
	return Curwdo;
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
Tbell();
return FALSE;
#if 0
/* SAM Rethink this...*/
	
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

	return(TRUE);
#endif
}


int noResize = 0;

/* See if window size has changed */
void Wsize()
{
	int orow;

	if(noResize) return;

	orow = Rowmax;
	Termsize();

	/* if Rowmax changed we must update window sizes */
	if(Rowmax != orow)
		Curwdo->last = Rowmax;
}



/* Resize PAW by moving bottom window by 'diff' lines, if possible. */
Boolean Resize(diff)
int diff;
{
	WDO *last;
	int i;
	
	if(Curwdo->last - Curwdo->first + diff < 5) return FALSE;
	if(diff > 0)
		for(i = 0; i < diff; ++i)
			Scrnmarks[i + Curwdo->last].modf = TRUE;
	Curwdo->last += diff;
	Rowmax += diff;
	Curwdo->modeflags = INVALID;
	Clrecho();
	return TRUE;
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

void Z1wind() {}
void Z2wind() { Tbell(); }
void Zdelwind() { Tbell(); }
void Zprevwind() { Tbell(); }
void Znextwind() { Tbell(); }

void Zgrowwind()
{
	Sizewindow(Arg);
	Arg = 0;
}

void Zshrinkwind()
{
	Sizewindow(-Arg);
	Arg = 0;
}

void Zsizewind()
{
	if(!Sizewindow(Arg - Wheight() + 1)) Tbell();
	Arg = 0;
}


void Znxtothrwind() { Tbell(); }
void Zprevothrwind() { Tbell(); }


/* If buffer is in a current window, switchto that window, else put the buffer
 * in the current or other window.
 */
void Bgoto(buff)
Buffer *buff;
{
	Cswitchto(buff);
}


void Loadwdo(bname) char *bname; {}


/* These routines are ONLY callable at startup. */

static WDO *Wstart = NULL;	/* Set in Wload */

void Winit()
{
	if(Wstart == NULL)
		/* Create first window over entire screen. */
		Wcreate(Tstart, Rowmax - 2);
	else
		/* We created the window[s] with Wload[s]. */
		Wsize();
}


void Wload(bname, first, last, sloc, iscurrent)
char *bname;
int first, last, iscurrent;
unsigned long sloc;
{	/* ignore all but current window */
	if(iscurrent)
	{
		Buffer *buff;
		
		if((buff = Cfindbuff(bname)) == NULL) buff = Cfindbuff(MAINBUFF);
		Bswitchto(buff);
		if((Wstart = Wcreate(first, last)) == NULL) NoMem();
		Mrktomrk(buff->mark, Wstart->wmrk);
		Boffset(sloc);
		Bmrktopnt(Wstart->wstart);
		Bpnttomrk(Wstart->wpnt);	/* return it */
		Wstart->first = first;
		Wstart->last  = last;
		Rowmax = Wstart->last + 2;
	}
}

#endif
