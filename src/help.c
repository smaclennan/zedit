/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"
#include "help.h"

#include "keys.h"

extern struct cnames Cnames[];
 
char *Htype[] =
{
	"Special",
	"Other",
	"Variables",
	"Cursor",
	"Copy/Delete",
	"Search/Replace",
	"Macro",
	"File",
	"Buffer/Window",
	"Display",
	"Mode",
	"Help/Status",
	"Bindings",
	"Shell",	
};
#define HTYPES	(sizeof(Htype) / sizeof(char *))


#if FORK_ZHELP
#include <signal.h>
#include <sys/wait.h>
 
int HelpChild = 0;

Proc Zhelp()
{
	extern Proc TextHelp();
	
	KillHelp();
	if((HelpChild = fork()) == 0)
	{
		char path[PATHMAX];
		Findpath(path, "help.z.x", FINDPATHS, TRUE);
		execlp("zhelp", "zhelp", path, NULL);
		exit(666);
	}
	
	if(HelpChild == 0)
#ifdef BORDER3D
		Tbell();
#else
		TextHelp();
#endif
}

void KillHelp()
{
	if(HelpChild == 0) return;
	if(kill(HelpChild, SIGKILL) == 0)
		wait(NULL);		/* SAM Should check this */
	HelpChild = 0;
}

#else
void KillHelp()	{}
#endif	

#ifndef BORDER3D
/* a "sentence" ends in a tab, NL, or two consecutive spaces */
int Issentence()
{
	static Byte prev = '\0';
	int rc;
	
	rc = Buff() != '\t' && Buff() != NL && !(Buff() == ' ' && prev == ' ');
	prev = rc ? Buff() : '\0';
	return(rc);
}


#if FORK_ZHELP
Proc TextHelp()
#else
Proc Zhelp()
#endif
{
	extern char Lbufname[];
	static Byte level = 0, z;
	Buffer *tbuff, *was;
	FILE *fp = NULL;		/*shutup*/
	char str[ STRMAX + 1 ];
	int i;

	if( level && (tbuff = Cfindbuff(HELPBUFF)) != Curbuff )
	{	/* just switch to the .help buffer */
		strcpy( Lbufname, Curbuff->bname );
		Bgoto(tbuff);
		return;
	}
	switch( level )
	{
		case 0:
			/* create the window */
			for( z = 0; z < NUMFUNCS && Cnames[z].fnum != ZHELP; ++z ) ;
			if((fp = Findhelp(z, TRUE, str)) == 0) return;
			was = Curbuff;
#ifdef SAM_OLD_WAY
			if( (tbuff = Cmakebuff(HELPBUFF, NULL)) )
			{
				tbuff->bmode |= SYSBUFF;
				strcpy( Lbufname, was->bname );
				tbuff->bmode |= VIEW;
				Reframe();
				Curwdo->modeflags = INVALID;
			}
#endif
			if(WuseOther(HELPBUFF))
			{
				strcpy(Lbufname, was->bname);
				Curbuff->bmode |= VIEW;
			}
			else
			{
				fclose(fp);
				break;
			}
			/* fall thru to level 1 */
			
		case 1:
			/* read in the top level */
			if(level == 0 || (fp = Findhelp(z, TRUE, str)) != 0)
			{
				Bempty();
				while( fgets(str, STRMAX, fp) && *str != ':' )
					Binstr( str );
				fclose( fp );
				Btostart();
				Bcsearch('n');
				Curbuff->bmodf = FALSE;
				level = 2;
			}
			break;
			
		case 2:
			/* accept input from top level and create secondary level */
			Getbword( str, STRMAX, Issentence );
			for( i = 0; i < HTYPES; ++i )
				if( strcmp(str, Htype[i]) == 0 )
				{
					Helpit( i );
					level = 3;
				}
			break;
			
		case 3:
			/* accept input from secondary level and display help */
			Bmakecol( Bgetcol(FALSE, 0) < 40 ? 5 : 45, FALSE );
			Getbword( str, 30, Issentence );
			if( strncmp(str, "Top", 3) == 0 )
			{
				level = 1;
				Zhelp();
			}
			else
			{
				for( i = 0; i < NUMFUNCS; ++i )
					if( strcmp(str, Cnames[i].name) == 0 )
					{
						level = 4;
						Help( i, TRUE );
						return;
					}
				for( i = 0; i < VARNUM; ++i )
					if( strcmp(str, Vars[i].vname) == 0 )
					{
						level = 4;
						Help( i, FALSE );
						return;
					}
			}
			break;
			
		case 4:
			/* go back to secondary level */
			Helpit(-1);
			level = 3;
			break;
	}
}


/* If type == -1, use saved value */
void Helpit(type )
int type;
{
	static int was;
	char buff[ STRMAX ];
	int col = 5, i;

	Echo( "Please wait..." );
	Bempty();
	if(type == -1)
		type = was;
	else
		was = type;
	sprintf(buff, "- %s Help -\n\n", Htype[type]);
	Tindent((Colmax - strlen(buff)) >> 1);
	Binstr( buff );
	if( type == H_VAR )
		for( i = 0; i < VARNUM; ++i )
			Dispit( Vars[i].vname, &col );
	else
		for( i = 0; i < NUMFUNCS; ++i )
			if( Cnames[i].htype == type )
				Dispit( Cnames[i].name, &col );
	if( col == 45 ) Binsert( '\n' );
	Binstr( "\n     Top Level Help Menu" );
	Btostart();
	Curbuff->bmodf = FALSE;
	Clrecho();
}


void Dispit( name, col )
char *name;
int *col;
{
	Bmakecol( *col, TRUE );
	Binstr( name );
	if( *col == 45 ) Binsert( '\n' );
	*col ^= 40;
}


#ifdef SAM_OLD_WAY
void Help( code, func )
int code;
Boolean func;
{
	extern char Lbufname[];
	extern Byte Keys[];
	extern Mark Scrnmarks[];

	FILE *fp;
	Boolean found = FALSE;
	char buff[ BUFSIZ ], *ptr, *p;
	int trow, k;

	Arg = 0;
	if( code < 0 ) return;
	if( fp = Findhelp(code, func, buff) )
	{
		trow = 0;
		Tstart = 0;
		p = buff + 1;
		do
		{
			Tsetpoint( trow, 0 );
			if( ptr = strchr(buff, '\n') ) *ptr = '\0';
			Tprntstr( p );
			Tcleol();
			Scrnmarks[ trow++ ].modf = TRUE;
		}
		while( fgets(p = buff, BUFSIZ, fp) && *buff != ':' );
		fclose( fp );

		/* setup to display either Bindings or Current value */
		Tsetpoint( trow, 0 );
		Tcleol();				
		Scrnmarks[ trow++ ].modf = TRUE;
		Tsetpoint( trow, 0 );
		if( func )
		{
			if( Cnames[code].fnum != ZNOTIMPL &&
				Cnames[code].fnum != ZINSERT )
			{
				Tprntstr( "Binding(s): " );
				for( k = 0; k < NUMKEYS; ++k )
					if( Keys[k] == Cnames[code].fnum )
						/*
						 * Don't display both C-X A and C-X a if bound to same
						 * Ditto for Meta
						 */
						if(((k < (256 + 'a') || k > (256 + 'z')) &&
							(k < (128 + 'a') || k > (128 + 'z'))) ||
							Keys[k] != Keys[k - ('a' - 'A')] )
						{
							if( found )
								Tprntstr( ",  " );
							else
								found = TRUE;
							Tprntstr( Dispkey(k, buff) );
						}
				if( !found ) Tprntstr( "Unbound" );
			}
		}
		else
		{
			Tprntstr( "Current value: " );
			Varval( &Vars[code] );
		}
		Tcleol();
		Scrnmarks[ trow++ ].modf = TRUE;		/* needed */
		Dwait( trow );
		if( Keys[Tgetcmd()] == ZABORT )
			Cswitchto( Cfindbuff(Lbufname) );
	}
}
#endif

void Help( code, func )
int code;
Boolean func;
{
	extern Byte Keys[];

	FILE *fp;
	Boolean found = FALSE;
	char buff[BUFSIZ], *p;
	int k;

	Arg = 0;
	if(code < 0) return;
	if((fp = Findhelp(code, func, buff)) != 0)
	{
		Bempty();
		p = buff + 1;
		do
			Binstr(p);
		while(fgets(p = buff, BUFSIZ, fp) && *buff != ':');
		fclose( fp );

		/* setup to display either Bindings or Current value */
		if( func )
		{
			if( Cnames[code].fnum != ZNOTIMPL &&
				Cnames[code].fnum != ZINSERT )
			{
				Binstr("\nBinding(s): ");
				for( k = 0; k < NUMKEYS; ++k )
					if( Keys[k] == Cnames[code].fnum )
						/*
						 * Don't display both C-X A and C-X a if bound to same
						 * Ditto for Meta
						 */
						if(((k < (256 + 'a') || k > (256 + 'z')) &&
							(k < (128 + 'a') || k > (128 + 'z'))) ||
							Keys[k] != Keys[k - ('a' - 'A')] )
						{
							if(found)
								Binstr(",  ");
							else
								found = TRUE;
							Binstr(Dispkey(k, buff));
						}
				if( !found ) Binstr( "Unbound" );
			}
		}
		else
		{
			Binstr( "\nCurrent value: " );
			Varval( &Vars[code] );
		}
		Btostart();
	}
}


FILE *Findhelp( code, func, buff )
int code;
Boolean func;
char *buff;
{
	FILE *fp;
	char *ptr;
	int len;
	
	Findpath( buff, ZHFILE, FINDPATHS, TRUE );
	if((fp = fopen(buff, "r")) == 0)
	{
		Echo( "Unable to Open Help File" );
		return( NULL );
	}
	ptr = func ? Cnames[ code ].name : Vars[ code ].vname;
	len = strlen( ptr );

	Echo( "Looking in help file..." );
	while( fgets(buff, STRMAX, fp) )
		if( *buff == ':' && strncmp(ptr, &buff[1], len) == 0 )
		{
			Clrecho();
			return( fp );
		}
	fclose( fp );
	Echo( "No Help" );
	return( NULL );
}
#endif	/* !BORDER3D */


int Isnotws()
{
	return( Buff() != '\n' && Buff() != '\t' && Buff() != ' ' );
}
