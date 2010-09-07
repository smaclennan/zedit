/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

extern Mark *Sstart;

static void Scroll ARGS((Boolean));

Proc Zbegline()
{
	Bmove(-1);
	Tobegline();
}


Proc Zendline()
{
	Bmove1();
	Toendline();
}


/* Support routine to calc column to force to for line up and down */
int Forcecol()
{
	static int fcol;

	return zCursor() ? fcol : (fcol = Bgetcol(TRUE, 0));
}


static void ScrollLine(forward)
Boolean forward;
{
	extern Mark *Sstart, *Psstart;
	extern Boolean Sendp;
	Mark save;

	if(Vars[VSINGLE].val)
	{
		Bmrktopnt(&save);
		Bpnttomrk(Sstart);
		forward ? Bcsearch(NL) : Bcrsearch(NL);
		Tobegline();
		Bmrktopnt(Sstart);
		Bmove(-1);
		Bmrktopnt(Psstart);
		Bpnttomrk(&save);
		Sendp = FALSE;
	}
}
	
Proc Zprevline()
{
	extern Mark *Sstart;
	int col = Forcecol();
	while(Arg-- > 0) Bcrsearch(NL);
	
	if(Bisbeforemrk(Sstart))
		ScrollLine(FALSE);
			
	Bmakecol(col, FALSE);
}


Proc Znextline()
{
	extern Mark *Send;
	extern Boolean Sendp;
	int col = Forcecol();

	while(Arg-- > 0) Bcsearch(NL);

	if(Sendp && !Bisbeforemrk(Send))
		ScrollLine(TRUE);

	Bmakecol(col, FALSE);
}


Proc Zprevchar()
{
	Bmove(-Arg);
	Arg = 0;
}


Proc Znextchar()
{
	Bmove(Arg);
	Arg = 0;
}


Proc Zprevpage()
{
	int i, n, col = Forcecol();
	
	Bpnttomrk(Sstart);
	for(n = i = Wheight() - Prefline() - 2; i > 0 && Bcrsearch(NL); --i)
		i -= Bgetcol(TRUE, 0) / Tmaxcol();
	Bmakecol(col, FALSE);
	Reframe();
}


Proc Znextpage()
{
	int i, col = Forcecol();
	
	Bpnttomrk(Sstart);
	for(i = Wheight() + Prefline() - 2; i > 0 && Bcsearch(NL); --i)
	{
		Bmove(-1);
		i -= Bgetcol(TRUE, 0) / Tmaxcol();
		Bmove1();
	}
	Bmakecol(col, FALSE);
	Reframe();
}


#define ISWORD	Istoken

Proc Zbword()
{
	Moveto  (ISWORD, BACKWARD);
	Movepast(ISWORD, BACKWARD);
}


Proc Zfword()
{
	Movepast(ISWORD, FORWARD);
	Moveto  (ISWORD, FORWARD);
}


Proc Ztostart()
{
	Btostart();
}


Proc Ztoend()
{
	Btoend();
}


Proc Zswapmrk()
{
	Mark tmark;

	Arg = 0;
	Mrktomrk(&tmark, Curbuff->mark);
	Zsetmrk();
	Bpnttomrk(&tmark);
}


Proc Zopenline()
{
	Binsert(NL);
	Bmove(-1);
}


Proc Zlgoto()
{
	long line, cnt;
	Page *tpage;

	if(Argp)
		line = Arg;
	else if((line = Getnum("Line: ")) == -1)
		return;

	/* find the correct page */
	for( cnt = 0, tpage = Curbuff->firstp; tpage->nextp; tpage = tpage->nextp )
	{
		if( tpage->lines == EOF )
		{
			Makecur( tpage );
			tpage->lines = Cntlines( Curplen );
		}
		if( (cnt += tpage->lines) >= line )
		{
			cnt -= tpage->lines;
			break;
		}
	}
	Makecur( tpage );
	Makeoffset( 0 );
	
	/* go to the correct offset */
	for( cnt = line - cnt - 1; cnt > 0; --cnt )
		Bcsearch( NL );
}

Proc Zcgoto()
{
	int col;
	
	if( (col = (int)Getnum("Column: ")) == -1 ) return;
	Bmakecol( --col, TRUE );
}


long Getnum( prompt )
char *prompt;
{
	extern long strtol();
	
	char str[ 10 ];
	long num = -1;

	/* get the line number */	
	*str = '\0';
	if( Argp )
	{
		num = Arg;
		Arg = 0;
	}
	else if( Getarg(prompt, str, 9) == 0 )
		num = strtol( str, NULL, 0 );
	return( num );
}


Mark *Bookmrks[BOOKMARKS];			/* stack of book marks */
char *Bookname[BOOKMARKS];			/* stack of book names */
int  Bookmark = -1;					/* current book mark */
int  Lastbook = -1;					/* last bookmark */
	

Proc Zsetbookmrk()
{
	if(Argp)
	{
		Arg = 0;
		*PawStr = '\0';
		if(Getarg("Bookmark name: ", PawStr, STRMAX)) return;
	}
	else
		strcpy(PawStr, "Unamed");

	Bookmark = ++Bookmark % BOOKMARKS;
	if(Bookname[Bookmark]) free(Bookname[Bookmark]);
	Bookname[Bookmark] = strdup(PawStr);

	if(Bookmark > Lastbook)
	{
		Lastbook = Bookmark;
		Bookmrks[Bookmark] = Bcremrk();
	}
	else
		Bmrktopnt(Bookmrks[Bookmark]);

	if(Argp)
		sprintf(PawStr, "Book Mark %s(%d) Set",
			Bookname[Bookmark], Bookmark + 1);
	else
		sprintf(PawStr, "Book Mark %d Set", Bookmark + 1);
	Echo(PawStr);
}


Proc Znxtbookmrk()
{
	extern char Lbufname[];
	
	if(Bookmark < 0)
	{
		Echo("No bookmarks set.");
		return;
	}
	
	if(Argp)
	{
		int b;
		
		Arg = 0;
		if((b = Getplete("Bookmark name: ", NULL, Bookname, sizeof(char *),
			Lastbook + 1)) == -1) return;
printf("Getplete returned %d -> ", b);
printf("%s\n", Bookname[b]);
	}

	if(Bookmrks[Bookmark]->mbuff != Curbuff)
	{
		strcpy(Lbufname, Curbuff->bname);
		Bgoto(Bookmrks[Bookmark]->mbuff);
	}
	Bpnttomrk( Bookmrks[Bookmark] );
	Curwdo->modeflags = INVALID;
	sprintf( PawStr, "Book Mark %d", Bookmark + 1 );
	Echo( PawStr );
	if( --Bookmark < 0 ) Bookmark = Lastbook;
}


Proc Zviewline()
{
	extern Boolean Sendp;
	extern Mark *Psstart;
	Mark pmark;
	
	Bmrktopnt( &pmark );
	Tobegline();
	Bmrktopnt( Sstart );
	Bmove( -1 );
	Bmrktopnt( Psstart );
	Sendp = FALSE;
	Bpnttomrk( &pmark );
}


Proc Zredisplay()
{
#ifndef BORDER3D
	WDO *wdo;
#endif

	Wsize();
#ifdef BORDER3D
	Curwdo->modeflags = INVALID;
#else
	for(wdo = Whead; wdo; wdo = wdo->next) wdo->modeflags = INVALID;
#endif
	Redisplay();
#if COMMENTBOLD
	Recomment();
#endif
#ifdef BORDER3D
	/* Draw the boarder around the window */
	DrawBorders();
#endif
}


Proc Zbegwind()
{
	Bpnttomrk(Sstart);
}


Proc Zendwind()
{
	int i;

	Bpnttomrk(Sstart);
	for(i = Wheight() - 1; i && Bcsearch(NL); --i) ;
}


Proc Zscrollup()
{
	Scroll(FALSE);
}


Proc Zscrolldown()
{
	Scroll(TRUE);
}


static void Scroll(forward)
Boolean forward;
{
	extern Mark *Psstart;
	extern int Sendp;
	Mark *pmark = Bcremrk();

	Bpnttomrk(Sstart);
	if(forward)
		while(Arg-- > 0 && Bcsearch(NL)) ;
	else
		while(Arg-- > 0 && Bcrsearch(NL)) ;
	Tobegline();
	Bmrktopnt( Sstart );
	Bmove( -1 );
	Bmrktopnt( Psstart );
	Sendp = FALSE;

	if(Mrkaftermrk(Sstart, pmark))
		Bmove1();
	else
		Bpnttomrk( pmark );

	Unmark( pmark );
}
