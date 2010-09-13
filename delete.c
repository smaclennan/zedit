/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"


Buffer *Killbuff;			/* the kill buffer */


Proc Zmakedel() {}


Proc Zdelchar()
{
	Bdelete( Arg );
	Arg = 0;
}


Proc Zrdelchar()
{
	Bmove(-Arg);
	Bdelete(Arg);
	Arg = 0;
}


Proc Zdeleol()
{
	Mark *tmark = Bcremrk();

	if( !Bisend() && Buff() == NL )
		Bmove1();
	else if(Vars[VKILLLINE].val)
	{
		Boolean atstart;

		Tobegline();
		atstart = Bisatmrk(tmark);
		Toendline();
		if(atstart) Bmove1(); /* delete the NL */
	}
	else
		Toendline();
	Killtomrk( tmark );
	Unmark( tmark );
}


Proc Zdelline()
{
	Mark *tmark;

	Tobegline();
	tmark = Bcremrk();
	Bcsearch( NL );
	Killtomrk( tmark );
	Unmark( tmark );
}

	
Proc Zdelrgn()
/* Delete from the point to the mark */
{
	Killtomrk( Curbuff->mark );
}


/* Copy from point to the mark into delbuff */
Proc Zcopyrgn()
{
	Copytomrk(Curbuff->mark);
}


/* UNDO */
/* Insert the delete buffer at the point */
Proc Zyank()
{
	extern Boolean First;
	extern Mark *Send;
	Buffer *tbuff;
	int yanked;
	Mark *tmark, save;	/* save must NOT be a pointer */

	if (InPaw && First) {
		Bdelete( Curplen );
		First = FALSE;
	}

	Mrktomrk(&save, Send);
	tbuff = Curbuff;
	Bmrktopnt(Curbuff->mark);
	Bswitchto(Killbuff);
	Btoend();
	tmark = Bcremrk();
	Btostart();
	yanked = Bcopyrgn( tmark, tbuff );
	Unmark( tmark );
	Bswitchto( tbuff );
	undo_add(yanked);
	if( Bisaftermrk(&save) )
		Reframe();
}


Proc Zdelword()
{
	Mark *tmark;

	tmark = Bcremrk();
	Moveto(Isword, FORWARD);
	Movepast(Isword, FORWARD);
	Killtomrk( tmark );
	Unmark( tmark );
}


Proc Zrdelword()
{
	Mark *tmark;

	tmark = Bcremrk();
	Zbword();
	Killtomrk( tmark );
	Unmark( tmark );
}


Proc Zgetbword()
{
	extern int Arg;
	extern unsigned Cmd;
	extern Buffer *Buff_save, *Paw;
	char word[ STRMAX ], *ptr;
	Mark *tmark, *start;
	
	if( InPaw )
	{
		Bswitchto( Buff_save );
		Getbword( word, STRMAX, Istoken );
		Bswitchto( Paw );
		for( ptr = word; *ptr; ++ptr )
		{
			Cmd = *ptr;
			Pinsert();
		}
	}
	else
	{
		tmark = Bcremrk();					/* save current Point */
		Moveto( Istoken, FORWARD );			/* find start of word */
		Movepast( Istoken, BACKWARD );
		start = Bcremrk();
		Movepast( Istoken, FORWARD );		/* move Point to end of word */
		Copytomrk( start );					/* copy to Kill buffer */
		Bpnttomrk( tmark );					/* move Point back */
		Unmark( tmark );
		Unmark( start );
	}
	Arg = 0;
}


Proc Zdelblanks()
{
	Mark *tmark, *pmark;

	pmark = Bcremrk();
	if( Bcrsearch(NL) )
	{
		Bmove1();
		tmark = Bcremrk();
		Movepast( Isspace, BACKWARD );
		if( !Bisstart() ) Bcsearch( NL );
		if( Bisbeforemrk(tmark) )
			Bdeltomrk( tmark );
		Unmark( tmark );
	}
	if( Bcsearch(NL) )
	{
		tmark = Bcremrk();
		Movepast( Isspace, FORWARD );
		if( Bcrsearch(NL) ) Bmove1();
		if( Bisaftermrk(tmark) )
			Bdeltomrk( tmark );
		Unmark( tmark );
	}
	Bpnttomrk( pmark );
	Unmark( pmark );
}


Proc Zjoin()
{
	Toendline();
	Bdelete( 1 );
	Zdelwhite();
	Binsert( ' ' );
}


Proc Zempty()
{
	if(Ask("Empty buffer? ") != YES) return;
	Bempty();
	Curbuff->bmodf = MODIFIED;
}


void Copytomrk(Mark *tmark)
{
	Buffer *save = Curbuff;
	Bswitchto(Killbuff);
	if (Delcmd())
		Btoend();
	else
		Bempty();
	Bswitchto(save);
	Bcopyrgn(tmark, Killbuff);
}
