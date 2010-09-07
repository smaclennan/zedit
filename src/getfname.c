/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"
#if SYSV4
#include <dirent.h>
#else
#include <sys/dir.h>
#endif

LLIST *Flist = NULL;
Boolean Didmatch = FALSE;


static int getname ARGS((char*, char*, Boolean));

int Getfname(prompt, path)
char *prompt, *path;
{
	int rc;

	if((rc = getname(prompt, path, FALSE)) > 0)
		Error("Invalid path.");
	return rc;
}


int Getdname(prompt, path)
char *prompt, *path;
{
	int rc;

	if((rc = getname(prompt, path, TRUE)) > 0)
		Error("Invalid dir.");
	return rc;
}


/* Returns:
 *		0 for ok
 *		ABORT(-1) for abort
 *		> 0 for error
 */
static int getname(prompt, path, isdir)
char *prompt, *path;
Boolean isdir;
{
	extern char *strcpy();
	extern Byte Keys[];
	char tmp[ PATHMAX + 1 ];
	int tab, space, metameta, rc;

	metameta = Keys[ 128 + 27 ];	/* for people who use csh/ksh */
	Keys[ 128 + 27 ] = ZFNAME;
	tab = Keys[ '\t' ];
	Keys[ '\t'  ] = ZFNAME;
	space = Keys[ ' ' ];
	Keys[ ' ' ]   = ZMATCH;
	if( (rc = Getarg(prompt, strcpy(tmp, path), PATHMAX)) == 0 )
		if((rc = Pathfixup(path, tmp)) == -1)
			rc = isdir ? 0 : 1;
	Keys[ ' '  ] = space;
	Keys[ '\t' ] = tab;
	Keys[ 128 + 27 ] = metameta;
	Freelist( &Flist );
	if( Didmatch )
	{
		Zredisplay();
		Didmatch = FALSE;
	}
	return( rc );
}


Proc Zfname()
{
	extern int Pawlen;
	Boolean update;
	LLIST *list;
	char txt[ PATHMAX + 1 ], *fname, *match = NULL;
	int len, n = 0, f = 0, rc;

	if( !(list = GetFill( txt, &fname, &len, &update)) )
	{
		Tbell();
		return;
	}
	if( *fname )
		for(; list && (rc=strncmp(fname,list->fname,len))>=0; list=list->next)
			if( rc == 0 )
			{
				if( match )
					n = f = nmatch( match, list->fname );
				else
					n = strlen( match = list->fname );
			}
	if( match )
	{
		if( n > len )
		{
			Btoend();
			while( len < n && Curplen < Pawlen )		/* SAM */
				Binsert( match[len++] );
			if(len < n) Tbell();
		}
		if( f == 0 && Isdir(Getbtxt(txt, PATHMAX)) && Curplen < Pawlen)	/*SAM*/
			Binsert( PSEP );
	}
	else if( !update )
		Tbell();
}


Proc Zmatch()
{
	LLIST *list;
	char dir[PATHMAX + 1], *fname, *p;
	int row, col, len;

	if((list = GetFill(dir, &fname, &len, &col)) == NULL)
	{
		Tbell();
		return;
	}
	p = dir + strlen(dir);
	*p++ = PSEP;
	Didmatch = TRUE;
	Tgoto(Tstart, 0);
	Tprntstr("Choose one of:");
	Tcleol();
	row = Tstart + 1; col = 0;
	for( ; list; list = list->next)
		if(len == 0 || strncmp(fname, list->fname, len) == 0)
		{
			Tgoto(row, col);
			strcpy(p, list->fname);
			if(strlen(list->fname) > 23) list->fname[23] = '\0';
			Tprntstr(list->fname);
			if(Isdir(dir)) Tputchar(PSEP);
			Tcleol();
			if((col += 25) > 72)
			{
				if(++row < Rowmax - 2)
					col = 0;
				else
					break;
			}
		}
	if(col) row++;
	while(row < Rowmax - 2)
	{
		Tgoto(row++, 0);
		Tcleol();
	}
}


LLIST *GetFill( dir, fname, len, update )
char *dir, **fname;
int *len;
Boolean *update;
{
	extern Boolean First;
	char txt[ PATHMAX + 1 ];

	if( First )
	{
		Bdelete( Curplen );
		First = FALSE;
	}
	Getbtxt( txt, PATHMAX );
	if( Pathfixup(dir, txt) > 0 ) return( NULL );
	if((*update = strcmp(dir, txt)))
		Makepaw( dir, FALSE );
	*fname = Lastpart( dir );
	*len = strlen( *fname );

	/* If ExpandPaths not set, may be no directory specified! */
	if(*fname == dir)
		return Fill_list("./");

	if( *fname - dir == 1 )
	{	/* special case for root dir */
		strcpy( txt, *fname );
		strcpy( ++*fname, txt );
	}
	*(*fname - 1) = '\0';
	return( Fill_list(dir) );
}


#define OBJEXT		".o"

LLIST *Fill_list( dir )
char *dir;
{
	static char savedir[ PATHMAX + 1 ];
	DIR *dp;
#if SYSV4
	struct dirent *dirp;
#else
	struct direct *dirp;
#endif

	if( Flist && strcmp(dir, savedir) == 0 )
		return( Flist );
	strcpy( savedir, dir );
	Freelist( &Flist );

	if((dp = opendir(dir)) == NULL)
		return Flist;

	while((dirp = readdir(dp)))
	{
#if ULTRIX
		char *fname = dirp->gd_name;
#else
		char *fname = dirp->d_name;
#endif
		if(!Isext(fname, OBJEXT))
			Add( &Flist, fname );
	}

	closedir(dp);

	return( Flist );
}


int nmatch( s1, s2 )
char *s1, *s2;
{
	int i;

	for( i = 0; Tolower(*s1) == Tolower(*s2); ++i, ++s1, ++s2 ) ;
	return( i );
}


LLIST *Add( list, fname )
LLIST **list;
char *fname;
{
	LLIST *new, *l;

	if((new = (LLIST *)malloc(sizeof(LLIST))))
	{
		strcpy( new->fname, fname );
		if( *list == NULL || strcmp(fname, (*list)->fname) < 0 )
		{
			new->next = *list;
			if( *list ) (*list)->prev = new;
			new->prev = NULL;
			*list = new;
		}
		else
		{
			for( l = *list; l->next && strcmp(l->next->fname, fname) < 0;
				 l = l->next ) ;
			if( l->next ) l->next->prev = new;
			new->next = l->next;
			l->next = new;
			new->prev = l;
		}
	}
	return( new );
}


void Freelist( list )
LLIST **list;
{
	LLIST *next;

	while( *list )
	{
		next = (*list)->next;
		free((char *)(*list));
		*list = next;
	}
}
