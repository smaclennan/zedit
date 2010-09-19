#include "z.h"

#if XWINDOWS

/* SAM Currently only X, but should try in rest of product, especially for
 * single line scrolling.
 */

Boolean ScrolledWindow = FALSE;

/* Called to scroll a window +/-n lines.
 * Assumption:	nothing in buffer has changed
 *				not in paw
 */
void ScrollWindow(n)
int n;
{
	extern Mark *Sstart, *Psstart, *Send, Scrnmarks[];
	extern Boolean Sendp;
	int col, bcol, tsave;
	int pntrow, pntcol;
	WDO *wdo;
	Mark pmark;
	register int trow;
	Boolean needpnt = TRUE;
	int i;

	ScrolledWindow = TRUE;
	if(n == 0) return;

	/* Find new point. */
	Bmakecol(Forcecol(), FALSE);
#if 0
	if((i = n) > 0)
		while(i-- > 0) Bcsearch(NL);
	else
		while(i++ < 0) Bcrsearch(NL);
#endif
	Bmrktopnt(&pmark);

	/* Find new start. */
	Bpnttomrk(Sstart);
	if(n > 0)
		for(i = n; i > 0 && !Bisend(); --i) Bcsearch(NL);
	else
		for(i = n; i < 0 && !Bisstart(); ++i) Bcrsearch(NL);
	if(i == n)
	{	/* Bisend or Bisstart */
		Bpnttomrk(&pmark);
		return;
	}

	Bmrktopnt(Sstart);
	if(Bisstart())
		Bmrktopnt(Psstart);
	else
	{
		Bmove(-1);
		Bmrktopnt(Psstart);
		Bmove1();
	}
	
#if COMMENTBOLD
	ResetComments();
#endif

	for(trow = Curwdo->first; trow < Curwdo->last; ++trow)
	{
		Bmrktopnt(&Scrnmarks[trow]);
		Scrnmarks[trow].modf = FALSE;

		Tsetpoint(trow, col = Tstart);
			
		/* Removing the !Bisend from this loop made the most difference! */
		while(!ISNL(Buff()) && (col += Twidth(Buff())) < Colmax)
		{	
			if(Bisatmrk(Curbuff->mark))
				SetMark(TRUE);
			else
			{	
#if COMMENTBOLD
				CheckComment();
#endif
				Tprntchar(Buff());
			}
			if(!Bmove1())
				break;	/* EOB */
		}

		Tcleol();

		if(Bisatmrk(Curbuff->mark) && (ISNL(Buff()) || Bisstart() || Bisend()))
			SetMark(FALSE);
		if(col >= Tmaxcol())
		{
#ifdef PC
			Pcol = Tmaxcol() - 1;
			Tputchar( 0x10 );
#else
#if XWINDOWS
			Pcol = Tmaxcol() - 1;
#else
			for( ; col < Tmaxcol() - 1; ++col) Tprntchar(' ');
#endif
			Tstyle( T_BOLD );
			Tprntchar( '>' );
			Tstyle( T_NORMAL );
#endif
		}
		else
		{
#ifdef PC
			if(Vars[VBORDER].val)
			{
				Pcol = Tmaxcol() - 1;
				Tputchar( 0xb3 );
				Pcol = col;
			}
#endif	

#if 1
			if(Bisend())
			{
				Bshoveit();
				++trow;
				break;
			}
			else if(ISNL(Buff()))
				if(!Bmove1())
				{
					++trow;
					break;
				}
#else
			if(!Bisend() && ISNL(Buff()))
				Bmove1();
#endif
		}

		if(Bisaftermrk(&pmark) && needpnt)
		{
			pntrow = trow;
			needpnt = FALSE;
		}
	}

	Bmrktopnt( &Scrnmarks[trow] );
	Bmrktopnt(Send);
	Sendp = TRUE;
	
	/* If we broke out early... */
	if(trow < Curwdo->last)
	{
		Pcol = 0;
		for(Prow = trow; Prow < Curwdo->last; ++Prow)
			Tcleol();
	}

#if COMMENTBOLD
	Tstyle(T_NORMAL);
#endif
	
	Bpnttomrk(&pmark);

#if 1
	bcol = Bgetcol( TRUE, 0 );
	/* position the cursor */
	col = bcol % (Tmaxcol() - 1 - Tstart);
	/* special case for NL or Bisend at column 80 */
	if( col == 0 && bcol && (ISNL(Buff()) || Bisend()) )
		col = Tmaxcol() - 1 - Tstart;
	else if( !Bisend() && (col + Width(Buff(), col, FALSE) >= Tmaxcol()) )
		col = 0;
	Tgoto( pntrow, col + Tstart );
#endif

#if !XWINDOWS && !PC
	/*
	 * If we display the cursor on the mark, they both disappear.
	 * This code checks for this case: if true it removes the mark
	 * and invalidates its position so it will be updated when the
	 * cursor moves on...
	 */
	if(Bisatmrk(Curbuff->mark))
	{
		Tstyle(T_NORMAL);
		Tprntchar((Bisend() || ISNL(Buff())) ? ' ' : Buff());
		Tgoto( pntrow, col + Tstart );
		was->moffset = PSIZE + 1;		/* Invalidate it */
	}
#endif

#ifdef PC
	Boldline();
#endif
#if MSDOS
	Mouse_cursor( TRUE );
#endif
	Tflush();
}
#endif
