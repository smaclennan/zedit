/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

#if MSDOS && defined(VBUFF)
#include <dos.h>
#include "emm.h"

#ifdef __TURBOC__
/* use the Microsoft versions */
#undef FP_SEG
#undef FP_OFF
#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))
#endif

/* LARGE memory model only! */

int EMMtype = USESWAP;
static int EMMhandle;
static char far *EMMpage[ 4 ];
static unsigned EMMoffs[ 16 ] =
{ 
	   0, 1024,  2048,  3072,  4096,  5120,  6144,  7168,
	8192, 9216, 10240, 11264, 12288, 13312, 14336, 15360
};

static int emm_ppage ARGS((int lpage));
static int emm_map ARGS((int epage, int ppage));
static void exmove ARGS((int lpage, Byte *data, Boolean from));
static void toaddr24 ARGS((Byte *addr24, unsigned long addr));

/* Block Move Descriptor Table */
static struct
{
	Byte dummy1[ 16 ];
	Short slen;
	Byte saddr[ 3 ];
	Byte saccess;
	Byte dummy2[ 2 ];
	Short dlen;
	Byte daddr[ 3 ];
	Byte daccess;
	Byte dummy3[ 18 ];
} bmdt;


/* Returns 0 if EMM unavailable, else k allocated. */
int EMMinit( int min )
{
	char *name;
	int page;
	union REGS regs;
	struct SREGS sregs;

	/* check if EMM driver exists */
	regs.x.ax = 0x3567;
	segread( &sregs );
	intdosx( &regs, &regs, &sregs );
	FP_SEG( name ) = sregs.es;
	FP_OFF( name ) = 10;
	if( strncmp(name, "EMMXXXX0", 8) == 0 )
	{	/* EMM driver exists - check status of hardware/software */
		regs.h.ah = 0x40;
 		int86( 0x67, &regs, &regs );
		if( regs.h.ah == 0 )
		{	/* get the page frame */
			regs.h.ah = 0x41;
			int86( 0x67, &regs, &regs );
			if( regs.h.ah == 0 )
			{	/* everythings cool - set 'EMMpage's and get free pages */
				for( page = 0; page < 4; ++page )
				{
					FP_SEG( EMMpage[page] ) = regs.x.bx;
					FP_OFF( EMMpage[page] ) = 0x4000 * page;
				}
				regs.h.ah = 0x42;
				int86( 0x67, &regs, &regs );
				if( regs.h.ah == 0 && (regs.x.bx << 4) >= min )
				{	/* alloc the pages */
					page = regs.x.bx;
					regs.h.ah = 0x43;
					regs.x.bx = page;
					int86( 0x67, &regs, &regs );
					EMMhandle = regs.x.dx;
					EMMtype = EXPANDED;
					return( regs.h.ah ? 0 : page << 4 );
				}
			}
		}
	}

	/* check for extended memory */
	if( Vars[VEXTEND].val )
	{
		regs.h.ah = 0x88;
		int86( 0x15, &regs, &regs );
		if( regs.x.ax >= min )
		{
			EMMtype = EXTENDED;
			return regs.x.ax;
		}
	}
	return 0;
}


/* free the handle and associated pages */
void EMMfree( void )
{
	union REGS regs;

	if( EMMtype == EXPANDED )
	{
		regs.h.ah = 0x45;
		regs.x.dx = EMMhandle;
		int86( 0x67, &regs, &regs );
	}
}


void EMMread( int lpage, Byte *data )
{
	int ppage;
	
	if( EMMtype == EXPANDED )
	{
		ppage = emm_ppage( lpage );
		memcpy( data, EMMpage[ppage] + EMMoffs[lpage % 16], PSIZE );
	}
	else
		exmove( lpage, data, 1 );
}


void EMMwrite( int lpage, Byte *data )
{
	int ppage;
	
	if( EMMtype == EXPANDED )
	{
		ppage = emm_ppage( lpage );
		memcpy( EMMpage[ppage] + EMMoffs[lpage % 16], data, PSIZE );
	}
	else
		exmove( lpage, data, 0 );
}


/*
Returns the physical page number to use. Maps in if necessary.
	1. First checks if 'lpage' already assigned a 'ppage'
	2. If no 'ppage', tries to assign an unused 'ppage'
	3. If all used, assigned LRU 'ppage'
*/
static int emm_ppage( int lpage )
{
	static unsigned age = 0;
	static unsigned usage[] = { 0, 0, 0, 0 };
	static int pageuse[ 4 ] = { -1, -1, -1, -1 };

	int i, ppage = -1;
	unsigned oldest = -1;
	
	++age;				/* update the age counter */
	lpage >>= 4;		/* convert from lpage to epage (1k to 16k block) */
	for( i = 0; i < 4; ++i )
		if( pageuse[i] == lpage )
		{
			usage[ i ] = age;
			return i;
		}
		else if( pageuse[i] == -1 )
			ppage = i;

	if( ppage == -1 )
		/* LRU */
		for( i = 0; i < 4; ++i )
			if( usage[i] < oldest )
			{
				oldest = usage[ i ];
				ppage = i;
			}

	pageuse[ ppage ] = lpage;
	usage[ ppage ] = age;
	emm_map( lpage, ppage );
	return ppage;
}


/* map the logical page 'lpage' to the physical page */
static int emm_map( int epage, int ppage )
{
	union REGS regs;

	regs.h.ah = 0x44;
	regs.h.al = ppage;		/* physical page */
	regs.x.bx = epage;		/* extended memory page */
	regs.x.dx = EMMhandle;
	int86( 0x67, &regs, &regs );
	return regs.h.ah;
}


/* this routine moves from/to extended memory */
static void exmove( int lpage, Byte *data, Boolean from )
{
	Byte *p1;
	unsigned long addr;
	union REGS regs;
	struct SREGS sregs;

	/* set the bmdt defaults */
	memset( &bmdt, 0, sizeof(bmdt) );
	bmdt.slen = bmdt.dlen = PSIZE;
	bmdt.saccess = bmdt.daccess = 0x93;
	
	/* convert the lpage to 24-bit address */
	addr = 0x100000 + ((unsigned long)lpage << 10);
	toaddr24( from ? bmdt.saddr : bmdt.daddr, addr );

	/* convert the data address to 24-bit address */
	addr = ((unsigned long)FP_SEG( data ) << 4) + FP_OFF( data );
 	toaddr24( from ? bmdt.daddr : bmdt.saddr, addr );

	/* do the move */
	regs.h.ah = 0x87;
	regs.x.cx = PSIZE >> 1;		/* words */
	p1 = (Byte *)&bmdt;
	regs.x.si = FP_OFF( p1 );
	sregs.es  = FP_SEG( p1 );
	int86x( 0x15, &regs, &regs, &sregs );
}


static void toaddr24( Byte *addr24, unsigned long addr )
{
	int i;
	
	for( i = 0; i < 3; ++i, addr >>= 8 )
		*addr24++ = addr;
}
#endif
