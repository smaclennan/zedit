#include "z.h"

/*
Regular expression compile and match routines.
See regexp(5) and ed(1).
NOTES:	loc1 is now REstart
		loc2 was removed, buffer will be left pointing here!
		locs is set externally by ed and sed - removed!
*/

static Boolean advance ARGS((Byte*));
static Boolean ecmp ARGS((Mark*, int));
static void getrnge ARGS((Byte*));

#define	CBRA	2
#define	CCHR	4
#define	CDOT	8
#define	CCL		12
#define	CDOL	20
#define	CCEOF	22
#define	CKET	24
#define	CBACK	36

#define	STAR	01
#define RNGE	03

#define	NBRA	9

#define PLACE(c)	ep[c >> 3] |= bittab[c & 07]
#define ISTHERE(c)	(ep[c >> 3] & bittab[c & 07])
#define GETC()		(*sp++)
#define PEEKC()		(*sp)
#define UNGETC(c)	(--sp)
#define RETURN(c)   return(0)
#define ERROR(c)    return(c)

Mark *REstart;		/* assigned in Setup */
Mark braslist[NBRA];
Mark braelist[NBRA];
int ebra, nbra;

int	circf;
static int low;
static int size;

static Byte bittab[] = { 1, 2, 4, 8, 16, 32, 64, 128 };


/*
Step thru the buffer trying to match the compiled expression.
Returns TRUE if matched, else FALSE.
The point is left at the end of the matched string or the buffer end and
REstart points to the start of the match.
*/
Boolean Step( ep )
register Byte *ep;
{
	/* ^ must match from start */
	if( circf )
	{
		/* if not at the start of the current line - go to the next line */
		if( !Bisstart() )
		{
			Bmove( -1 );		/* check previous char */
			if( ISNL(Buff()) )	/* we where at start */
				Bmove1();		/* undo move */
			else
				Bcsearch( NL );	/* goto next line */
		}
	}

	/* regular algorithm */
	while( !Bisend() )
	{
		Bmrktopnt( REstart );
		if( advance(ep) )
			return( TRUE );
		if( circf )
			Bcsearch( NL );	/* goto next line */
		else
		{
			Bpnttomrk( REstart );
			Bmove1();
		}
	}
	return( FALSE );
}

/*
Called by step to try to match at current position.
*/
static Boolean advance( ep )
register Byte *ep;
{
	int c, ct;
	Mark *bbeg, curlp, tmark;

	while( !Bisend() )
	{
		switch( *ep++ )
		{
			case CCHR:
				if( *ep++ == STRIP(Buff()) )
					break;
				return( FALSE );
	
			case CDOT:
				if( ISNL(Buff()) )
					return( FALSE );
				break;
	
			case CDOL:
				if( ISNL(Buff()) )
					continue;
				return( FALSE );
	
			case CCEOF:
				return( TRUE );
	
			case CCL:
				if( ISTHERE(((unsigned char)STRIP(Buff()))) )
				{
					ep += 16;
					break;
				}
				return( FALSE );
			
			case CBRA:
				Bmrktopnt( &braslist[*ep++] );
				continue;
	
			case CKET:
				Bmrktopnt( &braelist[*ep++] );
				continue;
	
			case CCHR | RNGE:
				c = STRIP(Buff());
				Bmove1();
				getrnge( ep );
				while( low-- )
					if( STRIP(Buff()) != c )
						return( FALSE );
					else
						Bmove1();
				Bmrktopnt( &curlp );
				while( size-- && STRIP(Buff()) == c )
					Bmove1(); 
				if( size < 0 || STRIP(Buff()) != c )
					Bmove1();
				ep += 2;
				goto star;

			case CDOT | RNGE:
				getrnge( ep );
				while( low-- )
					if( ISNL(Buff()) )
						return( FALSE );
					else
						Bmove1();
				Bmrktopnt( &curlp );
				while( size-- && !ISNL(Buff()) )
						Bmove1();
				if(size < 0 || ISNL(Buff()) )
					Bmove1();
				ep += 2;
				goto star;
	
			case CCL | RNGE:
				getrnge( ep + 16 );
				while( low-- )
				{
					c = Buff() & 0177;
					Bmove1();
					if( !ISTHERE(c) )
						return( FALSE );
				}
				Bmrktopnt( &curlp );
				while( size-- )
				{
					c = Buff() & 0177;
					Bmove1();
					if( !ISTHERE(c) )
						break;
				}
				if( size < 0 )
					Bmove1();
				ep += 18;		/* 16 + 2 */
				goto star;
	
			case CBACK:
				bbeg = &braslist[ *ep ];			/* start of prev match */
				ct = &braelist[ *ep++ ] - bbeg;		/* length */
				if( ecmp(bbeg, ct) )				/* same?? */
					continue;
				return( FALSE );
	
			case CBACK | STAR:
				bbeg = &braslist[ *ep ];
				ct = &braelist[ *ep++ ] - bbeg;
				Bmrktopnt( &curlp );
				while( ecmp(bbeg, ct) ) ;
				while( Bisaftermrk(&curlp) || Bisatmrk(&curlp) )
				{
					if( advance(ep) ) return( TRUE );
					Bmove( -ct );
				}
				return( FALSE );
	
			case CDOT | STAR:
				Bmrktopnt( &curlp );        /* save the current position */
				Toendline();
				goto star;
	
			case CCHR | STAR:
				Bmrktopnt( &curlp );        /* save the current position */
				while( STRIP(Buff()) == *ep )      /* skip over matches */
					Bmove1();
				ep++;                       /* go on */
				goto star;
	
			case CCL | STAR:
				Bmrktopnt( &curlp );
				while( !Bisend() && ISTHERE(((unsigned char)STRIP(Buff()))) )
					Bmove1();
				ep += 16;
				goto star;
	
			star:
				do
				{
					Bmrktopnt( &tmark );
					if( advance(ep) )			/* try to match */
						return( TRUE );
					Bpnttomrk( &tmark );
					Bmove( -1 );				/* go back and try again */
				}
				while( Bisaftermrk(&curlp) );	/* till back to start */
				Bpnttomrk( &curlp );			/* Don't slip backwards */
				return( FALSE );
		}
		Bmove1();
	}
	return( *ep == CCEOF );
}

/*
Called by advance to match [] type ranges.
*/
static void getrnge( str )
Byte *str;
{
	low = *str++ & 0377;
	size = (*str == (Byte)'\377') ? 20000 : (*str & 0377) - low;
}

/*
Compare the string at buffer mrk start to the string at the current point.
Match cnt chars.
Returns TRUE if matched.
Moves the buffer point and start mrk.
*/
static Boolean ecmp( start, cnt )
Mark *start;
int cnt;
{
	Byte c;
	
	while( cnt-- > 0 && !Bisend() )
	{
		Bswappnt( start );
		c = STRIP(Buff());
		Bmove1();
		Bswappnt( start );
		if( STRIP(Buff()) != c )
			return( FALSE );
		Bmove1();
	}
	return( TRUE );
}


/*
Compile the match string.
Returns 0 if ok, else errnum.
*/
#define EOFCH	'\0'

int Compile(instring, ep, endbuf)
register Byte *ep;
Byte *instring, *endbuf;
{
	register Byte *sp = instring;
	register int c;
	Byte *lastep = instring;
	int cclcnt;
	char bracket[NBRA], *bracketp;
	int closed;
	int neg;
	int lc;
	int i, cflg;

	lastep = 0;
#ifdef ALLOW_NL
	if((c = GETC()) == EOFCH /* SAM || c == '\n'*/) {
/* SAM these changes allow \n as part of string
		if(c == '\n') {
			UNGETC(c);
		}
*/
#else
	if((c = GETC()) == EOFCH || c == '\n' ) {
		if(c == '\n') {
			UNGETC(c);
		}
#endif
		if(*ep == 0)
			ERROR(41);
		RETURN(ep);
	}
	bracketp = bracket;
	circf = closed = nbra = ebra = 0;
	if(c == '^')
		circf++;
	else
		UNGETC(c);
	while(1) {
		if(ep >= endbuf)
			ERROR(50);
		c = GETC();
		if(c != '*' && ((c != '\\') || (PEEKC() != '{')))
			lastep = ep;
		if(c == EOFCH) {
			*ep++ = CCEOF;
			RETURN(ep);
		}
		switch(c) {

		case '.':
			*ep++ = CDOT;
			continue;

#ifdef ALLOW_NL
/* SAM allow \n as part of string
		case '\n':
			UNGETC(c);
			*ep++ = CCEOF;
			RETURN(ep);
*/
#else
		case '\n':
			UNGETC(c);
			*ep++ = CCEOF;
			RETURN(ep);
#endif
		case '*':
			if(lastep == 0 || *lastep == CBRA || *lastep == CKET)
				goto defchar;
			*lastep |= STAR;
			continue;

		case '$':
			if(PEEKC() != EOFCH && PEEKC() != '\n')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			if(&ep[17] >= endbuf)
				ERROR(50);

			*ep++ = CCL;
			lc = 0;
			for(i = 0; i < 16; i++)
				ep[i] = 0;

			neg = 0;
			if((c = GETC()) == '^') {
				neg = 1;
				c = GETC();
			}

			do {
				if(c == '\0' || c == '\n')
					ERROR(49);
				if(c == '-' && lc != 0) {
					if((c = GETC()) == ']') {
						PLACE('-');
						break;
					}
					while(lc < c) {
						PLACE(lc);
						lc++;
					}
				}
				lc = c;
				PLACE(c);
			} while((c = GETC()) != ']');
			if(neg) {
				for(cclcnt = 0; cclcnt < 16; cclcnt++)
					ep[cclcnt] ^= -1;
				ep[0] &= 0376;		/* ignore NULL */
				ep[1] &= 0373;		/* ignore NL */
			}

			ep += 16;

			continue;

		case '\\':
			switch(c = GETC()) {

			case '(':
				if(nbra >= NBRA)
					ERROR(43);
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;

			case ')':
				if(bracketp <= bracket || ++ebra != nbra)
					ERROR(42);
				*ep++ = CKET;
				*ep++ = *--bracketp;
				closed++;
				continue;

			case '{':
				if(lastep == 0)
					goto defchar;
				*lastep |= RNGE;
				cflg = 0;
			nlim:
				c = GETC();
				i = 0;
				do {
					if('0' <= c && c <= '9')
						i = 10 * i + c - '0';
					else
						ERROR(48);
				} while(((c = GETC()) != '\\') && (c != ','));
				if(i > 255)
					ERROR(47);
				*ep++ = i;
				if(c == ',') {
					if(cflg++)
						ERROR(44);
					if((c = GETC()) == '\\')
						*ep++ = 255;
					else {
						UNGETC(c);
						goto nlim;
						/* get 2'nd number */
					}
				}
				if(GETC() != '}')
					ERROR(45);
				if(!cflg)	/* one number */
					*ep++ = i;
				else if((size_t)(ep[-1] & 0377) < (size_t)(ep[-2] & 0377))
					ERROR(46);
				continue;

			case '\n':
				ERROR(40);

			case 'n':
				c = '\n';
				goto defchar;

			default:
				if(c >= '1' && c <= '9') {
					if((c -= '1') >= closed)
						ERROR(51);
					*ep++ = CBACK;
					*ep++ = c;
					continue;
				}
			}
	/* Drop through to default to use \ to turn off special chars */

		defchar:
		default:
			lastep = ep;
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
}

void Regerr( errnum )
int errnum;
{
	static char *errs[] = {		
		/*40*/	"Illegal or missing delimiter.",
		/*41*/	"No search string.",
		/*42*/	"() imbalance.",
		/*43*/	"Too many (.",
		/*44*/	"More than two numbers given in {}.",
		/*45*/	"} expected after .",
		/*46*/	"First number exceeds second in {}.",
		/*47*/	"Range endpoint too large.",
		/*48*/	"Bad number.",
		/*49*/	"[] imbalance.",
		/*50*/	"Regular expression overflow.",
		/*51*/	"\"digit\" out of range.",
	};

	Error(errs[errnum-40]);
}
