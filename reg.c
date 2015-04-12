#include "buff.h"

#define BUILTIN_REG
#ifndef BUILTIN_REG
#include <regex.h>

/* This is for zedit... do not rely on it in threaded code */
int circf;

bool step(struct buff *buff, uint8_t *ep, struct mark *REstart)
{
	return false;
}

int compile(Byte *instring, uint8_t *ep, uint8_t *endbuf)
{
	circf = 0;
	if (*instring == '^')
		circf = 1;

	return 0;
}

const char *regerr(int errnum)
{
	return "duh";
}
#else
/* Regular expression compile and match routines.
 * See regexp(5) and ed(1).
 * NOTES:	loc1 is now REstart
 *		loc2 was removed, buffer will be left pointing here!
 *		locs is set externally by ed and sed - removed!
 */

static bool advance(struct buff *buff, uint8_t *ep);
static bool ecmp(struct buff *buff, struct mark *, int);
static void getrnge(uint8_t *);

#define	CBRA	2
#define	CCHR	4
#define	CDOT	8
#define	CCL	12
#define	CDOL	20
#define	CCEOF	22
#define	CKET	24
#define	CBACK	36

#define	STAR	01
#define RNGE	03

#define	NBRA	9

#define PLACE(c)	(ep[c >> 3] |= bittab[c & 07])
#define ISTHERE(c)	(ep[c >> 3] & bittab[c & 07])
#define GETC()		(*sp++)
#define PEEKC()		(*sp)
#define UNGETC(c)	(--sp)
#define ISNL(c)			((c) == '\n')
#define buff() (*buff->curcptr)

static struct mark braslist[NBRA];
static struct mark braelist[NBRA];
static int ebra, nbra;

int	circf;
static int low;
static int size;

static Byte bittab[] = { 1, 2, 4, 8, 16, 32, 64, 128 };


/* Step thru the buffer trying to match the compiled expression.
 * Returns true if matched, else false.
 * The point is left at the end of the matched string or the buffer end and
 * REstart points to the start of the match.
 */
bool step(struct buff *buff, uint8_t *ep, struct mark *REstart)
{
	struct mark tmark;

	if (REstart == NULL) REstart = &tmark;

	/* ^ must match from start */
	if (circf)
		/* if not at the start of the current line - go to the
		 * next line */
		if (bpeek(buff) != '\n')
			bcsearch(buff, '\n');	/* goto next line */

	/* regular algorithm */
	while (!bisend(buff)) {
		bmrktopnt(buff, REstart);
		if (advance(buff, ep))
			return true;
		if (circf)
			bcsearch(buff, '\n');	/* goto next line */
		else {
			bpnttomrk(buff, REstart);
			bmove1(buff);
		}
	}
	return false;
}

bool lookingat(struct buff *buff, Byte *str)
{
	uint8_t ep[ESIZE];
	if (compile(str, ep, ep + ESIZE))
		return false;

	struct mark tmark;
	bmrktopnt(buff, &tmark);
	if (advance(buff, ep))
		return true;

	bpnttomrk(buff, &tmark);
	return false;
}

/* Called by step to try to match at current position. */
static bool advance(struct buff *buff, uint8_t *ep)
{
	int c, ct;
	struct mark *bbeg, curlp, tmark;

	while (!bisend(buff)) {
		switch (*ep++) {
		case CCHR:
			if (*ep++ == buff())
				break;
			return false;

		case CDOT:
			if (ISNL(buff()))
				return false;
			break;

		case CDOL:
			if (ISNL(buff()))
				continue;
			return false;

		case CCEOF:
			return true;

		case CCL:
			if (ISTHERE(((Byte)buff()))) {
				ep += 16;
				break;
			}
			return false;

		case CBRA:
			bmrktopnt(buff, &braslist[*ep++]);
			continue;

		case CKET:
			bmrktopnt(buff, &braelist[*ep++]);
			continue;

		case CCHR | RNGE:
			c = *ep++;
			getrnge(ep);
			while (low--) {
				if (buff() != c)
					return false;
				bmove1(buff);
			}
			bmrktopnt(buff, &curlp);
			while (size-- && buff() == c)
				bmove1(buff);
			ep += 2;
			goto star;

		case CDOT | RNGE:
			getrnge(ep);
			while (low--)
				if (ISNL(buff()))
					return false;
				else
					bmove1(buff);
			bmrktopnt(buff, &curlp);
			while (size-- && !ISNL(buff()))
				bmove1(buff);
			if (size < 0 || ISNL(buff()))
				bmove1(buff);
			ep += 2;
			goto star;

		case CCL | RNGE:
			getrnge(ep + 16);
			while (low--) {
				c = buff() & 0177;
				bmove1(buff);
				if (!ISTHERE(c))
					return false;
			}
			bmrktopnt(buff, &curlp);
			while (size--) {
				c = buff() & 0177;
				bmove1(buff);
				if (!ISTHERE(c))
					break;
			}
			/* SAM why? goes one too far for \{m\} */
// SAM			if (size < 0)
// SAM				bmove1(buff);
			ep += 18;		/* 16 + 2 */
			goto star;

		case CBACK:
			bbeg = &braslist[*ep]; /* start of prev match */
			ct = &braelist[*ep++] - bbeg; /* length */
			if (ecmp(buff, bbeg, ct)) /* same?? */
				continue;
			return false;

		case CBACK | STAR:
			bbeg = &braslist[*ep];
			ct = &braelist[*ep++] - bbeg;
			bmrktopnt(buff, &curlp);
			while (ecmp(buff, bbeg, ct))
				;
			while (bisaftermrk(buff, &curlp) || bisatmrk(buff, &curlp)) {
				if (advance(buff, ep))
					return true;
				bmove(buff, -ct);
			}
			return false;

		case CDOT | STAR:
			bmrktopnt(buff, &curlp); /* save the current position */
			toendline(buff);
			goto star;

		case CCHR | STAR:
			bmrktopnt(buff, &curlp); /* save the current position */
			while (buff() == *ep)  /* skip over matches */
				bmove1(buff);
			ep++;                       /* go on */
			goto star;

		case CCL | STAR:
			bmrktopnt(buff, &curlp);
			while (!bisend(buff) && ISTHERE(((Byte)buff())))
				bmove1(buff);
			ep += 16;

star:
			do {
				bmrktopnt(buff, &tmark);
				if (advance(buff, ep)) /* try to match */
					return true;
				bpnttomrk(buff, &tmark);
				if (!bmove(buff, -1)) /* go back and try again */
					break;
			} while (!bisbeforemrk(buff, &curlp)); /* till back to start */
			bpnttomrk(buff, &curlp); /* Don't slip backwards */
			return false;
		}
		bmove1(buff);
	}
	return *ep == CCEOF;
}

/* Called by advance to match \{\} type ranges. */
static void getrnge(uint8_t *str)
{
	low = *str++ & 0377;
	size = (*str == (uint8_t)'\377') ? 20000 : (*str & 0377) - low;
}

/* Compare the string at buffer mrk start to the string at the current point.
 * Match cnt chars.
 * Returns true if matched.
 * Moves the buffer point and start mrk.
 */
static bool ecmp(struct buff *buff, struct mark *start, int cnt)
{
	Byte c;

	while (cnt-- > 0 && !bisend(buff)) {
		bswappnt(buff, start);
		c = buff();
		bmove1(buff);
		bswappnt(buff, start);
		if (buff() != c)
			return false;
		bmove1(buff);
	}
	return true;
}

/* Compile the match string.
 * Returns 0 if ok, else errnum.
 */
#define EOFCH	('\0')

int compile(Byte *instring, uint8_t *ep, uint8_t *endbuf)
{
	Byte *sp = instring;
	int c;
	uint8_t *lastep = NULL;
	int cclcnt;
	char bracket[NBRA], *bracketp;
	int closed;
	int neg;
	int lc;
	int i, cflg;

	c = GETC();
#ifdef ALLOW_NL
	if (c == EOFCH) {
		if (*ep == NULL)
			return 41;
		return 0;
	}
#else
	if (c == EOFCH || c == '\n') {
		if (c == '\n')
			UNGETC(c);
		if (*ep == 0)
			return 41;
		return 0;
	}
#endif
	bracketp = bracket;
	circf = closed = nbra = ebra = 0;
	if (c == '^')
		circf++;
	else
		UNGETC(c);
	while (1) {
		if (ep >= endbuf)
			return 50;
		c = GETC();
		if (c != '*' && ((c != '\\') || (PEEKC() != '{')) && c != '?')
			lastep = ep;
		if (c == EOFCH) {
			*ep++ = CCEOF;
			return 0;
		}
		switch (c) {

		case '.':
			*ep++ = CDOT;
			continue;

#ifndef ALLOW_NL
		case '\n':
			UNGETC(c);
			*ep++ = CCEOF;
			return 0;
#endif
		case '*':
			if (!lastep || *lastep == CBRA || *lastep == CKET)
				goto defchar;
			*lastep |= STAR;
			continue;

		case '$':
			if (PEEKC() != EOFCH && PEEKC() != '\n')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			if (&ep[17] >= endbuf)
				return 50;

			*ep++ = CCL;
			lc = 0;
			for (i = 0; i < 16; i++)
				ep[i] = 0;

			neg = 0;
			c = GETC();
			if (c == '^') {
				neg = 1;
				c = GETC();
			}

			do {
				if (c == '\0' || c == '\n')
					return 49;
				if (c == '-' && lc != 0) {
					c = GETC();
					if (c == ']') {
						PLACE('-');
						break;
					}
					while (lc < c) {
						PLACE(lc);
						lc++;
					}
				}
				lc = c;
				PLACE(c);
			} while ((c = GETC()) != ']');
			if (neg) {
				for (cclcnt = 0; cclcnt < 16; cclcnt++)
					ep[cclcnt] ^= -1;
				ep[0] &= 0376;		/* ignore NULL */
				ep[1] &= 0373;		/* ignore NL */
			}

			ep += 16;

			continue;

		case '?': /* \{0,1\} */
			if (lastep == NULL)
				goto defchar;
			*lastep |= RNGE;
			*ep++ = 0;
			*ep++ = 1;
			continue;

		case '\\':
			switch (c = GETC()) {

			case '(':
				if (nbra >= NBRA)
					return 43;
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;

			case ')':
				if (bracketp <= bracket || ++ebra != nbra)
					return 42;
				*ep++ = CKET;
				*ep++ = *--bracketp;
				closed++;
				continue;

			case '{':
				if (lastep == NULL)
					goto defchar;
				*lastep |= RNGE;
				cflg = 0;
nlim:
				c = GETC();
				i = 0;
				do {
					if ('0' <= c && c <= '9')
						i = 10 * i + c - '0';
					else
						return 48;
				} while (((c = GETC()) != '\\') && (c != ','));
				if (i > 255)
					return 47;
				*ep++ = i;
				if (c == ',') {
					if (cflg++)
						return 44;
					c = GETC();
					if (c == '\\')
						*ep++ = 255;
					else {
						UNGETC(c);
						goto nlim;
						/* get 2'nd number */
					}
				}
				if (GETC() != '}')
					return 45;
				if (!cflg)	/* one number */
					*ep++ = i;
				else if ((ep[-1] & 0377) < (ep[-2] & 0377))
					return 46;
				continue;

			case '\n':
				return 40;

			case 'n':
				c = '\n';
				goto defchar;

			default:
				if (c >= '1' && c <= '9') {
					if (--c  >= closed)
						return 51;
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

const char *regerr(int errnum)
{
	static const char * const errs[] = {
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
		/*52*/  "Out of memory.",
	};

	return errs[errnum - 40];
}
#endif
