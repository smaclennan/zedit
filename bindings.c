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
#include <sys/stat.h>

Byte CRdefault = ZNEWLINE;
extern Byte Keys[];

Proc Zbind()
{
	extern struct cnames Cnames[];
	int f, key;
	
	if( Argp )
	{
		Bind();
		Arg = 0;
		return;
	}
	f = Getplete("Bind: ", (char *)NULL, (char **)Cnames, CNAMESIZE, NUMFUNCS);
	if( f != -1 )
	{
		if( Bindone("Key: ", TRUE, &key) )
		{
			Keys[ key ] = Cnames[ f ].fnum;
			if( key == CR ) CRdefault = Keys[ key ];
		}
		else
			Tbell();
	}
	Clrecho();
}


Proc Zkeybind()
{
	extern Byte Keys[];
	extern struct cnames Cnames[];
	char str[ STRMAX ];
	int rc;
	unsigned key;

	Arg = 0;
	Echo( "Key: " );
	if( (key = Keys[Tgetcmd()]) == ZCTRLX )
	{
		Echo( "Key: C-X " );
		key = Keys[ Tgetcmd() + 256 ];
	}
	else if( key == ZMETA )
	{
		Echo( "Key: M-" );
		key = Keys[ Tgetcmd() + 128 ];
	}
	if( key == ZNOTIMPL )
		Echo( "Unbound" );
	else
		for( rc = 0; rc < NUMFUNCS; ++rc )
			if( Cnames[rc].fnum == key )
			{
				sprintf( str, "Bound to %s", Cnames[rc].name );
				Echo( str );
			}
}


Proc Zcmdbind()
{
	extern struct cnames Cnames[];
	char line[STRMAX];
	int f, k, found = 0;
	
	Arg = 0;
	*PawStr = '\0';
	f = Getplete("Command: ", NULL, (char **)Cnames, CNAMESIZE, NUMFUNCS);
	if(f != -1)
	{
		for(k = 0; k < NUMKEYS; ++k)
			if(Keys[k] == Cnames[f].fnum)
				/*
				 * Don't display both C-X A and C-X a if bound to same
				 * Ditto for Meta
				 */
				if(((k < (256 + 'a') || k > (256 + 'z')) &&
					(k < (128 + 'a') || k > (128 + 'z'))) ||
					Keys[k] != Keys[k - ('a' - 'A')])
				{
					if(found) strcat(PawStr, " or ");
					strcat(PawStr, Dispkey(k, line));
					if(strlen(PawStr) > Colmax) break;
					found = TRUE;
				}
		if(found)
			Echo(PawStr);
		else
			Echo("Unbound");
	}
}


Proc Zdispbinds()
{
	extern Proc (*Vcmds[])(), (*Pawcmds[])(), Znotimpl();
	extern struct cnames Cnames[];

	Boolean found;
	FILE *fp;
	char line[ STRMAX ];
	int j, f;
	unsigned k;

	if(Argp)
	{
		*line = '\0';
		if(Getarg("Output file: ", line, STRMAX)) return;
		if((fp = fopen(line, "w")) == NULL)
		{
			Echo("Unable to create");
			return;
		}
	}
	else
	{
#ifdef BORDER3D
		Tbell();
		return;
#else
		fp = NULL;
		WuseOther(LISTBUFF);
#endif
	}
	Echo( "Please Wait..." );
	Out("COMMAND                            PERMS     BINDING\n", fp);
	for( f = 0; f < NUMFUNCS; ++f )
		if( Cnames[f].fnum != ZNOTIMPL && Cnames[f].fnum != ZINSERT)
		{
			sprintf( line, "%-35s%cw%c       ", Cnames[f].name,
					 Vcmds[Cnames[f].fnum]   == Znotimpl ? '-' : 'r',
					 Pawcmds[Cnames[f].fnum] == Znotimpl ? '-' : 'p' );
			Out(line, fp);
			found = FALSE;
			for( k = 0; k < NUMKEYS; ++k )
				if( Keys[k] == Cnames[f].fnum )
					/*
					 * Don't display both C-X A and C-X a if bound to same
					 * Ditto for Meta
					 */
					if(((k < (256 + 'a') || k > (256 + 'z')) &&
						(k < (128 + 'a') || k > (128 + 'z'))) ||
						Keys[k] != Keys[k - ('a' - 'A')] )
					{
						if( found )
							for(j = Bgetcol(FALSE, 0); j < 45; ++j)
								Out(" ", fp);
						Out(Dispkey(k, line), fp);
						Out("\n", fp);
						found = TRUE;
					}
			if( !found )
				Out("Unbound\n", fp);
		}
	Btostart();
	if(!fp) Curbuff->bmodf = FALSE;
	Clrecho();
	Arg = 0;
}

Proc Out(line, fp)
char *line;
FILE *fp;
{
	if(fp)
		fputs(line, fp);
	else
		Binstr(line);
}


static char *BindFname(fname)
char *fname;
{
#if TERMINFO || ANSI
	extern char *Term;
	sprintf(fname, ".zb.%s", Term);
#elif XWINDOWS
	strcpy(fname, ".zb.X");
#else
	strcpy(fname, ZBFILE);
#endif
	return fname;
}
	
void Loadbind()
{
	char fname[30], path[PATHMAX + 1];
	int i;

	BindFname(fname);
	for(i = FINDPATHS; i && (i = Findpath(path, fname, i, TRUE)); --i)
		Bindfile(path, READ_BINARY);
#if DBG
	Fcheck();
#endif
}


/* Save a bindings file.
 * Use Findpath starting at the $HOME directory to find where to
 * save the bindings file. If no bindings file exists, save in the
 * $HOME dir.
 */
Proc Zsavebind()
{
	char fname[30], path[PATHMAX + 1];
	int i, n;

	BindFname(fname);
	for(n = 0, i = 3; i && (i = Findpath(path, fname, i, TRUE)); --i) n = i;
	if(n)
		Findpath(path, fname, n, TRUE);
	else
		sprintf(path, "%s/%s", Me->pw_dir, fname);
	if(Argp && Getfname("Bind File: ", path)) return;
	if(Bindfile(path, WRITE_MODE))
	{
		sprintf(PawStr, "%s written.", path);
		Echo(PawStr);
	}
}

#define OLD_WAY
#ifdef OLD_WAY
Boolean Bindfile(fname, mode)
char *fname;
int mode;
{
	extern int Cmask;
	char version[ 3 ];
	int fd, modesave, rc = FALSE;

	if( (fd = open(fname, mode, Cmask)) != EOF )
	{
		modesave = Curbuff->bmode;		/* set mode to normal !!! */
		Curbuff->bmode = NORMAL;
		Curwdo->modeflags = INVALID;
		if( mode == WRITE_MODE )
		{
			write( fd, "01", 2 );
			if( write(fd, (char *)Keys, 510) != 510 )
				Error( "Unable to Write Bindings File" );
			else
				rc = TRUE;
		}
		else
		{
			read( fd, version, 2 );
			if( *version != '0' )
				Error( "Incompatible Bindings File" );
			else if( read(fd, (char *)Keys, NUMKEYS) == -1 )
				Error( "Unable to Read Bindings File" );
			else
			{
				CRdefault = Keys[ CR ];
				rc = TRUE;
			}
		}
		(void)close( fd );
		Curbuff->bmode = modesave;
		Curwdo->modeflags = INVALID;
	}
	else if( mode == WRITE_MODE )
		Error( "Unable to Create Bindings File" );
	return( rc );
}
#else
Boolean Bindfile(fname, mode)
char *fname;
int mode;
{
	extern struct cnames Cnames[];
	FILE *fp;
	int modesave, rc = FALSE;
	int i, j;

	if((fp = fopen(fname, mode == WRITE_MODE ? "w" : "r")) != 0)
	{
		modesave = Curbuff->bmode;		/* set mode to normal !!! */
		Curbuff->bmode = NORMAL;
		Curwdo->modeflags = INVALID;
		if( mode == WRITE_MODE )
		{
			for(i = 0; i < NUMKEYS; ++i)
				for(j = 0; j < NUMFUNCS; ++j)
					if(Keys[i] == Cnames[j].fnum)
					{
						fprintf(fp, "%s\t%d\n", Cnames[j].name, i);
						break;
					}
			rc = TRUE;
		}
		else
		{
			char name[40];
			int key;
			
			*name = '?';
			if(fscanf(fp, "%s\t%d\n", name, &key) == 2)
			{	/* new way */
				rewind(fp);
				while(fscanf(fp, "%s\t%d\n", name, &key) == 2)
				{
					for(j = 0; j < NUMFUNCS; ++j)
						if(strcmp(name, Cnames[j].name) == 0)
						{
							Keys[key] = Cnames[j].fnum;
							break;
						}
				}
				rc = TRUE;
			}
			else if(*name == '0')
			{	/* old way */
				int fd;
				
				if((fd = open(fname, READ_BINARY)) != EOF)
				{
					read(fd, name, 2);
					rc = read(fd, (char *)Keys, NUMKEYS);
					close(fd);
					if(rc == -1)
						Error("Unable to Read Bindings File");
					else
					{
						CRdefault = Keys[CR];
						rc = TRUE;
					}
				}
			}
		}
		fclose(fp);
		Curbuff->bmode = modesave;
		Curwdo->modeflags = INVALID;
	}
	else if( mode == WRITE_MODE )
		Error( "Unable to Create Bindings File" );
	return rc;
}
#endif

Boolean Bindone( prompt, first, key )
char *prompt;
int first, *key;
{		
	extern unsigned Cmd;
	
	Echo( prompt );
	if( Keys[*key = Tgetcmd()] == ZABORT )
		return( FALSE );
	else if( Keys[*key] == ZQUOTE )
	{
		Arg = 0;
		Zquote();
		*key = Cmd;
	}
	else if( first && Keys[*key] == ZMETA )
		if( Bindone("Key: M-", FALSE, key) )
			*key += 128;
		else
			return( FALSE );
	else if( first && Keys[*key] == ZCTRLX )
	{
		if( Bindone("Key: C-X ", FALSE, key) )
			*key += 256;
		else
			return( FALSE );
	}
	return( TRUE );
}
