/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

/* Ask Yes/No question.
 * Returns YES, NO, or ABORT
 */
int Ask(msg)
char *msg;
{
	extern Byte Keys[];
	int rc;
	unsigned cmd;
	
	Echo(msg);
	do
		switch(cmd = Tgetcmd())
		{
			case 'y':
			case 'Y':
				rc = YES;
				break;

			case 'N':
			case 'n':
				rc = NO;
				break;

			default:
				Tbell();
				rc = (Keys[cmd] == ZABORT) ? ABORT : BADCHAR;
				break;
		}
	while(rc == BADCHAR);
	Clrecho();
	return rc;
}


/* Move a block of chars around point and "from" to "to".
 * Assumes point is before "from".
*/
void Blockmove( from, to )
Mark *from, *to;
{
	char tmp;

	while( Bisbeforemrk(from) )
	{
		tmp = Buff();
		Bswappnt( to );
		Binsert( tmp );
		Bswappnt( to );
		Bdelete( 1 );
	}
}


#if !XWINDOWS
/* more efficient to not make it a macro */
void Clrecho() { PutPaw( "", 2 ); }
#endif

Boolean Isext( fname, ext )
char *fname, *ext;
{
	extern char *strrchr();
	char *ptr;

	return(fname && (ptr = strrchr(fname, '.')) && strcmp(ptr, ext) == 0);
}


Boolean zCursor()
/* Was the last command a "cursor" command? */
{
	extern Byte Lfunc;
	
	return( Lfunc == ZPREVLINE || Lfunc == ZNEXTLINE ||
			Lfunc == ZPREVPAGE || Lfunc == ZNEXTPAGE );
}


Boolean Delayprompt( msg )
char *msg;
{
	int rc;
	
	if((rc = Delay())) PutPaw(msg, 2);
	return(rc);
}


#if !XWINDOWS
#include <sys/time.h>
#include <sys/poll.h>

Boolean Delay()
{
	static struct pollfd ufd = { .fd = 1, .events = POLLIN };

	return InPaw || Tkbrdy() || poll(&ufd, 1, 1000) == 1 ? 1 : 0;
}

#else

Boolean Delay()
{
	long t;

	if( InPaw || Mstate == INMACRO ) return( FALSE );
	t = time( (long *)0 ) + 2;	/* at least 1 second */
    do
    	if( Tkbrdy() ) return( FALSE );
    while( time((long*)0) < t );
	return( TRUE );
}
#endif

/* Was the last command a delete to kill buffer command? */
Boolean Delcmd()
{
	extern Byte Lfunc;

	return	Lfunc == ZDELEOL  || Lfunc == ZDELLINE  || Lfunc == ZDELRGN   ||
			Lfunc == ZCOPYRGN || Lfunc == ZDELWORD  || Lfunc == ZRDELWORD ||
			Lfunc == ZMAKEDEL || Lfunc == ZGETBWORD;
}

/* Was the last command a delete of any type? */
Boolean DelcmdAll()
{
	extern Byte Lfunc;

	return Delcmd() || Lfunc == ZDELCHAR || Lfunc == ZRDELCHAR;
}

char PawStr[ COLMAX + 10 ];

#if !XWINDOWS
/*
Put a string into the PAW.
type is:	0 for echo			Echo()		macro
			1 for error			Error()		macro
			2 for save pos
*/
void PutPaw( str, type )
char *str;
int type;
{
	int trow, tcol;

	if(type == 1) Tbell();
	if(!InPaw)
	{
		trow = Prow; tcol = Pcol;
		Tsetpoint(Tmaxrow() - 1, 0);
		Tprntstr(str);
		Tcleol();
 		if(type != 1) Tsetpoint(trow, tcol);
		Tforce();
		Tflush();
		if(type == 1 && Mstate != INMACRO) Tgetcmd();
	}
}
#endif

/*
Find the correct path for the config files.
There are 4 config files: bindings, config, help, and save
	bindings	- $EXE  + $HOME + . + ConfigDir
	config		- $EXE  + $HOME + . + ConfigDir
	help		- $EXE  | $HOME | . | ConfigDir
	save		- $HOME	| . | ConfigDir

returns:	4 for $EXE
			3 for $HOME
			2 for .
			1 for ConfigDir
			0 for not found, 'path' set to 'fname'
*/
int Findpath( p, f, s, m )
char *p, *f;
int s;
Boolean m;
{
	extern char *getenv(), *strcpy();
	extern char *Thispath, *ConfigDir;

	switch(s)
	{
		case 4:	 if(Isfile(p, Thispath, f, m))		return 4;
		case 3:	 if(Isfile(p, Me->pw_dir, f, m))	return 3;
		case 2:	 if(Isfile(p, ".", f, m))			return 2;
		case 1:  if(Isfile(p, ConfigDir, f, m))		return 1;
		default: strcpy(p, f);						return 0;
	}
}


/*
	Get the word at the current buffer point and store in 'word'.
	Get at the most 'max' characters.
	Leaves the point alone.
*/
Boolean Getbword( word, max, valid )
char word[];
int max;
int (*valid)();
{
	int i;
	Mark tmark;

	Bmrktopnt( &tmark );
	Moveto( Istoken, FORWARD );
	if( Bisend() )
		Moveto( Istoken, BACKWARD );
	Movepast( Istoken, BACKWARD );
	for( i = 0; !Bisend() && valid() && i < max; ++i, Bmove1() )
		word[ i ] = Buff();
	word[ i ] = '\0';
	Bpnttomrk( &tmark );
	return( i );
}


/*
	Get the current buffer text and store in 'txt'.
	Get at the most 'max' characters.
	Leaves the point alone.
*/
char *Getbtxt( txt, max )
char txt[];
int max;
{
	int i;
	Mark tmark;

	Bmrktopnt( &tmark );
	for( Btostart(), i = 0; !Bisend() && i < max; Bmove1(), ++i )
		txt[ i ] = Buff();
	txt[ i ] = '\0';
	Bpnttomrk( &tmark );
	return( txt );
}


void Killtomrk( tmark )
Mark *tmark;
{
	Copytomrk( tmark );
	Bdeltomrk( tmark );
}


void Movepast( pred, forward )
int (*pred)();
Boolean forward;
/* Go forward or back past a thingy */
{
	if( !forward ) Bmove( -1 );
	while( !(forward ? Bisend() : Bisstart()) && (*pred)() )
		Bmove( forward ? 1 : -1 );
	if( !forward && !(*pred)() ) Bmove1();
}


void Moveto( pred, forward )
int (*pred)();
Boolean forward;
/* Go forward or back to a thingy */
{
	if( !forward ) Bmove( -1 );
	while( !(forward ? Bisend() : Bisstart()) && !(*pred)() )
		Bmove( forward ? 1 : -1 );
	if( !forward && !Bisstart() ) Bmove1();
}


char *Strup( str )
char *str;
{
	char *ptr;

	for( ptr = str; *ptr; ++ptr ) *ptr = Toupper( *ptr );
	return( str );
}


void Tindent( arg )
int arg;
/* Put in the right number of tabs and spaces */
{
    extern struct avar Vars[];

    if( Vars[VSPACETAB].val == 0 )
		for( ; arg >= Tabsize; arg -= Tabsize )
			Binsert( '\t' );
	Sindent( arg );
}


int Toupper( c )
int c;
{
	return( TOUPPER(c) );
}


int Tolower( c )
int c;
{
	return( TOLOWER(c) );
}


int Isspace()
/* Must be a real function */
{
	return( isspace(Buff()) );
}



int Isword()
{
	return( isalnum(Buff()) || Buff() == '_' || Buff() == '.' ||
			Buff() == '$' );
}


int Istoken()
/* Must be a real function. $ for PL/M */
{
/* SAM added / as token char */
	return( isalnum(Buff()) || Buff() == '_' || Buff() == '.' ||
			Buff() == '$' || Buff() == '/' );
}


int Iswhite()
/* Must be a real function */
{
	return( STRIP(Buff()) == ' ' || STRIP(Buff()) == '\t' );
}


char *Bakname( bakname, fname )
char *bakname, *fname;
{
	strcpy( bakname, fname );
	strcat( bakname, "~" );
	return( bakname );
}


Boolean Mv(from, to)
char *from, *to;
{
	Boolean rc;

	unlink(to);
	if((rc = link(from, to)) == 0)
		unlink(from);
	return rc;
}


Boolean Cp(from, to)
char *from, *to;
{
	FILE *in, *out;
	char buf[1024];
	int n, rc= TRUE;
	
	if((in = fopen(from, "r")) == NULL || (out = fopen(to, "w")) == NULL)
		return FALSE;
	while((n = fread(buf, 1, 1024, in)) > 0)
		if(fwrite(buf, 1, n, out) != n)
		{ 
			rc = FALSE;
			break;
		}
	fclose(in);
	fclose(out);
	return rc;
}


#if XWINDOWS
/* Move the buffer point to an absolute row, col */
void Pntmove(row, col)
int row, col;
{
	extern Mark Scrnmarks[];
	extern int Pawcol;
	WDO *wdo;
	int i;

	if(InPaw)
	{	/* Can't move out of paw */
		if(row != Rowmax - 1 || col < Pawcol)
			Tbell();
		else
		{
			col = Bmakecol(col - Pawcol, FALSE);
			Tsetpoint(Rowmax - 1, col);
		}
		return;
	}

#ifdef BORDER3D
	if(row < Curwdo->first || row >= Curwdo->last) { Tbell(); return; }
	for(i = Curwdo->first; i < row; ++i)
	{
		Bpnttomrk(&Scrnmarks[i]);
		if(Bisend())
		{	/* at end of buffer - stop */
			if(i > wdo->first) --i;
			col = Colmax;
			break;
		}
	}
	Bpnttomrk(&Scrnmarks[i]);
	col = Bmakecol(col, FALSE);
	Tsetpoint(i, col);
#else
	/* don't move Point into Paw or mode row */
	for(wdo = Whead; wdo; wdo = wdo->next)
		if(row >= wdo->first && row < wdo->last)
		{	/* find offset in window */
			for(i = wdo->first; i < row; ++i)
			{
				Wswitchto(wdo);
				Bpnttomrk(&Scrnmarks[i]);
				if(Bisend())
				{	/* at end of buffer - stop */
					if(i > wdo->first) --i;
					col = Colmax;
					break;
				}
			}
			Bpnttomrk(&Scrnmarks[i]);
			col = Bmakecol(col, FALSE);
			Tsetpoint(i, col);
			return;
		}
	Tbell();
#endif
}
#endif


/* Limit a filename to at most Tmaxcol() - 'num' cols */
char *Limit(fname, num)
char *fname;
int num;
{
	int off;
	
	off = strlen(fname) - (Tmaxcol() - num);
	return(off > 0 ? fname + off : fname);
}


/* called at startup when out of memory */
void NoMem()
{
	Error("Out of memory.");
	exit(1);
}


#ifndef __STDC__
int Strnicmp( s, t, n )
char *s, *t;
int n;
{
	int i;
	
	for( i = 1; Tolower(*s) == Tolower(*t); ++s, ++t, ++i )
		if( *s == '\0' || i == n )
			return( 0 );
	return( Tolower(*s) - Tolower(*t) );

}
#endif

/* Find first occurance in str1 of str2. NULL if not found.
 * Case insensitive!
 */
char *Strstr( str1, str2 )
char *str1, *str2;
{
	int i, len, max;
	
	len = strlen( str2 );
	max = strlen( str1 ) - len;
	for( i = 0; i <= max; ++i )
		if( Strnicmp(&str1[i], str2, len) == 0 )
			return( &str1[i] );
	return( NULL );
}
