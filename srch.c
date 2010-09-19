/* srch.c - Zedit search functions
 * Copyright (C) 1988-2010 Sean MacLennan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "z.h"


static Boolean Promptsearch(char *prompt, int type);
static void Promptreplace(int type);
static Boolean Dosearch(void);
static Boolean Replaceone(int, Boolean *, Boolean *, Byte *, Boolean);


Boolean Insearch;	/* set by Nocase, reset by Getarg */

struct mark *Gmark;		/* used by global search routines */

#define QHELP	\
"Options: ' ' 'y'=change; 'n'=don't; '.'=change & quit; 'u'=undo; '^G'=quit"

void Zincsrch()
{
	Doincsrch("I-search: ", FORWARD);
}

void Zrincsrch()
{
	Doincsrch("Reverse I-search: ", BACKWARD);
}

void Doincsrch(char *prompt, Boolean forward)
{
	Boolean go = TRUE;
	char str[STRMAX + 1], *p;
	int cmd, i = 0, first = 1;
	struct mark marks[STRMAX];

	memset(str, '\0', STRMAX);
	strcpy(str, Nocase(prompt));
	p = str + strlen(str);
	while (go) {
		Refresh();
		PutPaw(str, 2);
		cmd = Tgetcmd();
		if (isprint(cmd) && i < STRMAX) {
			Bmrktopnt(&marks[i]);
			p[i++] = cmd;
			Bmove(-i);
			if (Bsearch(p, forward)) {
				Bmove(i);
			} else {
				Bpnttomrk(&marks[--i]);
				p[i] = '\0';
				Tbell();
			}
		} else if (cmd == 19) { /* CTRL-S */
			struct mark tmark;

			if (first) {
				/* use last search */
				int n = strlen(old);
				strcat(p, old);
				for (i = 0; i < n; ++i)
					Bmrktopnt(&marks[i]);
			}

			Bmrktopnt(&tmark); /* save in case search fails */
			if (Bsearch(p, forward))
				Bmove(i);
			else {
				Bpnttomrk(&tmark);
				Tbell();
			}
		} else if (Keys[cmd] == ZRDELCHAR && i > 0) {
			Bpnttomrk(&marks[--i]);
			p[i] = '\0';
		} else if (Keys[cmd] != ZNOTIMPL) {
			if (cmd != CR)
				Pushcmd(cmd);
			go = FALSE;
		}
		first = 0;
	}
	Clrecho();
	if (Keys[cmd] == ZABORT)
		Bpnttomrk(&marks[0]);
	else
		strcpy(old, p);
	searchdir[0] = forward;
	searchdir[1] = 0;
}

void Zsearch()
{
#if XWINDOWS && defined(POPTARTS)
	if (Argp) {
		PopupSearch();
		return;
	}
#endif
	Promptsearch("Search: ", FORWARD);
}

void Zrsearch()
{
	Promptsearch("Reverse Search: ", BACKWARD);
}

void Zgsearch()
{
	if (Getarg(Nocase("Global Search: "), old, STRMAX))
		return;
	Cswitchto(Bufflist);
	Btostart();
	searchdir[0] = FORWARD;
	searchdir[1] = SGLOBAL;
	Zagain();
}

void Zresrch()
{
	Promptsearch("RE Search: ", REGEXP);
}

void Zgresrch()
{
	if (Getarg(Nocase("Global RE Search: "), old, STRMAX))
		return;
	Cswitchto(Bufflist);
	Btostart();
	searchdir[0] = REGEXP;
	searchdir[1] = SGLOBAL;
	Zagain();
}

void Zagain()
{
	if (searchdir[1] == SGLOBAL) {
		if (!Gmark)
			Gmark = Bcremrk(); /* set here in case exit/reload */
		while (!Promptsearch("", AGAIN)) {
			Curbuff = Curbuff->next;
			if (Curbuff) {
				Cswitchto(Curbuff);
				Btostart();
				Arg = 1;
			} else {
				Bpnttomrk(Gmark);
				Unmark(Gmark);
				Gmark = NULL;
				Curwdo->modeflags = INVALID;
				searchdir[1] = 0;
				return;
			}
		}
	} else
		Promptsearch("Search: ", AGAIN);
}

void Zreplace()
{
	Promptreplace(FORWARD);
}

void Zquery()
{
	Promptreplace(QUERY);
}

void Zrereplace()
{
	Promptreplace(REGEXP);
}

static void Promptreplace(int type)
{
	char *prompt = NULL;
	int rc;

	switch (type) {
	case FORWARD:
		prompt = "Replace: ";		break;
	case QUERY:
		prompt = "QReplace: ";		break;
	case REGEXP:
		prompt = "RE Replace: ";	break;
	}
	if (Getarg(Nocase(prompt), old, STRMAX))
		return;
	rc = Getarg(Nocase("with: "), new, STRMAX);
	if (rc == ABORT)
		return;
	if (rc)
		*new = '\0';

	Doreplace(type);
}

void Doreplace(int type)
{
	Boolean exit = FALSE, crgone, query;
	Byte ebuf[ESIZE];
	struct mark *pmark, tmark;
	struct buff *tbuff;
	int rc = 0;

	Arg = 0;

	query = type == FORWARD ? FALSE : TRUE;

	crgone = *old && *(old + strlen(old) - 1) == '\n';
	pmark = Bcremrk();

	if (type == REGEXP)
		rc = Compile((Byte *)old, ebuf, &ebuf[ESIZE]);
	if (rc)
		Regerr(rc);
	else if (Argp) {
		for (tbuff = Bufflist; tbuff && !exit; tbuff = tbuff->next) {
			Cswitchto(tbuff);
			Bmrktopnt(&tmark);
			Btostart();
			while (Replaceone(type, &query, &exit, ebuf, crgone) &&
			       !exit)
				;
			Bpnttomrk(&tmark);
		}
		Clrecho();
		Cswitchto(pmark->mbuff);
	} else if (!Replaceone(type, &query, &exit, ebuf, crgone) && !exit)
		Echo("Not Found");
	else
		Clrecho();

	Bpnttomrk(pmark);
	Unmark(pmark);
}

static Boolean Replaceone(int type, Boolean *query, Boolean *exit, Byte *ebuf,
			  Boolean crgone)
{
	Boolean found = FALSE;
	char tchar = ',', *ptr;
	int dist, changeprev = 0;
	struct mark *prevmatch;

	prevmatch = Bcremrk();
	Echo("Searching...");
	while (!*exit &&
	       (type == REGEXP ? Step(ebuf) : Bsearch(old, FORWARD))) {
		found = TRUE;
		if (*query) {
replace:
#if XWINDOWS && defined(POPTARTS)
input:
			switch (tchar = GetQueryCmd(tchar))
#else
			Echo("Replace? ");
			Refresh();
input:
			switch (tchar = Tgetcmd()) {
#endif
				case ' ':
				case ',':	/* handled later */
				case 'Y':
				case 'y':
					break;	/* do the change */

				case '.':
					*exit = TRUE; /* change, then abort */
					break;

				case '!':
					*query = FALSE; /* global change */
					Echo("Replacing...");
					break;

				case 'U': /* goto prev match */
				case 'u':
					if (type == REGEXP)
						Tbell();
					else {
						Bpnttomrk(prevmatch);
						if (changeprev) {
							Bdelete(strlen(new));
							Binstr(old);
							Bpnttomrk(prevmatch);
						}
					}
					goto replace;

				case '?':
					Echo(QHELP);
					goto input;

				case 'S': /* skip file */
				case 's':
					Unmark(prevmatch);
					return FALSE;

				case 'q': /* abort */
				case 'Q':
					*exit = TRUE;
					continue;

				default:
					if (Keys[(int)tchar] == ZABORT)
						*exit = TRUE;
					else {
						Bmrktopnt(prevmatch);
						changeprev = 0;
						/* skip and continue */
						Bmove1();
					}
					continue;
			}
		}
		Bmrktopnt(prevmatch);
		changeprev = 1;

		/* change it! */
		if (type == REGEXP) {
			/* force Killtomrk to delete previous kill */
			Lfunc = ZNOTIMPL;
			Killtomrk(REstart);
			for (ptr = new; *ptr; ++ptr)
				switch (*ptr) {
				case '\\':
					Binsert(*(++ptr) ? *ptr : '\\'); break;
				case '&':
					Zyank(); break;
				default:
					Binsert(*ptr);
				}
		} else {
			Bdelete(strlen(old));
			Binstr(new);
		}
		if (*query && tchar == ',') {
			Refresh();
			if (Ask("Confirm? ") != YES) {
				/* change it back */
				if (type == REGEXP) {
					for (dist = 0;
					     !Bisatmrk(REstart);
					     ++dist, Bmove(-1))
						;
					Bdelete(dist);
					Zyank();
				} else {
					Bmove(-strlen(new));
					Bdelete(strlen(new));
					Binstr(old);
				}
			}
		}
		/* special case for "^" && "$" search strings */
		if (type == REGEXP && (ISNL(Buff()) || circf) && !crgone)
			Bmove1();
		if (*query)
			Echo("Searching...");
		else if (Tkbrdy())
			*exit = TRUE;
	}
	Unmark(prevmatch);
#if XWINDOWS && defined(POPTARTS)
	QueryDone();
#endif
	return found;
}

static Boolean Promptsearch(char *prompt, int type)
{
	if (*old == '\0' || type != AGAIN) {
		if (Getarg(Nocase(prompt), old, STRMAX)) {
			Arg = 0;
			return ABORT;
		}
		searchdir[0] = type == AGAIN ? FORWARD : type;
		searchdir[1] = 0;
	}
	return Dosearch();
}

static Boolean Dosearch()
{
	Boolean found = TRUE;
	Byte ebuf[ESIZE];
	int fcnt = 0, rc;
	struct mark *tmark;

	tmark = Bcremrk();
	Bmove(searchdir[0] == BACKWARD ? -1 : 1);
	Echo("Searching...");
	if (searchdir[0] == REGEXP) {
		rc = Compile((Byte *)old, ebuf, &ebuf[ESIZE]);
		if (rc == 0) {
			while (Arg-- > 0 && found) {
				found = Step(ebuf);
				if (found)
					++fcnt;
			}
		} else
			Regerr(rc);
		if (found)
			Bpnttomrk(REstart);
	} else {
		while (Arg-- > 0 && found) {
			found = Bsearch(old, searchdir[0]);
			if (found)
				++fcnt;
			Bmove1();
		}
		Bmove(-1);
	}
	if (!found) {
		Bpnttomrk(tmark);
		if (fcnt) {
			Echo("Found ");
			Titot(fcnt);
		} else
			Echo("Not Found");
	} else
		Clrecho();
	Unmark(tmark);
	Arg = 0;
	return found;
}


/* This is an implementation of the Boyer-Moore Search.
 * It uses the delta1 only with the fast/slow loops.
 * It searchs for the string 'str' starting at the current buffer location. If
 * forward is TRUE, it searches forward, else it searches backwards.
 * The search will be case insensitive if the buffers current mode is so set.
 */
Boolean Bsearch(char *str, Boolean forward)
{
	Boolean exact;
	int delta[NUMASCII], len, i, shift;

	exact = Curbuff->bmode & EXACT;
	len = strlen(str) - 1;

	if (forward) {
		/* Init the delta table to str length.
		 * For each char in the str, store the offset from the
		 * str start in the delta table.
		 * If we are in case insensitive mode - lower case the
		 * match string and mark both the upper case version
		 * and the lower case version of the match string
		 * chars in the delta array.
		 */
		for (i = 0; i < NUMASCII; ++i)
			delta[i] = len ? len : 1;
		for (i = 0; i <= len;  ++i)
			delta[(int)str[i]] = len - i;
		if (!exact)
			for (i = 0; i <= len;  ++i) {
				delta[Toupper(str[i])] = len - i;
				str[i] = Tolower(str[i]);
			}

		/* search forward*/
		Bmove(len);
		while (!Bisend()) {
			/* fast loop - delta will be 0 if matched */
			while (!Bisend() && delta[Buff()])
				Bmove(delta[Buff()]);
			/* slow loop */
			for (i = len;
				 (char)STRIP(Buff()) == str[i] ||
				 (!exact && Tolower(STRIP(Buff())) == str[i]);
				 Bmove(-1), --i)
					if (i == 0)
						return TRUE;
			/* compute shift. shift must be forward! */
			if (i + delta[Buff()] > len)
				shift = delta[Buff()];
			else
				shift = len - i + 1;
			Bmove(shift);
		}
	} else {
		/* Init the delta table to str length.
		 * For each char in the str, store the negative offset
		 * from the str start in the delta table.
		 */
		for (i = 0; i < NUMASCII; ++i)
			delta[i] = len ? -len : -1;
		for (i = len; i >= 0; --i)
			delta[(int)str[i]] = -i;
		if (!exact)
			for (i = len; i >= 0; --i) {
				delta[Toupper(str[i])] = -i;
				str[i] = Tolower(str[i]);
			}
		/* reverse search */
		Bmove(-len);
		while (!Bisstart()) {
			/* fast loop - delta will be 0 if matched */
			while (delta[Buff()] && !Bisstart())
				Bmove(delta[Buff()]);
			/* slow loop */
			for (i = 0;
			     i <= len &&
				     ((char)STRIP(Buff()) == str[i] ||
				      (!exact &&
				       Tolower(STRIP(Buff())) == str[i]));
			     ++i, Bmove1())
				;
			if (i > len) {
				/* we matched! */
				Bmove(-len - 1);
				return TRUE;
			}
			/* compute shift. shift must be backward! */
			Bmove(delta[Buff()] + i < 0 ? delta[Buff()] : -i - 1);
		}
	}
	return FALSE;
}

char *Nocase(char *prompt)
{
	static char is[20], upper[20];

	if (prompt) {
		strcpy(is, prompt);
		strcpy(upper, prompt);
		Strup(upper);
		Insearch = TRUE;
	}
	return (Curbuff->bmode & EXACT) ? is : upper;
}
