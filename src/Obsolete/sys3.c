/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

#if SYS3
/*
These are the routines that are defined in SYSV but not SYS3, at least not
on the DY-4 machine.
*/


void memcpy( dest, src, num )
char *dest, *src;
int num;
{
	while( num-- > 0 ) *dest++ = *src++;
}


void memset( dest, ch, num )
char *dest, ch;
int num;
{
	while( num-- > 0 ) *dest++ = ch;
}


Byte *memchr( src, ch, num )
Byte *src, ch;
int num;
{
	for( ; num-- > 0; ++src )
		if( *src == ch )
			return( src );
	return( NULL );
}


char *getcwd( cwd, len )
char *cwd;
int len;
{
	extern FILE *popen();
	extern char *malloc();
	FILE *fp;
	char *Cwd;

	if( (ptr = Cwd) || (ptr = malloc(len + 2)) )	
		if( fp = popen("pwd", "r") )
		{
			Readstr( ptr, fp );
			pclose( fp );
			return( ptr );
		}
	return( NULL );
}


/*
	convert [ws][-][0][x][digits] to number of base 'base'
		- base must be 16 or less
		- if base is 0
			- if the number starts with 0x, assume hex
			- if the number starts with 0, assume octal
			- else assume decimal
*/
long strtol( str, end, base )
char *str, **end;
int base;
{
	extern char *strchr();
	static char valid[] = "0123456789abcdef";
	Boolean neg;
	char *ptr;
	long num = 0, t;
	
	while( isspace(*str) ) ++str;
	if( neg = *str == '-' ) ++str;
	if( base == 0 )
		if( *str == '0' )
		{
			++str;
			if( *str == 'x' || *str == 'X' )
			{
				++str;
				base = 16;
			}
			else
				base = 8;
		}
		else
			base = 10;
	for( ; (ptr = strchr(valid, Tolower(*str))) &&
		   (t = ptr - valid) < base;
		   num = num * base + t, ++str );
	if( end ) *end = str;
	return( neg ? -num : num );
}


void dup2( fd1, fd2 )
int fd1, fd2;
/* force fd2 to point to fd1 */
{
	(void)close( fd1 );
	dup( fd2 );
}


#ifdef NO_LONGER_USED
/* old ttychk using ioctl */
int ttychk()
{
	long int cnt;

	if( Pending ) return( TRUE );
	return( Pending = (ioctl(0, FIONREAD, &cnt) < 0) ? 0 : cnt );
}
#endif


/*
**	@(#)getopt.c	2.2 (smail) 1/26/87
*/

/*
 * Here's something you've all been waiting for:  the AT&T public domain
 * source for getopt(3).  It is the code which was given out at the 1985
 * UNIFORUM conference in Dallas.  I obtained it by electronic mail
 * directly from AT&T.  The people there assure me that it is indeed
 * in the public domain.
 * 
 * There is no manual page.  That is because the one they gave out at
 * UNIFORUM was slightly different from the current System V Release 2
 * manual page.  The difference apparently involved a note about the
 * famous rules 5 and 6, recommending using white space between an option
 * and its first argument, and not grouping options that have arguments.
 * Getopt itself is currently lenient about both of these things White
 * space is allowed, but not mandatory, and the last option in a group can
 * have an argument.  That particular version of the man page evidently
 * has no official existence, and my source at AT&T did not send a copy.
 * The current SVR2 man page reflects the actual behavor of this getopt.
 * However, I am not about to post a copy of anything licensed by AT&T.
 */


/*LINTLIBRARY*/
#ifndef NULL
#define NULL	0
#endif
#define EOF	(-1)
#define ERR(s, c)	if(opterr){\
	char errbuf[2];\
	errbuf[0] = c; errbuf[1] = '\n';\
	(void) write(2, argv[0], (unsigned)strlen(argv[0]));\
	(void) write(2, s, (unsigned)strlen(s));\
	(void) write(2, errbuf, 2);}

#ifndef __STDC__	/* for __STDC__ pick up proto from <std.h> */
extern int write();
#endif

int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;

int
getopt(argc, argv, opts)
int	argc;
char	**argv, *opts;
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1)
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == 0) {
			optind++;
			return(EOF);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=strchr(opts, c)) == NULL) {
		ERR(": illegal option -- ", c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			ERR(": option requires an argument -- ", c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}
#endif
