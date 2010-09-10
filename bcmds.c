/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/

#include "z.h"
#include <sys/stat.h>

extern char Lbufname[];

Proc Zswitchto()
{
	extern char **Bnames;
	extern int Numbuffs;
	extern unsigned Nextpart;
	
	char *was;
	int rc;

	Arg = 0;
#if XWINDOWS
	/* If an arg specified, try to start Zblist */
	if(Argp && StartProg("Zblist") == 0) return;
#endif
	was = Curbuff->bname;
	Nextpart = ZSWITCHTO;
	rc = Getplete("Buffer: ", Lbufname, Bnames, sizeof(char *), Numbuffs);
	Nextpart = ZNOTIMPL;
	if(rc == -1) return;
	strcpy(Lbufname, was);
	Loadwdo(Bnames[rc]);
	Cswitchto(Cfindbuff(Bnames[rc]));
}

Proc Znextbuff()
{
	Buffer *next;
	
	if((next = Curbuff->prev) == 0 && Curbuff->next)
		for(next = Curbuff->next; next->next; next = next->next) ;
	if(next)
	{
		strcpy( Lbufname, Curbuff->bname );
		Cswitchto(next);
		Reframe();
	}
	else Tbell();
}
	
Proc Zkillbuff()
{
	Buffer *tbuff;
	char bname[ BUFNAMMAX + 1 ];

	if( Argp )
	{
		strcpy( bname, Lbufname );
		do
			if( Getarg("Buffer: ", bname, BUFNAMMAX) ) return;
		while((tbuff = Cfindbuff(bname)) == NULL);
		Bswitchto( tbuff );
	}
	if( Curbuff->bmodf )
	  switch(Ask("Save Changes? "))
	  {
	    case ABORT: return;
	    case YES:   Zfilesave(); break;
	  }
	Delbuff(Curbuff);
}


Proc Delbuff(buff)
Buffer *buff;
{
	char bname[ BUFNAMMAX + 1 ];
	int wascur;
	Buffer *tbuff;
	WDO *wdo;

	wascur = buff == Curbuff;
	strcpy( bname, buff->bname );	/* save it for Delbname */
	if( strcmp(Lbufname, bname) == 0 ) *Lbufname = '\0';
	if( Bdelbuff(buff) )
	{
#if XWINDOWS
		XDeleteBuffer(bname);
#endif
		Delbname( bname );
		if(wascur && *Lbufname && (tbuff = Cfindbuff(Lbufname)) != NULL)
			Bswitchto( tbuff );
		Cswitchto(Curbuff);

#ifdef BORDER3D
		/* if window pointed to deleted buff then update it */
		wdo = Curwdo;
#else
		/* make sure all windows pointed to deleted buff are updated */
		for(wdo = Whead; wdo; wdo = wdo->next)
#endif
			if(wdo->wbuff == buff)
			{
				wdo->wbuff = Curbuff;
				Bmrktopnt(wdo->wpnt);
				Mrktomrk(wdo->wmrk, Curbuff->mark);
				Bmrktopnt(wdo->wstart);
				wdo->modeflags = INVALID;
			}
	}
}


#define WASTED		(BUFNAMMAX + 14)

Proc Zlstbuff()
{
#ifdef BORDER3D
	/* SAM FIX FOR BORDER3D */
	StartProg("Zblist");
#else
	extern char **Bnames;
	extern int Numbuffs;

	WDO *was = Curwdo;
	int i;
	Buffer *tbuff;

	if(WuseOther(LISTBUFF))
	{
		for(i = 0; i < Numbuffs; ++i)
		{
			if((tbuff = Cfindbuff(Bnames[i])) != NULL)
			{
				sprintf(PawStr, "%-*s %c%c %8lu %s ", BUFNAMMAX, tbuff->bname,
						(tbuff->bmode & SYSBUFF) ? 'S' : ' ',
						tbuff->bmodf ? '*' : ' ',
						Blength(tbuff),
						tbuff->fname ? Limit(tbuff->fname,WASTED) : UNTITLED);
				Binstr(PawStr);
				Binsert('\n');
			}
			else
			{
				sprintf(PawStr, "%-*s Problem\n", BUFNAMMAX, Bnames[i]);
				Binstr(PawStr);
			}
		}
		Curbuff->bmodf = FALSE;
		Wswitchto(was);
	}
	else Tbell();
	Arg = 0;
#endif	
}


Proc Zunmodf()
{
	Curbuff->bmodf = FALSE;
}
