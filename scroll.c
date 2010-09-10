#include "z.h"

#if XWINDOWS
#include <X11/Xlib.h>
#include "xwind.h"


/* Called to scroll a window +/-n lines.
 * Assumtions: Sendp set
 *           : Scrnmarks up to date
 *           : Point at Prow
 *           : no lines > Colmax
 */
 
/* BUGS
	Seems to be almost working!!!!!!
	Always scrolling at EOB
*/

/* Always scroll +/- 1 line for now... */
void ScrollWindow(n)
int n;
{
	extern Mark Scrnmarks[], *Sstart, *Psstart, *Send;
	extern Boolean Sendp;
	extern Byte Lfunc;

	extern Display *display;
	extern Window zwindow;
	extern GC curgc;

	register int last = Curwdo->last - 1;
	int i;

	Mark pmark;
	
	Bmrktopnt(&pmark);

	/* SAM force to 1 line scroll.... */
	n = n > 0 ? 1 : -1;

	if(n > 0)
	{	/* Scroll forward */

		/* Scroll the window */
		XCopyArea(display, zwindow, zwindow, curgc,
			0, fontheight,
			Xcol[Colmax], Xrow[last],
			0, 0);

		/* update the screen marks */
		for(i = 0; i < last + 1; ++i)
			Mrktomrk(&Scrnmarks[i], &Scrnmarks[i + 1]);
		Bpnttomrk(&Scrnmarks[i]);
		Bcsearch(NL);
		Bswappnt(&Scrnmarks[i]);
		
		/* Update Sstart */
		Mrktomrk(Sstart, &Scrnmarks[0]);
		
		/* Psstart */
		Bswappnt(Sstart);
		Bmove(-1);
		Bmrktopnt(Psstart);
		Bmove1();
		Bswappnt(Sstart);
		
		/* SAM HACK - SHOULD SET RIGHT */
		Sendp = FALSE;

		/* Update missing line */
		Prow = last;
		Pcol = 0;
#if 1
		Outline();
#else
		Tprntstr("NEW LINE");
		Tflush();
#endif
	}

	Bpnttomrk(&pmark);			
}
#endif



Outline()
{
	Mark pmark;
	int col = 0;

	Bmrktopnt(&pmark);		/* SAM We could probably go to a Scrnmark! */ 

	/* Removing the !Bisend from this loop made the most difference! */
	if(Bisend())
	{
		Tcleol();
		return;
	}
	while(!ISNL(Buff()) && (col += Twidth(Buff())) < Colmax)
	{
		if(Bisatmrk(&pmark))
		{	/* draw the cursor */
		}
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

#if 0
	/* SAM ? ? ? */
		if(Bisend())
		{
			Bshoveit();
			break;
		}
		else if(ISNL(Buff()))
			if(!Bmove1())
			{
				break;
			}
#endif
	}
	Tflush();

	Bpnttomrk(&pmark);
}
