/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"
#include "keys.h"

#define VERSION		"Zedit Macro Compiler version 1.0"

#if MSDOS
#define METAX	128 + 0x2d
#else
#define METAX	128 + 'X'
#endif


/* Dummy 'Cmds' is needed for the bind command to compile */
Proc (*Cmds[NUMFUNCS])();

/* this contains the Cnames array definition */
#include "cnames.h"

Boolean Must;		/* TRUE if must use M-X commands (-x option) */

void main( argc, argv )
int argc;
char *argv[];
{
	extern Boolean MParse();
	FILE *in, *out;
	char *outfile;
	Short buff[ MSIZE + 128 ], k, num = 0, rc = MACROVERSION;
	
	if( Must = (argc > 1 && strcmp(argv[1], "-x") == 0) )
		--argc;
	if( argc < 2 )
	{
		puts( "usage: zc [-x] infile [outfile]" );
		exit( 1 );
	}
	
	puts(VERSION);

	outfile = argc == 2 ? "z.mac" : argv[2 + Must];
	if( !(in = fopen(argv[1 + Must], "r")) || !(out = fopen(outfile, "w")) )
	{
		printf( "Unable to open %s\n", in ? outfile : argv[1 + Must] );
		exit( 1 );
	}

	Bind();

	while( MParse(buff, &k, in) && num++ < NUMMACROS )
	{
		/* if first time, write out macro version */
		if( num == 1 )
			fwrite( (char *)&rc, sizeof(Short), 1, out );
		/* write out macro name */
		fprintf( out, "M%02d\n", num );
		/* key definition */
		fwrite( (char *)&k, sizeof(Short), 1, out );
		/* write out the macro proper */
		fwrite( (char *)buff, MSIZE, 1, out );
	}
	
	if( num == NUMMACROS ) puts( "Too many macros in file" );

	fclose( in );
	fclose( out );
}


/*
Parse the input macro file of the form:
	>><str>						;marks the start of a macro
	:<key>						;key def
	<cmd> ['<arg>[']]			;always appends a CR to arg
	'<str>[']					;appends CR if ending ' missing
	#<str>						;ignores line
*/
Boolean MParse( macro, k, fp )
Short *macro;
int *k;
FILE *fp;
{
	extern Boolean Doarg(), Cmd();
	Boolean found = FALSE;
	char buff[ 256 ], *p;
	Short *mend;
	
	*k = NUMKEYS;
	memset( macro, '.', MSIZE );
	mend = macro + (MSIZE >> 1) - 2;	/* leave room for End Macro */
	while( fgets(p = buff, 256, fp) )
	{
		if( *buff == '>' && buff[1] == '>' )
		{
			if( found ) break;
		}
		else if( *buff == ':' )
		{
			if( (*k = Key(p + 1)) >= NUMKEYS )
				printf( "Bad key def: %s\n", p + 1 );
		}
		else if( *buff != '#' )
		{
			while( isspace(*p) ) ++p;
			if( *p == '\'' || *p == '"' )
			{
				if( !Doarg(p, &macro, mend, FALSE) )
				{
					puts( "Macro too long" );
					return( FALSE );
				}
			}
			else if( !Cmd(p, &macro, mend) )
			{
				puts( "Macro too long" );
				return( FALSE );
			}
			found = TRUE;
		}
	}
	*macro++ = 0x18;	/* C-X KLUDGE for End Macro! */
	*macro   = '0';
	return( found );
}


/* 
Handle an argument. Returns FALSE if macro too long.
If 'cr' is true, a CR is appended.
May go 1 past mend.
*/
Boolean Doarg( s, macro, mend, cr )
char *s;
Short **macro, *mend;
Boolean cr;		/* TRUE if MUST add cr */
{
	if( *s++ == '"' )
	{
		MetaX( s, macro );
		while( *s && *s != '\n' && *s != '"' ) ++s;
		if( *s ) ++s;
	}
	else
	{
		for( ; *s && *s != '\'' && *macro < mend; ++s )
			if( *s == '\n' )
				*(*macro)++ = (Short)'\r';
			else
				*(*macro)++ = (Short)(*s);
		if( *s == '\'' && cr )
			*(*macro)++ = (Short)'\r';
	}
	while( isspace(*++s) ) ;
	if( *s == '\'' || *s == '"' )
		Doarg( s, macro, mend, cr );
	return( *macro < mend );
}


/* Handle a possible command and argument. Returns FALSE if macro too long. */
Boolean Cmd( s, macro, mend )
char *s;
Short **macro, *mend;
{
	extern char Keys[];
	char *e, *l;
	int i, j, k, len;
	
	/* Find the end of the cmd name (e) and the last non-whitespace (l) */
	for( e = l = s; ; ++e )
		if( isalnum(*e) )
			l = e + 1;
		else if( !isspace(*e) )
			break;

	/* if there is a cmd name - look for a match in 'cnames' */
	if( len = l - s )
	{
		for( i = 0; i < NUMFUNCS; ++i )
			if( Strnicmp(s, Cnames[i].name, len) == 0 )
			{
				/* Found a match in 'cnames' - check for uniqueness */
				if( i < NUMFUNCS-1 && !Strnicmp(s, Cnames[i+1].name, len) )
				{
					printf( "Command '%.*s' not unique\n", len, s );
					return( TRUE );
				}
				k = Cnames[ i ].fnum;
				if( !Must )
				{
					/* KLUDGE add the M-X */
					*(*macro)++ = METAX;
					MetaX( Cnames[i].name, macro );
				}
				else
				{
					for( j = 0; j < NUMKEYS && Keys[j] != k; ++j ) ;
					if( Keys[j] == k )
					{
						if( j > 255 && j < 384 )
						{	/* KLUDGE C-X command */
							*(*macro)++ = 0x18;	/* C-X */
							j -= 256;
						}
						*(*macro)++ = j;
					}
					else
					{
						/* KLUDGE add the M-X */
						*(*macro)++ = METAX;
						MetaX( Cnames[i].name, macro );
					}
				}
				if( *e == '\'' || *e == '"' )
					Doarg( e, macro, mend, k != ZARG );
				return( *macro < mend );
			}
		printf( "Command '%.*s' not found\n", len, s );
	}
	return( TRUE );
}


/* Find minimum unique for M-X command */
MetaX( s, macro )
char *s;
Short **macro;
{
	extern char *strchr();
	char cmd[ 100 ], *s1, *mstr;
	int len = 0, len1, i, rc, start = 0;

	memset( cmd, 0, 100 );
	while( *s )
	{
		*(*macro)++ = *s;
		cmd[ len++ ] = *s++;

		/* look for a match */
		for( i = start;
			 i < NUMFUNCS && (rc = Strnicmp(Cnames[i].name, cmd, len)) < 0;
		 	 ++i) ;

		if( rc == 0 )
		{	/* found one - check for uniqueness */
			start = i;
			mstr = Cnames[i].name;
			if( ++i < NUMFUNCS && Strnicmp(cmd, Cnames[i].name, len) == 0 )
			{
				/* not unique - check for a partial match */
				if( s1 = strchr(mstr + len, ' ') )
				{
					len1 = s1 - mstr + 1;
					while( i < NUMFUNCS &&
						   Strnicmp(cmd, Cnames[i].name, len) == 0 &&
						   (rc = strncmp(mstr, Cnames[i].name, len1)) == 0 )
						   		++i;
						   
					/* found a partial match - skip word */
					if( rc == 0 )
						do
							cmd[ len++ ] = *s;
						while( *s && *s++ != ' ' );
				}
			}
			else 
			{	/* Found unique match */
				*(*macro)++ = '\r';
				return;
			}
		}
	}
}


/*
Given a "key string", calculate the key value.
Returns NUMKEYS for badly formatted strings.
Formats:
	?			C-?			PC	F1   thru F10
	C-X ?		C-X C-?		PC	M-F1 thru M-F10
	M-?			M-C-?
*/
int Key( s )
char *s;
{
	int k = 0;
	
	Unspace( s, '#' );

#if TERMCAP
	if( *s == 'K' ) return( Termkeyit(s) );
#endif
#if MSDOS
	if( *s == 'F' ) return( Func(s, 0x3a) );
#endif

	/* handle C-X and M- */
	if( strncmp(s, "C-X", 3) == 0 && *(s + 3) )
	{
		s += 3;
		k = 256;
	}
	else if( strncmp(s, "M-", 2) == 0 )
	{
		s += 2;
		k = 128;
#if MSDOS
		if( *s == 'F' )
			return( Func(s, 0x67) );
		else if( *(s + 1) == '\0' && (k += Altit(*s)) < NUMKEYS )
			return( k );
		else
			return( NUMKEYS );
#endif
	}

	/* handle C- */
	if( strncmp(s, "C-", 2) == 0 )
	{
		s += 2;
		k -= 'A' - 1;
	}
	
	/* normal character */
	if( *s && *(s + 1) == '\0' )
		return( k + *s );
	return( NUMKEYS );
}


/* Remove spaces in 's' stopping at NULL or char 'end'. UPPERCASES. */
Unspace( s, end )
char *s, end;
{
	char *p;
	
	for( p = s; *s && *s != end; ++s )
		if( !isspace(*s) )
			*p++ = TOUPPER( *s );
	*p = '\0';
}


#if TERMCAP
int Termkeyit( s )
char *s;
{
	extern char *strchr();
	static char a1[] = "UDRLH0123456789";
	char *p;
	
	if( (p = strchr(a1, *(s + 1))) && *(s + 2) == '\0' )
		return( (p - a1) + TC_UP );
	return( NUMKEYS );
}
#endif


#if MSDOS
int Altit( ch )
char ch;
{
	extern char *strchr();

	static char a1[] = "QWERTYUIOP....ASDFGHJKL.....ZXCVBNM";
	static char a2[] = "!@#$%^&*";
	static char a3[] = "()-=";

	char *p;
	int kbd;

	if( ch == '.' ) return( NUMKEYS );
	if( p = strchr(a1, ch) ) return( (p - a1) + 0x10 );
	if( p = strchr(a2, ch) ) return( (p - a2) + 0x78 );
	if( p = strchr(a3, ch) ) return( (p - a3) );
}


int Func( s, add )
char *s;
int add;
{
	int k;
	
	k = atoi( s + 1 );
	if( k >= 1 && k <= 10 )
		return( k + add + 128 );
	else
		return( NUMKEYS );
}
#endif


int Strnicmp( s, t, n )
char *s, *t;
int n;
{
	int i;
	
	for( i = 1; TOLOWER(*s) == TOLOWER(*t); ++s, ++t, ++i )
		if( *s == '\0' || i == n )
			return( 0 );
	return( TOLOWER(*s) - TOLOWER(*t) );

}
