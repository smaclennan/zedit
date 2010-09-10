/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"
#include <sys/stat.h>
#include <assert.h>

extern char Lbufname[], Fname[];

Proc Zfindfile()
{
#if XWINDOWS
	if(Argp && StartProg("Zfindfile") == 0)
	{
		Arg = 0;
		return;
	}
#endif
	if(Getfname("Find File: ", Fname) == 0)
		Findfile(Fname, FALSE);
}


Proc Zviewfile()
{
	if( Getfname("View File: ", Fname) ) return;
	Findfile(Fname, FALSE);
	Curbuff->bmode |= VIEW;
	Curwdo->modeflags = INVALID;
}


Proc Zeditfile()
{
	unsigned long offset;
	
	if(!Curbuff->fname)
	{
		Tbell();
		return;
	}
	if(Curbuff->bmodf && Ask("File modifed. Ok to loose changes?") != YES)
		return;
	offset = Blocation(0);
	Breadfile(Curbuff->fname);
	Boffset(offset);
#if XWINDOWS
	Zredisplay();
#endif
}

	
Boolean Findfile(path, startup)
char *path;
int startup;
{
	extern char *strrchr();
	extern Buffer *Bufflist;

	char tbname[ BUFNAMMAX + 1 ];
	char *was;
	Buffer *tbuff;
	int rc = TRUE;

	Arg = 0;
	was = Curbuff->bname;

	/* limit name to BUFNAMMAX */	
	strncpy(tbname, Lastpart(path), BUFNAMMAX);
	tbname[BUFNAMMAX] = '\0';

	/* If is this file already in a buffer - use it.
	 * At startup, we are done.
	 */
	for( tbuff = Bufflist; tbuff; tbuff = tbuff->next )
		if( tbuff->fname && strcmp(path, tbuff->fname) == 0 )
		{
			if(startup) return TRUE;
			Bswitchto( tbuff );
			strcpy( Lbufname, was );
			break;
		}

	if( !tbuff )
	{
		if(Cfindbuff(tbname))
		{	/* Resolve buffer name collisions by creating a unique name */
			char *p;
			int i;
			
			i = strlen(tbname);
			p = &tbname[i < BUFNAMMAX - 3 ? i : BUFNAMMAX - 3];
			i = 0;
			do
				sprintf(p, ".%d", ++i);
			while(Cfindbuff(tbname));
		}

		if(!startup) Loadwdo(tbname);

		rc = Readone( tbname, path );
	}

	if(!startup)
	{
		Cswitchto(Curbuff);
		Reframe();
	}

	return rc;
}


Proc Zsaveall()
{
	if(Argp)
	{
		extern Buffer *Bufflist;
		Buffer *tbuff;
		
		for(tbuff = Bufflist; tbuff; tbuff = tbuff->next)
			if(!(tbuff->bmode & SYSBUFF) && tbuff->fname)
				tbuff->bmodf = MODIFIED;
	}
	Saveall(TRUE);
}


Proc Zfilesave()
{
	if(Argp)
		Saveall(FALSE);
	else
		Filesave();
}


Boolean Filesave()
{
	char path[ PATHMAX + 1 ];
	
	Arg = 0;
	if(Curbuff->fname == NULL)
	{
		*path = '\0';
		if( Getfname("File Name: ", path) == 0 )
			Curbuff->fname = strdup( path );
		else
			return( FALSE );
		Curwdo->modeflags = INVALID;
	}
	sprintf( PawStr, "Writing %s", Lastpart(Curbuff->fname) );
	Echo( PawStr );
	return( Bwritefile(Curbuff->fname) );
}


Proc Zfilewrite()
{
	char path[ PATHMAX + 1 ], *prompt;
		
	Arg = 0;
	prompt = Argp ? "Write Region: " : "Write File: ";
	*path = '\0';
	if( Getfname(prompt, path) == 0 )
	{
		if( Argp )
		{
			sprintf( PawStr, "Writing %s", path );
			Echo( PawStr );
			Write_rgn(path);
			Clrecho();
		}
		else
		{
			if( Curbuff->fname ) free( Curbuff->fname );
			Curbuff->fname = strdup( path );
			Curbuff->mtime = 0;	/* this is no longer valid */
			Zfilesave();
			Curwdo->modeflags = INVALID;
		}
	}
}


/*
 * Write the region to 'path'. Assumes 'path' correct.
 * Returns: TRUE, FALSE, ABORT
 */
int Write_rgn(path)
char *path;
{
	Buffer *tbuff, *save;
	int rc = FALSE;

	save = Curbuff;
	if((tbuff = Cmakebuff("___tmp___", (char *)NULL)) != 0)
	{
		Bswitchto( save );
		Bcopyrgn( Curbuff->mark, tbuff );
		Bswitchto( tbuff );
		Curbuff->bmode = save->bmode;
		rc = Bwritefile( path );
		Bswitchto( save );
		Bdelbuff( tbuff );
	}
	return(rc);
}


Proc Zfileread()
{
	int rc;
	
	if( Getfname("Read File: ", Fname) ) return;
	if((rc = Fileread(Fname)) > 0)
	{
		sprintf( PawStr, "Unable to read %s", Fname );
		Error( PawStr );
	}
}


/* read 'fname' into buffer at Point */
int Fileread(fname)
char *fname;
{
	Buffer *tbuff, *save;
	Mark *tmark;
	int rc = 1;

	save = Curbuff;
	if((tbuff = Bcreate()) != NULL)
	{
		Bswitchto(tbuff);
		Curbuff->bmode = save->bmode;
		if((rc = Breadfile(fname)) == 0)
		{
			Btoend();
			tmark = Bcremrk();
			Btostart();
			Bcopyrgn( tmark, save );
			Unmark( tmark );
		}
		Bswitchto( save );
		Bdelbuff( tbuff );
	}
	return rc;
}


/*
Fixup the pathname. 'to' and 'from' cannot overlap.
- if the path starts with a ~, lookup the user in the /etc/passwd file
- add the current directory if not rooted.
- remove the . and .. entries
Returns -1 if the 'from' is a directory
		 1 if the directory portion of a new file is invalid.
		 2 if ~ specifier invalid
		 0 if all is well
NOTE: assumes a valid path (in particular /.. would not work)
*/

static struct passwd *Getpwnam ARGS((char *name));

int Pathfixup(to, from)
char *to, *from;
{
	extern char *getenv();
	extern char *Cwd;

	char *start, save, dir[ PATHMAX ], *p;
	int rc;
	struct passwd *pwd;
	struct stat sbuf;

	start = to;
	*to = '\0';
	if(*from == '~')
	{
		for(p = dir, ++from; *from && !Psep(*from); ++from, ++p ) *p = *from;
		*p = '\0';
		if(*dir)
		{
			if((pwd = Getpwnam(dir)) == NULL) return 2;
			strcpy(to, pwd->pw_dir);
			free_pwent(pwd);
		}
		else
			strcpy(to, Me->pw_dir);
		to += strlen(to);

		if(*from && !Psep(*from) && !Psep(*(to - 1))) *to++ = PSEP;
	}
	else if(*from == '$')
	{
		for(p = dir, ++from; *from && !Psep(*from); ++from, ++p ) *p = *from;
		*p = '\0';
		if((p = getenv(dir)) == NULL) return 2;
		strcpy(to, p);
		to += strlen(to);

		if(*from && !Psep(*from) && !Psep(*(to - 1))) *to++ = PSEP;
	}
	else
	{
		if(Vars[VEXPAND].val && !Psep(*from))
		{	/* add the current directory */
			strcpy(to, Cwd);
			to += strlen(to);
			if( !Psep(*(to - 1)) ) *to++ = PSEP;
		}
	}

	if(Vars[VEXPAND].val)
	{	/* now handle the filename */
		for( ; *from; ++from )
			if( *from == '.' )
			{
				if( Psep(*(from + 1)) )
					++from;
				else if( *(from + 1) == '.' &&
						(Psep(*(from + 2)) || *(from + 2) == '\0') )
				{
					to -= 2;
					while( to > start && !Psep(*to) ) --to;
					++to;
					if( *(++from + 1) ) ++from;
				}
				else
					*to++ = *from;
			}
			else if( Psep(*from) )
			{	/* strip redundant seperators */
				if( to == start || !Psep(*(to - 1)) )
					*to++ = PSEP;
			}
			else
				*to++ = *from;
		*to = '\0';
	}
	else
	{
		strcpy(to, from);
	}

	/* validate the filename */
	if( stat(start, &sbuf) == EOF )
	{	/* file does not exit - validate the path */
		if((to = strrchr(start, PSEP)) != NULL)
		{
			save = *to;
			*to = '\0';
			rc = !Isdir( start );
			*to = save;
		}
		else rc = 0;
	}
	else if( sbuf.st_mode & S_IFDIR )
		rc = -1;
	else
		rc = 0;
#if DBG
	if(strlen(start) >= PATHMAX)
		Dbg("TOO LONG %d '%s'\n", strlen(start), start);
#endif
	return( rc );
}


Boolean Isdir( path )
char *path;
{
	struct stat sbuf;

	return( stat(path, &sbuf) == 0 && (sbuf.st_mode & S_IFDIR) );
}


Boolean Isfile( path, dir, fname, must )
char *path, *dir, *fname;
Boolean must;
{
	if(!dir || !fname) return FALSE;
	strcpy( path, dir );
	if( !Psep(*(path + strlen(path) - 1)) )
		strcat( path, "/" );
	strcat( path, fname );
	return( !must || access(path, 0) == 0 );
}

/* Same as getpwnam but handles partial matches. */
static struct passwd *Getpwnam(name)
char *name;
{
	struct passwd *pwd, *match = NULL;
	int len = strlen(name);

	setpwent();
	while((pwd = getpwent()))
		if(strncmp(pwd->pw_name, name, len) == 0)
		{
			if(strcmp(pwd->pw_name, name) == 0)
			{	/* full match */
				endpwent();
				return dup_pwent(pwd);
			}
			else if(!match)
				/* partial match - return first match */
				match = dup_pwent(pwd);
		}
	endpwent();
	return match;
}
