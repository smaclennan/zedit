/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"
#include "keys.h"
#include "help.h"

extern int Curmodf;
int Arg;
Boolean Argp;

/* Word COMMANDS */

Proc Zcapword()
{
	if(Findstart())
	{	
		Buff() = Toupper( Buff() );
		Curbuff->bmodf = Curmodf = MODIFIED;
		for( Bmove1(); !Bisend() && Istoken(); Bmove1() )
			Buff() = Tolower( Buff() );
		Vsetmod( FALSE );
	}
}


Proc Zlowword()
{
	if(Findstart())
	{
		for( ; !Bisend() && Istoken(); Bmove1() )
		{
			Curbuff->bmodf = Curmodf = MODIFIED;
			Buff() = Tolower( Buff() );
		} 
		Vsetmod( FALSE );
	}
}


Proc Zupword()
{
	if(Findstart())
	{
		for( ; !Bisend() && Istoken(); Bmove1() )
		{
			Curbuff->bmodf = Curmodf = MODIFIED;
			Buff() = Toupper( Buff() );
		} 
		Vsetmod( FALSE );
	}
}


/* Find the start of a word. In normal/text, we always move to the start of
 * a word if we are in the middle. In program mode, if we are in a word, we
 * do not move. 
 */
Boolean Findstart()
{
	if((Curbuff->bmode & PROGMODE))
		Moveto(Istoken, FORWARD);
	else
	{
		while( !Bisstart() && Istoken() ) Bmove( -1 );
		while( !Bisend() && !isalpha(Buff()) ) Bmove1();
	}
	return(!Bisend());
}


Proc Zswapword()
{
	Mark *tmark, *tmp;

	Moveto( Istoken, FORWARD );
	if( Bisend() ) return;
	tmark = Bcremrk();
	Movepast( Istoken, FORWARD );
	tmp = Bcremrk();
	Bpnttomrk( tmark );
	Moveto( Istoken, BACKWARD );
	Blockmove( tmark, tmp );
	Movepast( Istoken, BACKWARD );
	Blockmove( tmark, tmp );
	Bpnttomrk( tmp );
	Unmark( tmark );
	Unmark( tmp );
}


Proc Zcenter()
{
	int tmp;

	Tobegline();
	Zdelwhite();
	Toendline();
	tmp = Bgetcol( TRUE, 0 );
	if( tmp <= Fillwidth() )
	{
		Tobegline();
		Tindent( (Fillwidth() - tmp) / 2 );
		Toendline();
	}
}

		
/* C MODE COMMANDS */


/* This code must handle any char so that expansion will work */
Proc Zcinsert()
{
	extern unsigned Cmd;
	int cnt, crfound = FALSE;
	Mark tmark;

	if( STRIP(Cmd) == '}' )
	{	/* line it up with last unmatched '{' */
		Arg = 0;
		Bmrktopnt( &tmark );	/* save current position */
		for( cnt = 0, Bmove(-1); !Bisstart(); Bmove(-1) )
			if( STRIP(Buff()) == '{' )
				if( cnt )
					--cnt;
				else
				{	/* matched */
					Tobegline();
					if(Vars[VMATCH].val & 0x100)
					{	/* show the rest of the line in the PAW */
						Mark t;
						int i, col;
						
						Bmrktopnt(&t);
						for(i = col = 0;
							!Bisend() && !ISNL(Buff()) && col < Colmax;
							++i, Bmove1())
						{
							col += Width(Buff(), col, FALSE);
							PawStr[i] = Buff();
						}
						PawStr[i] = '\0';
						Echo(PawStr);
						Bpnttomrk(&t);
					}
					Movepast( Iswhite, FORWARD );
					cnt = Bgetcol( TRUE, 0 );
					Bpnttomrk( &tmark );
					if( crfound )
					{
						Zdelwhite();
						Tindent( cnt );
					}
					Binsert( Cmd );
					return;
				}
			else if( STRIP(Buff()) == '}' )
				++cnt;
			else if( ISNL(Buff()) )
				crfound = TRUE;
		Bpnttomrk( &tmark );	/* no match - go back */
		Tbell();				/* and warn user */
	}
	else if(STRIP(Cmd) == '#')
	{
		Mark *tmark = Bcremrk();

		do
			Bmove(-1);
		while(!Bisstart() && Iswhite());

		if(ISNL(Buff()) && !Bisstart())
		{
			Bmove1();		/* skip NL */
			Bdeltomrk(tmark);		/* SAM Killtomrk(tmark); */
		}
		else
			Bpnttomrk(tmark);
		Unmark(tmark);
	}
	else if(STRIP(Cmd) == ':')
	{	/* for c++ look for public/protected */
		char word[16];
		
		Getbword(word, 15, Isword);
		if(strcmp(word, "public")    == 0 ||
		   strcmp(word, "private")   == 0 ||
		   strcmp(word, "protected") == 0)
		{
			Mark *tmark = Bcremrk();
			Tobegline();
			while(Isspace()) Bdelete(1);
			Bpnttomrk(tmark);
			Unmark(tmark);
		}
	}

	Binsert( Cmd );

#if COMMENTBOLD
	if(STRIP(Cmd) == '/')
	{	/* SAM What about overwrite mode? */
		Boolean comment;

		Bmove(-2);
		comment = Buff() == '*';
		Bmove(2);
		if(comment) AddComment();
	}
	else if(STRIP(Cmd) == '#')
	{
		if(Bmove(-2) == 0)
		{	/* # is first character in buffer */
			Bmove1();
			AddCPP();
		}
		else
		{
			Boolean cpp = ISNL(Buff());
			Bmove(2);
			if(cpp) AddCPP();
		}
	}
#endif

}


/* 'C' indent. Called for NL. */
Proc Zcindent()
{
	int width;
	Mark tmark;

	if( (Curbuff->bmode & OVERWRITE) )
		Bcsearch( NL );
	else
	{
		int sawstart = 0;

		Bmrktopnt(&tmark);
		do
		{
			if(STRIP(Buff()) == '#') Bmove(-1);
			Tobegline();
		} while(STRIP(Buff()) == '#' && !Bisstart());
		Movepast(Iswhite, FORWARD);
		if(Bisaftermrk(&tmark)) Bpnttomrk(&tmark);
		for(width = Bgetcol(TRUE, 0); Bisbeforemrk(&tmark); Bmove1())
			if(STRIP(Buff()) == '{')
			{
				width += Tabsize;
				++sawstart;
			}
			else if(STRIP(Buff()) == '}' && sawstart)
			{
				width -= Tabsize;
				--sawstart;
			}
		Bpnttomrk(&tmark);
		Binsert(NL);
		Tindent(width);
	}
}


/* This is called from Zfillpara if we are in C mode.
 * It is used to reformat comments.
 *
 * It is fairly deadly, so I break all rules and ask "Are you sure?"
 *
 * We assume comments are as follows:
 *
 *		[<ws>] <start_comment> [*]
 *		[<ws>] [*] [<text>]
 *			...
 *		[<ws>] [*] <end_comment> [<text>]
 */
Proc Zfillcomment()
{
	Mark *start, *end, *tmp, tmark;
	int col;
	
	if(Ask("Are you sure? ") != YES) return;

	if(Curbuff->bmodf) Zfilesave();

	Bmrktopnt(&tmark);

	/* find start of comment */
	if(!Bsearch("/*", FALSE))
	{
		Bpnttomrk(&tmark);
		Error("Unable to find start of comment.");
		return;
	}
	col = Bgetcol(TRUE, 0) + 1;
	Bcsearch(NL);
	start = Bcremrk();

	/* find end of comment */
	if(!Bsearch("*/", TRUE))
	{
		Unmark(start);
		Bpnttomrk(&tmark);
		Error("Unable to find end of comment.");
		return;
	}
	Zdelwhite();
	Tindent(col);
	Bcrsearch(NL);
	end = Bcremrk();

	/* Remove leading ws and '*'s */
	for(Bpnttomrk(start); Bisbeforemrk(end); Bcsearch(NL))
	{
		Zdelwhite();
		if(Buff() == '*')
		{
			Bdelete(1);
			Zdelwhite();
		}
	}
	
	/* do it! */
	Bpnttomrk(start);
 	do
	{
		/* mark the end of the paragraph and move the point to the start */
		Zfpara();
		Movepast(Isspace, BACKWARD);
		tmp = Bcremrk();
		if(Mrkaftermrk(tmp, end)) Mrktomrk(tmp, end);
		Zbpara();
		if(Bisbeforemrk(start)) Bpnttomrk(start);
		Tindent(col); Binstr("* ");

		/* main loop */
		while( Bisbeforemrk(tmp) )
		{
			Moveto( Isspace, FORWARD );
			if( Bgetcol(TRUE, 0) > Fillwidth())
			{
				Moveto( Isspace, BACKWARD );
				Zdelwhite();
				Binsert(NL);
				Tindent(col); Binstr("* ");
			}
			Movepast( Iswhite, FORWARD );
			if( Buff() == NL && Bisbeforemrk(tmp) )
			{	/* convert NL to space */
				Bdelete(1);
				Zdelwhite();
				Binsert(' ');
			}
		}
		Unmark( tmp );
		
		Movepast(Isspace, FORWARD);		/* setup for next iteration */
	}
	while(Bisbeforemrk(end));

	/* Fill in empty lines. */
	for(Bpnttomrk(start); Bisbeforemrk(end); Bcsearch(NL))
		if(Buff() == NL)
		{
			Tindent(col);
			Binsert('*');
		}
	if(Buff() == NL)
	{
		Tindent(col);
		Binsert('*');
	}
}

/* FILL MODE COMMANDS */


Proc Zfillchk()
{
	extern Proc (**Funcs)();
	extern unsigned Cmd;
	Boolean tmp;
	Mark *tmark;

	if( Cmd == CR ) Cmd = NL;
	if( Bgetcol(TRUE, 0) < Fillwidth() || InPaw )
		(*Funcs[Cmd == NL ? ZNEWLINE : ZINSERT])();
	else
	{
		tmark = Bcremrk();
		while( Bgetcol(TRUE, 0) > Fillwidth() )
		{
			Moveto( Isspace, BACKWARD );
			Movepast( Isspace, BACKWARD );
		}
		Zdelwhite();
		tmp = !Bisatmrk( tmark );
		Binsert( NL );
		Tindent( Margin() );
		if( tmp )
		{
			Bpnttomrk( tmark );
			Binsert( Cmd );
		}
		Unmark( tmark );
	}
}


Proc Zdelwhite()
{
	while( !Bisend() && Iswhite() )
		Bdelete( 1 );
	while( !Bisstart() )
	{
		Bmove( -1 );
		if( Iswhite() )
			Bdelete( 1 );
		else
		{
			Bmove1();
			break;
		}
	}
}


/* Not reentrant - must iterate for arg */
Proc Zfillpara()
{
	Boolean all;	
	Mark *tmark, *tmp;

	if(Curbuff->bmode & CMODE)
	{
		Zfillcomment();
		return;
	}
	if(Curbuff->bmode & PROGMODE)
	{
		Echo("Not in program mode");
		Tbell();
		return;
	}
	tmark = Bcremrk();		/* save the current point */
	if((all = Arg == 0) == TRUE) Btostart();
	Echo( "Reformatting..." );
	do
	{
		/* mark the end of the paragraph and move the point to the start */
		Zfpara();
		Movepast( Isspace, BACKWARD );
		tmp = Bcremrk();
		Zbpara();
		if(Buff() == '.') Bcsearch('\n');	/* for nroff */
	
		/* main loop */
		while( Bisbeforemrk(tmp) )
		{
			Moveto( Isspace, FORWARD );
			if( Bgetcol(TRUE, 0) > Fillwidth() )
			{
				Moveto( Isspace, BACKWARD );
				Zdelwhite();
				Binsert( NL );
				if( Margin() ) Tindent( Margin() );
				Moveto( Isspace, FORWARD );
			}
			Movepast( Iswhite, FORWARD );
			if( Buff() == NL && Bisbeforemrk(tmp) )
			{
				Bdelete( 1 );
				Zdelwhite();
				Binsert( ' ' );
			}
		}

		Unmark( tmp );
		Movepast( Isspace, FORWARD );		/* setup for next iteration */
	}
	while( (all || --Arg > 0) && !Bisend() && !Tkbrdy() );
	
	Clrecho();
	if( Arg > 0 || (all && !Bisend()) )
	{
		Echo( "Aborted" );
		Tgetcmd();
	}

	Bpnttomrk( tmark );						/* restore point */
	Unmark( tmark );
}


Proc Zfpara()
{
	char pc;
	
	/* Only go back if between paras */
	if( Isspace() ) Movepast( Isspace, BACKWARD );
	for(pc = '\0'; !Bisend() && !Ispara(pc, Buff()); pc = Buff(), Bmove1() ) ;
	Movepast( Isspace, FORWARD );
}



Proc Zbpara()
{
	char pc;
	
	Movepast( Isspace, BACKWARD );
	for(pc = '\0'; !Bisstart() && !Ispara(Buff(), pc); pc = Buff(), Bmove(-1)) ;
	Movepast( Isspace, FORWARD );
}


#ifdef __STDC__
Boolean Ispara( char pc, char ch )
#else
Boolean Ispara( pc, ch )
char pc, ch;
#endif
{
	/* We consider a FF, VT, or two NLs in a row to mark a paragraph.
	 * A '.' at the start of a line also marks a paragraph (for nroff)
	 */
	return( ch == '\f' ||
			ch == '\13' ||
			(pc == NL && (ch == NL || ch == '.')));
}


/* FORM MODE COMMANDS */

Proc Zformtab()
{
	if(Bsearch(FORMSTRING, TRUE))
		Bdelete(strlen(FORMSTRING));
	else
		Tbell();
}


/* MISC COMMANDS */


Proc Zprintpos()
{
	char str[ STRMAX ];
	unsigned long mark, point;
	unsigned line;

	Bswappnt( Curbuff->mark );
	mark = Blocation( &line );
	Bswappnt( Curbuff->mark );
	point = Blocation( &line );
	sprintf( str, "Line: %u  Column: %u  Point: %lu  Mark: %lu  Length: %lu",
				line, Bgetcol(FALSE, 0) + 1, point, mark, Blength(Curbuff) );
	Echo( str );
}


Proc Znotimpl()
/* Key has no binding */
{
	Tbell();
}


Proc Zsetmrk()
{
	Bmrktopnt( Curbuff->mark );
	Echo( "Mark Set." );
}


Proc Zquit()
{
	extern int ExitStatus;
	extern Buffer *Bufflist;
	Buffer *tbuff;
	Boolean modf = FALSE;

	if(!Argp)
	{
		for( tbuff = Bufflist; tbuff; tbuff = tbuff->next )
			if( tbuff->bmodf && !(tbuff->bmode & SYSBUFF) )
				modf = TRUE;
		if( modf && Ask("Modified buffers. Quit anyway? ") != YES ) return;
	}
	Quit();
	ExitStatus = 1;
}


/*
Exit the editor.
Warn about makes in progress.
If a buffer is modified, ask to write it out.
Dosen't save the system buffers.
*/
Proc Zexit()
{
#if PIPESH || XWINDOWS
	Buffer *make;

	if((make = Cfindbuff(MAKEBUFF)) && make->child != EOF)
		if(Ask("You have a make running. Kill it?") != YES)
			return;
#endif

	if( Saveall(Argp) )
		Quit();
}


void Quit()
{
	extern Boolean Exitflag;
#if PIPESH || XWINDOWS
	extern Buffer *Bufflist;
	Buffer *tbuff;

	for( tbuff = Bufflist; tbuff; tbuff = tbuff->next )
		if( tbuff->child != EOF )
			Unvoke( tbuff, FALSE );
	Checkpipes(0);		/* make sure waited for ALL children */
#endif
	if( Vars[VDOSAVE].val ) Save( Curbuff );
	Exitflag = TRUE;
	KillHelp();
	Tfini();

#if XWINDOWS
	closeSockets();
#endif

	exit(0);
}


/*
Prompt to save ALL modified non-system buffers.

If the user aborts a prompt, he is left in the buffer he aborted in
and the routine returns false.
Else the user returns to the buffer he started in.
*/
Boolean Saveall( must )
Boolean must;
{
	extern Buffer *Bufflist;
	Buffer *tbuff, *bsave;

	bsave = Curbuff;
	for( tbuff = Bufflist; tbuff; tbuff = tbuff->next )
		if( !(tbuff->bmode & SYSBUFF) && !Promptsave(tbuff, must) )
		{
			Curwdo->modeflags = INVALID;
			return( FALSE );
		}
	Bswitchto( bsave );
	return( TRUE );
}


/*
Prompt to save buffer if the buffer has been modified.
Always saves if 'must' is TRUE or SaveOnExit is set.
Returns FALSE if the user ABORTS the prompt.
*/
Boolean Promptsave( tbuff, must )
Buffer *tbuff;
Boolean must;
{
	char str[ BUFNAMMAX + 20 ];
	int ok = YES;

	if( tbuff->bmodf )
	{
		if( !must && !Vars[VSAVE].val )
		{
			sprintf( str, "Save buffer %s? ", tbuff->bname );
			if( (ok = Ask(str)) == ABORT )
				return( FALSE );
		}
		if( ok == YES || Vars[VSAVE].val )
		{
			Bswitchto( tbuff );
			if( Filesave() != TRUE )
				return( FALSE );
		}
	}
	return( TRUE );
}

Proc Zinsert()
/* Self inserting commands */
{
	extern unsigned Cmd;

	if(Curbuff->bmode & OVERWRITE)
	{
 		if(!Bisend() && Buff() != NL )
			Bdelete( 1 );
	}

	Binsert( Cmd );
	Mshow(Cmd);
}


/*
Handle the CR or NL keys.
In overwrite mode, a NL goes to start of next line.
In insert mode, its just inserted.
This also causes the current line of text to be sent to the down the pipe.
*/
Proc Znewline()
{
	if( Curbuff->bmode & OVERWRITE )
		Bcsearch( NL );
	else
		Binsert( NL );
#if PIPESH
	if( Curbuff->out_pipe )	Sendtopipe();
#endif
}


Proc Zoverin()
{
	Curbuff->bmode ^= OVERWRITE;
	if( !InPaw )
		Curwdo->modeflags = INVALID;
	Arg = 0;
}


Proc Zcase()
{
	extern Boolean Insearch;

	Curbuff->bmode ^= EXACT;
	if( InPaw && Insearch )
	{
		Tsetpoint( Tmaxrow() - 1, 0 );
		Tprntstr( Nocase(NULL) );
	}
	else
		Echo( Curbuff->bmode & EXACT ? "Exact Set" : "Exact Reset" );
	Arg = 0;
}


Proc Zarg()
{
	extern Byte Keys[];
	extern Proc (**Funcs)();
	extern unsigned Cmd;
	char str[ STRMAX ], *p;

	Argp = TRUE;
	Arg = 0;
	strcpy( str, "Arg: 0" );
	p = str + 5;	/* point to first digit */
	PutPaw( str, 2 );
	while( (Cmd = Tgetcmd()) >= '0' && Cmd <= '9' )
	{
		Arg = Arg * 10 + Cmd - '0';
		*p++ = Cmd;
		*p = '\0';
		PutPaw( str, 2 );
	}
	Clrecho();
	(*Funcs[Keys[Cmd]])();
}


Proc Zmeta()
/* Process Meta (ESC) commands. */
{
	extern Byte Keys[];
	extern unsigned Cmd;
	extern Proc (**Funcs)();
	Boolean tmp;

	tmp = Delayprompt( "Meta: " );
	Cmd = Tgetkb() | 128;
	if( tmp ) Clrecho();
	(*Funcs[ Cmd < SPECIAL_START ? Keys[Cmd] : ZNOTIMPL ])();
}



Proc Zctrlx()
/* Process ^X commands. */
{
	extern Byte Keys[];
	extern unsigned Cmd;
	extern Proc (**Funcs)();
	Boolean tmp;

	tmp = Delayprompt( "C-X: " );
	Cmd = Tgetcmd() | 256;
	if( tmp ) Clrecho();
	(*Funcs[ Cmd < SPECIAL_START ? Keys[Cmd] : ZNOTIMPL ])();
}


/* Process the M-X command */
Proc Zmetax()
{
	extern Proc (**Funcs)();
	extern struct cnames Cnames[];
	extern Byte Lfunc;
	extern unsigned Cmd;

	int rc = Getplete("M-X: ", NULL, (char **)Cnames, CNAMESIZE, NUMFUNCS);
	if( rc != -1 )
	{
		Cmd = Cnames[ rc ].fnum;
		Lfunc = ZMETAX;
		for(Arg = Arg == 0 ? 1 : Arg; Arg > 0; --Arg)
			(*Funcs[Cnames[rc].fnum])();
	}
}


Proc Zabort()
{
	Tbell();
	Arg = 0;
	if( InPaw ) InPaw = ABORT;
}


Proc Zquote()
{
	extern long strtol();
	extern unsigned Cmd;
	Boolean tmp;
	char n[ 3 ];

	tmp = Delayprompt( "Quote: " );
	Cmd = Tgetkb();
	if( isxdigit(Cmd) )
	{
		n[ 0 ] = Cmd;
		n[ 1 ] = Tgetkb();
		n[ 2 ] = '\0';
		Cmd = (int) strtol( n, NULL, 16 );
	}
	while( Arg-- > 0 )
		if( InPaw )
			Pinsert();
		else
			Binsert( Cmd );
	if( tmp ) Clrecho();
	Arg = 0;
}

Proc Zhexout()
{
	char str[ STRMAX ], *p;

	if( Arg > 25 ) Arg = 25;
	strcpy( str, "Hex: " );
	for( p = str; Arg-- > 0 && !Bisend(); Bmove1() )
	{
		p = strchr( p, '\0' );
		sprintf( p, " %02x", Buff() & 0xff );
	}
	Echo( str );
}


Proc Zswapchar()
{
	int tmp;

	if( Bisend() || Buff() == NL ) Bmove( -1 );
	if( !Bisstart() ) Bmove( -1 );
	tmp = Buff();
	Bdelete( 1 );
	Bmove1();
	Binsert( tmp );
}


Proc Ztab()
{
	int tcol;

    if( Vars[VSPACETAB].val )
	{
		tcol = Tabsize - (Bgetcol(FALSE, 0) % Tabsize);
		Sindent( tcol );
	}
    else
        Binsert( '\t' );
}

#if !XWINDOWS
Proc Zzoom()	{ Tbell(); }
#endif
