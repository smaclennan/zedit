/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

/*
 * This files contains the virtual buffering version of the low-level
 * buffer routines.
 *
 * This version is required for FASTREAD
 */
 
#ifdef VBUFF
#include <setjmp.h>

/* local to VBUFF only */
static PIM *Getmemp ARGS((void));

/* vbuff globals */
int  Swapf;						/* swap file fd */
int  Swapmax = SWAPMAX;			/* Swapmap size */
static char *Swapname;					/* swap file name */
static char *Swapmap;					/* array of bits */
static int  Pimcount;					/* actual count of Pages alloced */

PIM			*Pages;				/* the pool of pims */
static PIM	*Firstu, *Lastu;	/* the first and last free pims */

#if MSDOS
#include "emm.h"
#endif

/* Initialize the buffer stuff */
void Binit( numpims, swapmax )
int numpims, swapmax;
{
	extern char *getenv();
	PIM *tpim;
	char *ptr;
	int cnt;

#if MSDOS
	/* setup the swapfile or extended/expanded memory */
	if( cnt = EMMinit(Swapmax << 3) )
		Swapmax = cnt >> 3;
	else
#endif
	{	/* create the swapfile */
#if UNIX
		if( ((ptr = getenv("ZSWAPDIR")) || (ptr = Me->pw_dir)) &&
#else
		if( ((ptr = getenv("ZSWAPDIR")) || (ptr = getenv("HOME"))) &&
#endif
			(Swapname = malloc(strlen(ptr) + 15)) )
			sprintf( Swapname, "%s/%s", ptr, SWAPNAME );
		else
			Swapname = SWAPNAME;
		mktemp( Swapname );
		if( (Swapf = open(Swapname, UPDATE_MODE, 0600)) == EOF )
		{
			printf( "Unable to open swap file '%s'. (%d)\n", Swapname, errno );
			exit( 1 );
		}
#if UNIX
		unlink( Swapname );
#endif
	}

/* SAM
	Swapmax = swapmax << 7;				 * convert from M to (K / 8) */
	Swapmax = swapmax << 4;				/* convert from 100K to (K / 8) */
	if( Swapmax == 0 ) Swapmax = SWAPMAX;
	if( !(Swapmap = malloc(Swapmax)) )
	{
		puts( "Error: unable to allocate swap map" );
		Closeswap();
		exit( 1 );
	}
	memset( Swapmap, 0, Swapmax );

	/* allocate the PIMs - we must have at least 3 PIMs */
	numpims = MAX( numpims, MINPIMS );
	if( !(Pages = (PIM *)malloc(numpims * sizeof(PIM))) )
	{
		puts( "Error: unable to allocate PIMs" );
		Closeswap();
		exit( 1 );
	}
	for( Pimcount = 0, tpim = Pages; 
			 Pimcount < numpims && (tpim->pdata = (Byte *)malloc(PSIZE));
			 Pimcount++, tpim++ )
	{
		tpim->lfile = MEMORY;
		tpim->pim_modf = FALSE;
		tpim->nextu = tpim + 1;
		tpim->prevu = tpim - 1;
	}
	Firstu = Pages;
	Firstu->prevu = NULL;
	Lastu = tpim - 1;
	Lastu->nextu = NULL;
	XBinit();					/* init the cache */

	initScrnmarks();			/* init the screen marks and mark list */
}


Page *Bcrepage(tbuff, ppage, npage, cnt)
Buffer *tbuff;
Page *ppage, *npage;
int cnt;
{
	Page *new;

	if((new = (Page *)malloc(sizeof(Page))) != NULL)
	{
		new->nextp = npage;
		new->prevp = ppage;
		if(npage)
			npage->prevp = new;
		else
			tbuff->lastp = new;
		if(ppage)
			ppage->nextp = new;
		else
			tbuff->firstp = new;
		new->plen = 0;
		new->lines = EOF;						/* undefined */
		new->afile = SWAP;
		new->apage = cnt;
#ifdef FAST
		new->fd = new->fpage = EOF;			/* not Fastread */
#endif
	}
	return new;
}


/* Write out a modified PIM */
void Bflush()
{
	extern PIM *Lastu;
	register PIM *tpim;

	for(tpim = Lastu; tpim; tpim = tpim->prevu)
		if(tpim->pim_modf && tpim->lfile != MEMORY)
		{	/* found a modified PIM, write it out! */
			if(tpim->ldesc->plen)
#if MSDOS
				if(EMMtype)
					EMMwrite(tpim->lpage, tpim->pdata);
				else
#endif
				if(XBwrite(Swapf, tpim->lpage, tpim->pdata) == 0)
					Error("Swap Write Error");
#ifdef FAST
			if(tpim->lfile == INFILE) tpim->lfile = SWAP; 
#endif
			tpim->pim_modf = FALSE;
		}
}


void Freepage( tbuff, tpage )
Buffer *tbuff;
Page *tpage;
/* Free the page. */
{
	extern PIM *Pages;
	int tmp;
	PIM *tpim;

	if( tpage->nextp )
		tpage->nextp->prevp = tpage->prevp;
	else
		tbuff->lastp = tpage->prevp;
	if( tpage->prevp )
		tpage->prevp->nextp = tpage->nextp;
	else
		tbuff->firstp = tpage->nextp;
	if( tpage->afile == MEMORY )
	{
		tpim = &Pages[ tpage->apage ];
		/* this (and below) must be done via a tmp var due to a DY-4 bug */
		tmp = tpim->lpage >> 3;
		Swapmap[ tmp ] &= ~(1 << (tpim->lpage & 7));
		Pages[ tpage->apage ].lfile = MEMORY;
		Pages[ tpage->apage ].pim_modf = FALSE;
	}
	else
	{
		tmp = tpage->apage >> 3;
		Swapmap[ tmp ] &= ~(1 << (tpage->apage & 7));
	}
	free( (char *)tpage );
}


/* Find a free memory page. */
static PIM *Getmemp()
{
	extern jmp_buf zenv;
	extern PIM *Lastu;
	register PIM *tpim;
	register int cnt;

	/* look for an unmodified PIM - only check the first 1/3 of the chain */
	for( cnt = Pimcount / 3, tpim = Lastu;
		 cnt > 0 && tpim->pim_modf;
		 --cnt, tpim = tpim->prevu ) ;
				
	if( cnt <= 0 )
	{
		Bflush();
		for( tpim = Lastu; tpim && tpim->pim_modf; tpim = tpim->prevu ) ;
		if( !tpim ) longjmp( zenv, -1 );	/* ABORT */
	}

	if( tpim->lfile != MEMORY )
	{	/* reset the page back to what it was before Intomem */
		tpim->ldesc->afile = tpim->lfile;
		tpim->ldesc->apage = tpim->lpage;
	}

	return( tpim );
}


/* Put the 'page' in a PIM */
void Intomem( page )
Page *page;
{
	extern PIM *Pages;
	register PIM *tpim;
#ifdef FAST
	int len;
#endif

	if( page->afile == MEMORY ) return;
	tpim = Getmemp();
	tpim->lfile = page->afile;
	tpim->lpage = page->apage;
	tpim->pim_modf = FALSE;
	tpim->ldesc = page;
	if( page->plen )
#ifdef FAST
		if( page->afile == INFILE )
		{
			if( (len = XBread(page->fd, page->fpage, tpim->pdata)) == 0 )
				Error( "Bad Infile Read" );
			page->plen = len;
		 }
		else
#endif
#if MSDOS
		if( EMMtype )
			EMMread( tpim->lpage, tpim->pdata );
		else
#endif
		if( XBread(Swapf, tpim->lpage, tpim->pdata) == 0 )
			Error( "Bad Swap Read" );
	page->afile = MEMORY;
	page->apage = tpim - Pages;
}


/*
	Make a new page and link it into the chain.
	If there is not enough memory - free one of the PIMs and mark it as
	unusable and unlink it from the list.
	Returns NULL if out of swap space or out of memory.
*/
Page *Newpage( tbuff, ppage, npage )
Buffer *tbuff;
Page *ppage, *npage;
{
	extern PIM *Firstu, *Lastu;
	register int cnt;
	Page *tpage;
	PIM *tpim;

	for( cnt = 0; cnt < Swapmax && Swapmap[cnt] == -1; ++cnt) ;
	if( cnt == Swapmax )
	{
		Error( "Swap File Full" );
		return( NULL );
	}
	for( cnt <<= 3; Swapmap[cnt >> 3] & (1 << (cnt & 7)); ++cnt) ;
	if( !(tpage = Bcrepage(tbuff, ppage, npage, cnt)) )
		if( Pimcount > MINPIMS )
		{ /* out of memory, free a PIM and mark as LOCKED */
			tpim = Getmemp();
			free((char *)tpim->pdata);
			tpim->lfile = PIMLOCKED;
			/* link out of LRU chain */
			if( tpim->nextu )
				tpim->nextu->prevu = tpim->prevu;
			else
				Lastu = tpim->prevu;
			if( tpim->prevu )
				tpim->prevu->nextu = tpim->nextu;
			else
				Firstu = tpim->nextu;
			tpage = Bcrepage( tbuff, ppage, npage, cnt );
		}
		else
		{
			Error( "Out of Memory!" );
			return( NULL );
		}
		
	if( tpage ) Swapmap[ cnt >> 3 ] |= (1 << (cnt & 7));
	return( tpage );
}


/* Make tpage the current page */
void Makecur( tpage )
Page *tpage;
{
	extern Boolean Curmodf;
	PIM *tpim;

	if( Curpage == tpage ) return;
	if( Curpage )
	{
		Curpage->plen = Curplen;
		Pages[ Curpage->apage ].pim_modf = Curmodf ;
		if( Curmodf || Curpage->lines == EOF )
			Curpage->lines = Cntlines( Curplen );
	}
	Intomem( tpage );
	Curpage = tpage;
	tpim = &Pages[ Curpage->apage ];
	Cpstart = (Byte *)tpim->pdata;
	Curmodf = tpim->pim_modf;
	Curplen = Curpage->plen;
	if( tpim == Firstu ) return;
	if( !tpim->nextu ) 
		Lastu = tpim->prevu;
	else
		tpim->nextu->prevu = tpim->prevu;
	tpim->prevu->nextu = tpim->nextu;
	tpim->prevu = NULL;
	tpim->nextu = Firstu;
	Firstu->prevu = tpim;
	Firstu = tpim;
}


/* Split the current (full) page. */
Boolean Pagesplit()
{
	extern PIM *Pages;
	extern Boolean Curmodf;
	PIM *tpim;
	Page *tpage;
	register Mark *btmark;

	if( (tpage = Newpage(Curbuff, Curpage, Curpage->nextp)) == NULL )
		return( FALSE );

	Intomem( tpage );
	tpim = &Pages[ tpage->apage ];
	memmove( tpim->pdata, Cpstart + (PSIZE / 2), PSIZE / 2 );
	tpim->pim_modf = TRUE;

	Curmodf = TRUE;
	Curplen = PSIZE / 2;
	tpage->plen = PSIZE / 2;
	for( btmark = Mrklist; btmark; btmark = btmark->prev )
		if( btmark->mpage == Curpage && btmark->moffset >= (PSIZE / 2) )
		{
			btmark->mpage = tpage;
			btmark->moffset -= PSIZE / 2;
		}
	if( Curchar >= (PSIZE / 2) )
	{
		Makecur( tpage );
		Makeoffset( Curchar - (PSIZE / 2) );
	}
	return( TRUE );
}


Proc Closeswap()
{
#if MSDOS
	if( EMMtype )
		EMMfree();
	else
#endif
	{
		(void)close( Swapf );
#if !UNIX
		unlink( Swapname );
#endif
	}
}


Proc Zstat()
{
	extern char *Swapmap;
	extern int Pimcount, Swapmax, Numbuffs;
	extern PIM *Pages;

	char *mem[ 128 ];
	int i, size;				/* i MUST be an int! */
	unsigned f, c, cp, p;	/* MUST be unsigned */
	
#if MSDOS
	if(Argp)
	{
		extern Byte Vtype;
		char str[10];
		
		switch(EMMtype)
		{
			case USESWAP:	strcpy(str, "Virtual");		break;
			case EXTENDED:	strcpy(str, "Extended");	break;
			case EXPANDED:	strcpy(str, "Expanded");	break;
		}
		sprintf(PawStr, "%s Memory (%dk)\tVtype %02x", str, Swapmax<<3, Vtype);
		Echo(PawStr);
		return;
	}
#endif
	
#if NOALLOC
	/* Calc free memory - start at 256k chunks and go down */	
	for( f = 0, i = 0, size = 8; size > 0; --size )
		for( ; i < 128 && (mem[i] = malloc(PSIZE << size)); ++i )
			f += (1 << size);
	while( --i >= 0 ) free( mem[i] );
#endif

	/* Calc number of free Swap pages */
	for( i = c = 0; i < Swapmax; ++i )
		c += Bitcnt( Swapmap[i] & 0xff );
	cp = c * 25 / Swapmax >> 1;

	/* Calc number of PIMs - don't include unalloced ones */
	for( p = i = 0; i < Pimcount; ++i )
		p += Pages[i].lfile != PIMLOCKED;

#if NOALLOC
	sprintf(PawStr,
		"Buffers: %d   Buffer Used %uk (%u%%)   PIMs: %u",
		Numbuffs, c, cp, p);
#else
	sprintf( PawStr,
		"Free Memory: %uk   Buffers: %d   Buffer Used %uk (%u%%)   PIMs: %u",
		f, Numbuffs, c, cp, p );
#endif
	Echo( PawStr );
}


int Bitcnt( n )
int n;
{
	int b;
	
	for( b = n != 0; n = n & (n - 1); ++b ) ;
	return( b );
}

Byte Markch(mrk)
Mark *mrk;
{
	Byte ch;
	
	Bswappnt(mrk);
	ch = Buff();
	Bswappnt(mrk);
	return ch;
}
#endif
