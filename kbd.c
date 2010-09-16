/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "keys.h"

/*
	This file contains the keyboard input routines.
	The only routines accessed outside of this routine are Tgetcmd and
	the macro Pushcmd.

	Special code can be conditionally compiled in for the VT100.
*/

unsigned Cmdpushed = 0, Cmdstack[ 10 ];	/* stack and vars for T[un]getcmd */
unsigned Key_mask = 0;
char *Term;

#if !XWINDOWS
int Tgetcmd()
{
	int i, j, mask;
	int cmd;

	if( Cmdpushed ) return( Popcmd() );
	if( Mstate == INMACRO ) return( *Mptr++ );
	do
	{ /* try to match one of the termcap key entries */
		mask = Key_mask;
		for(j = 0; mask; ++j)
		{
			cmd = Tgetkb() & 0x7f;
			for( i = 0; i < NUMKEYS - SPECIAL_START; ++i )
				if( (mask & (1 << i)) && cmd == Tkeys[i].key[j] )
				{
					if( Tkeys[i].key[j + 1] == '\0' )
					{
						cmd = i + SPECIAL_START;
						goto found;
					}
				}
				else
					mask &= ~(1 << i);
		}

		// No match - push back the chars and try to handle
		// the first one.
		while( j-- > 0 ) Tungetkb();

		if((cmd = Tgetkb() & 0x7f) > NUMKEYS)
		{ // Ignore the key
			cmd = K_NODEF;
			Tbell();
		}
	}
	while( cmd == K_NODEF );

 found:
	if( Mstate == MCREATE )
		Addtomacro( cmd );

	return( cmd );
}
#endif


#if !XWINDOWS
/* stack and vars for T[un]getkb / Tkbrdy */
#define CSTACK		20
static Byte cstack[CSTACK];
static int cptr = -1;
int cpushed = 0;	/* needed in z.c */
static int Pending = FALSE;


Byte Tgetkb()
{
	cptr = (cptr + 1) % CSTACK;
	if(cpushed)
		--cpushed;
	else
	{
		Byte buff[CSTACK >> 1];
		int i, p;

		if((cpushed = read(0, (char *)buff, CSTACK >> 1) - 1) < 0)
			Hangup(1);	/* we lost connection */
		for(i = 0, p = cptr; i <= cpushed; ++i, p = (p + 1) % CSTACK)
			cstack[p] = buff[i];
	}
	Pending = FALSE;
	return(cstack[cptr]);
}


void Tungetkb()
{
	if( --cptr < 0 ) cptr = CSTACK - 1;
	++cpushed;
}


int Tkbrdy()
{
#if LINUX
	static struct pollfd stdin_fd = {
		.fd = 1,
		.events = POLLIN
	};

	if (Mstate == INMACRO || cpushed || Pending)
		return TRUE;

	Pending = poll(&stdin_fd, 1, 0);
	return Pending;
#else
	static struct timeval poll = {0,0};
	int fds = 1;

	return(Mstate == INMACRO || cpushed ||
		(Pending ? Pending :
			(Pending = select(1, (fd_set *)&fds, NULL, NULL, &poll))));
#endif
}
#endif /* !XWINDOWS */
