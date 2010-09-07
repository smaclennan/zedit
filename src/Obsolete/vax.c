/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

#if VAX

Boolean Mv( from, to )
char *from, *to;
{
	extern int Cmask;
	Boolean ok = TRUE;
	char buff[ PSIZE + 1 ];
	int fd1, fd2, n;

	if( (fd1 = open(from, READ_MODE)) != EOF && 
		(fd2 = open(to, WRITE_MODE, Cmask)) != EOF )
	{
		while( ok && (n = read(fd1, buff, PSIZE)) > 0 )
			ok = write(fd2, buff, n) == n;
		(void)close( fd1 );
		(void)close( fd2 );
		if( ok ) unlink( from );
		return( ok );
	}
	else if( fd1 != EOF )
		(void)close( fd1 );
	return( FALSE );
}

#include <stsdef.h>
#include <ssdef.h>
#include <descrip.h>
#include <iodef.h>
#include <ttdef.h>
#include <tt2def.h>

#define NIBUF	128						/* Input buffer size */
#define NOBUF	1024					/* MM says big buffers win! */
#define EFN		0                       /* Event flag */

char	obuf[ NOBUF+1 ];				/* Output buffer */
int		nobuf;							/* # of bytes in above */
char	ibuf[ NIBUF+1 ];				/* Input buffer */
int		nibuf;							/* # of bytes in above */
int		ibufi;							/* Read index */
int		oldmode[ 4 ];					/* Old TTY mode bits */
int		newmode[ 4 ];					/* New TTY mode bits */
short	iochan;							/* TTY I/O channel */


/*
	This function is called once to set up the terminal device stream.
	It translates TT until it finds the terminal, then assigns
	a channel to it and sets it raw.
*/
void Vaxopen()
{
	char oname[ 40 ];
	int iosb[ 2 ], status;
	struct dsc$descriptor idsc, odsc;

	odsc.dsc$a_pointer	= "TT";
	odsc.dsc$w_length	= strlen( odsc.dsc$a_pointer );
	odsc.dsc$b_dtype	= DSC$K_DTYPE_T;
	odsc.dsc$b_class	= DSC$K_CLASS_S;
	idsc.dsc$b_dtype	= DSC$K_DTYPE_T;
	idsc.dsc$b_class	= DSC$K_CLASS_S;
	do
	{
		idsc.dsc$a_pointer = odsc.dsc$a_pointer;
		idsc.dsc$w_length  = odsc.dsc$w_length;
		odsc.dsc$a_pointer = oname;
		odsc.dsc$w_length  = sizeof( oname );
		status = LIB$SYS_TRNLOG( &idsc, &odsc.dsc$w_length, &odsc );
		if( status != SS$_NORMAL && status != SS$_NOTRAN )
		{
			printf( "VAXERR: 01 (%d)\n", status );
			exit( status );
		}
		if( oname[ 0 ] == 0x1B )
		{
			odsc.dsc$a_pointer += 4;
			odsc.dsc$w_length  -= 4;
		}
	}
	while( status == SS$_NORMAL );
	if( (status = SYS$ASSIGN(&odsc, &iochan, 0, 0)) != SS$_NORMAL )
	{
		printf( "VAXERR: 02 (%d)\n", status );
		exit( status );
	}
	status = SYS$QIOW( EFN, iochan, IO$_SENSEMODE, iosb, 0, 0,
						oldmode, sizeof(oldmode), 0, 0, 0, 0 );
	if( status != SS$_NORMAL || (iosb[0] & 0xFFFF) != SS$_NORMAL )
	{
		printf( "VAXERR: 03 (%d)\n", status );
		exit( status );
	}
	newmode[ 0 ] = oldmode[ 0 ];
	newmode[ 1 ] = oldmode[ 1 ] | TT$M_NOECHO;
	newmode[ 1 ] &= ~(TT$M_TTSYNC | TT$M_HOSTSYNC);
	newmode[ 2 ] = oldmode[ 2 ] | TT2$M_PASTHRU;
	status = SYS$QIOW( EFN, iochan, IO$_SETMODE, iosb, 0, 0,
						newmode, sizeof(newmode), 0, 0, 0, 0 );
	if( status != SS$_NORMAL || (iosb[0] & 0xFFFF) != SS$_NORMAL )
	{
		printf( "VAXERR: 04 (%d)\n", status );
		exit( status );
	}
	Rowmax = ( newmode[1] >> 24 ) - 1;
	Colmax = newmode[ 0 ] >> 16;
}


/*
	This function gets called just before we go back home to the command
	interpreter. It puts the terminal back in a reasonable state.
	Return should be SS$NORMAL.
*/
int Vaxclose()
{
	int status, iosb[ 1 ];

	Vaxflush();
	status = SYS$QIOW( EFN, iochan, IO$_SETMODE, iosb, 0, 0,
						oldmode, sizeof(oldmode), 0, 0, 0, 0 );
	if( status == SS$_NORMAL && (iosb[0] & 0xFFFF) == SS$_NORMAL )
		status = SYS$DASSGN( iochan );
	return( status );
}


/*
	Read a character from the terminal, performing no editing.
	SAM - for now returns EOF on bad read
*/
int Tgetkb()
{
	int status, iosb[ 2 ], term[ 2 ];

	while( ibufi >= nibuf )
	{
		ibufi = 0;
		term[ 0 ] = term[ 1 ] = 0;
		status = SYS$QIOW( EFN, iochan, IO$_READLBLK | IO$M_TIMED,
							iosb, 0, 0, ibuf, NIBUF, 0, term, 0, 0 );
		if( status != SS$_NORMAL )
			return( EOF );				/* SAM */
		status = iosb[ 0 ] & 0xffff;
		if( status != SS$_NORMAL && status != SS$_TIMEOUT )
			return( EOF );				/* SAM */
		nibuf = (iosb[ 0 ] >> 16) + (iosb[ 1 ] >> 16);
		if( nibuf == 0 )
		{
			status = SYS$QIOW( EFN, iochan, IO$_READLBLK,
								iosb, 0, 0, ibuf, 1, 0, term, 0, 0 );
			if( status != SS$_NORMAL ||
				(status = (iosb[ 0 ] & 0xFFFF)) != SS$_NORMAL )
					return( EOF );		/* SAM */
			nibuf = (iosb[ 0 ] >> 16) + (iosb[ 1 ] >> 16);
		}
	}
	return( ibuf[ibufi++] & 0xff );		/* Allow multinational */
}


/*
	Write a character to the display. Terminal output is buffered, and
	we just put the characters in the big array, after checking for overflow.
*/
void Tputchar( c )
char c;
{
	if( nobuf >= NOBUF ) Vaxflush();
	obuf[ nobuf++ ] = c;
}


/* Flush terminal buffer. Does real work of terminal output. */
int Tflush()
{
	int status, iosb[ 2 ];

	status = SS$_NORMAL;
	if( nobuf )
	{
		status = SYS$QIOW( EFN, iochan, IO$_WRITELBLK | IO$M_NOFORMAT,
							iosb, 0, 0, obuf, nobuf, 0, 0, 0, 0 );
		if( status == SS$_NORMAL )
			status = iosb[ 0 ] & 0xFFFF;
		nobuf = 0;
	}
	return( status );
}

#endif	/* VAX */
