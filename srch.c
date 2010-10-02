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


Boolean Insearch;	/* set by Nocase, reset by getarg */

static struct mark *Gmark;	/* used by global search routines */

#define QHELP	\
"Options: ' ' 'y'=change; 'n'=don't; '.'=change & quit; 'u'=undo; '^G'=quit"

void Zincsrch(void)
{
	Doincsrch("I-search: ", FORWARD);
}

void Zrincsrch(void)
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
		refresh();
		putpaw(str, 2);
		cmd = tgetcmd();
		if (isprint(cmd) && i < STRMAX) {
			bmrktopnt(&marks[i]);
			p[i++] = cmd;
			bmove(-i);
			if (Bsearch(p, forward)) {
				bmove(i);
			} else {
				bpnttomrk(&marks[--i]);
				p[i] = '\0';
				tbell();
			}
		} else if (cmd == 19) { /* CTRL-S */
			struct mark tmark;

			if (first) {
				/* use last search */
				int n = strlen(old);
				strcat(p, old);
				for (i = 0; i < n; ++i)
					bmrktopnt(&marks[i]);
			}

			bmrktopnt(&tmark); /* save in case search fails */
			if (Bsearch(p, forward))
				bmove(i);
			else {
				bpnttomrk(&tmark);
				tbell();
			}
		} else if (Keys[cmd] == ZRDELCHAR && i > 0) {
			bpnttomrk(&marks[--i]);
			p[i] = '\0';
		} else if (Keys[cmd] != ZNOTIMPL) {
			if (cmd != CR)
				PUSHCMD(cmd);
			go = FALSE;
		}
		first = 0;
	}
	clrecho();
	if (Keys[cmd] == ZABORT)
		bpnttomrk(&marks[0]);
	else
		strcpy(old, p);
	searchdir[0] = forward;
	searchdir[1] = 0;
}

void Zsearch(void)
{
#if defined(XWINDOWS) && defined(POPTARTS)
	if (Argp) {
		PopupSearch();
		return;
	}
#endif
	Promptsearch("Search: ", FORWARD);
}

void Zrsearch(void)
{
	Promptsearch("Reverse Search: ", BACKWARD);
}

void Zgsearch(void)
{
	if (getarg(Nocase("Global Search: "), old, STRMAX))
		return;
	cswitchto(Bufflist);
	btostart();
	searchdir[0] = FORWARD;
	searchdir[1] = SGLOBAL;
	Zagain();
}

void Zresrch(void)
{
	Promptsearch("RE Search: ", REGEXP);
}

void Zgresrch(void)
{
	if (getarg(Nocase("Global RE Search: "), old, STRMAX))
		return;
	cswitchto(Bufflist);
	btostart();
	searchdir[0] = REGEXP;
	searchdir[1] = SGLOBAL;
	Zagain();
}

void Zagain(void)
{
	if (searchdir[1] == SGLOBAL) {
		if (!Gmark)
			Gmark = bcremrk(); /* set here in case exit/reload */
		while (!Promptsearch("", AGAIN)) {
			Curbuff = Curbuff->next;
			if (Curbuff) {
				cswitchto(Curbuff);
				btostart();
				Arg = 1;
			} else {
				bpnttomrk(Gmark);
				unmark(Gmark);
				Gmark = NULL;
				Curwdo->modeflags = INVALID;
				searchdir[1] = 0;
				return;
			}
		}
	} else
		Promptsearch("Search: ", AGAIN);
}

void Zreplace(void)
{
	Promptreplace(FORWARD);
}

void Zquery(void)
{
	Promptreplace(QUERY);
}

void Zrereplace(void)
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
	if (getarg(Nocase(prompt), old, STRMAX))
		return;
	rc = getarg(Nocase("with: "), new, STRMAX);
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
	pmark = bcremrk();

	if (type == REGEXP)
		rc = Compile((Byte *)old, ebuf, &ebuf[ESIZE]);
	if (rc)
		Regerr(rc);
	else if (Argp) {
		for (tbuff = Bufflist; tbuff && !exit; tbuff = tbuff->next) {
			cswitchto(tbuff);
			bmrktopnt(&tmark);
			btostart();
			while (Replaceone(type, &query, &exit, ebuf, crgone) &&
			       !exit)
				;
			bpnttomrk(&tmark);
		}
		clrecho();
		cswitchto(pmark->mbuff);
	} else if (!Replaceone(type, &query, &exit, ebuf, crgone) && !exit)
		Echo("Not Found");
	else
		clrecho();

	bpnttomrk(pmark);
	unmark(pmark);
}

static Boolean Replaceone(int type, Boolean *query, Boolean *exit, Byte *ebuf,
			  Boolean crgone)
{
	Boolean found = FALSE;
	char tchar = ',', *ptr;
	int dist, changeprev = 0;
	struct mark *prevmatch;

	prevmatch = bcremrk();
	Echo("Searching...");
	while (!*exit &&
	       (type == REGEXP ? Step(ebuf) : Bsearch(old, FORWARD))) {
		found = TRUE;
		if (*query) {
replace:
#if defined(XWINDOWS) && defined(POPTARTS)
input:
			switch (tchar = GetQueryCmd(tchar))
#else
			Echo("Replace? ");
			refresh();
input:
			switch (tchar = tgetcmd()) {
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
						tbell();
					else {
						bpnttomrk(prevmatch);
						if (changeprev) {
							bdelete(strlen(new));
							binstr(old);
							bpnttomrk(prevmatch);
						}
					}
					goto replace;

				case '?':
					Echo(QHELP);
					goto input;

				case 'S': /* skip file */
				case 's':
					unmark(prevmatch);
					return FALSE;

				case 'q': /* abort */
				case 'Q':
					*exit = TRUE;
					continue;

				default:
					if (Keys[(int)tchar] == ZABORT)
						*exit = TRUE;
					else {
						bmrktopnt(prevmatch);
						changeprev = 0;
						/* skip and continue */
						bmove1();
					}
					continue;
			}
		}
		bmrktopnt(prevmatch);
		changeprev = 1;

		/* change it! */
		if (type == REGEXP) {
			/* force killtomrk to delete previous kill */
			Lfunc = ZNOTIMPL;
			killtomrk(REstart);
			for (ptr = new; *ptr; ++ptr)
				switch (*ptr) {
				case '\\':
					binsert(*(++ptr) ? *ptr : '\\'); break;
				case '&':
					Zyank(); break;
				default:
					binsert(*ptr);
				}
		} else {
			bdelete(strlen(old));
			binstr(new);
		}
		if (*query && tchar == ',') {
			refresh();
			if (ask("Confirm? ") != YES) {
				/* change it back */
				if (type == REGEXP) {
					for (dist = 0;
					     !Bisatmrk(REstart);
					     ++dist, bmove(-1))
						;
					bdelete(dist);
					Zyank();
				} else {
					bmove(-strlen(new));
					bdelete(strlen(new));
					binstr(old);
				}
			}
		}
		/* special case for "^" && "$" search strings */
		if (type == REGEXP && (ISNL(Buff()) || circf) && !crgone)
			bmove1();
		if (*query)
			Echo("Searching...");
		else if (tkbrdy())
			*exit = TRUE;
	}
	unmark(prevmatch);
#if defined(XWINDOWS) && defined(POPTARTS)
	QueryDone();
#endif
	return found;
}

static Boolean Promptsearch(char *prompt, int type)
{
	if (*old == '\0' || type != AGAIN) {
		if (getarg(Nocase(prompt), old, STRMAX)) {
			Arg = 0;
			return ABORT;
		}
		searchdir[0] = type == AGAIN ? FORWARD : type;
		searchdir[1] = 0;
	}
	return Dosearch();
}

static Boolean Dosearch(void)
{
	Boolean found = TRUE;
	Byte ebuf[ESIZE];
	int fcnt = 0, rc;
	struct mark *tmark;

	tmark = bcremrk();
	bmove(searchdir[0] == BACKWARD ? -1 : 1);
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
			bpnttomrk(REstart);
	} else {
		while (Arg-- > 0 && found) {
			found = Bsearch(old, searchdir[0]);
			if (found)
				++fcnt;
			bmove1();
		}
		bmove(-1);
	}
	if (!found) {
		bpnttomrk(tmark);
		if (fcnt) {
			Echo("Found ");
			titot(fcnt);
		} else
			Echo("Not Found");
	} else
		clrecho();
	unmark(tmark);
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
				delta[toupper(str[i])] = len - i;
				str[i] = tolower(str[i]);
			}

		/* search forward*/
		bmove(len);
		while (!bisend()) {
			/* fast loop - delta will be 0 if matched */
			while (!bisend() && delta[Buff()])
				bmove(delta[Buff()]);
			/* slow loop */
			for (i = len;
				 (char)STRIP(Buff()) == str[i] ||
				 (!exact && tolower(STRIP(Buff())) == str[i]);
				 bmove(-1), --i)
					if (i == 0)
						return TRUE;
			/* compute shift. shift must be forward! */
			if (i + delta[Buff()] > len)
				shift = delta[Buff()];
			else
				shift = len - i + 1;
			bmove(shift);
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
				delta[toupper(str[i])] = -i;
				str[i] = tolower(str[i]);
			}
		/* reverse search */
		bmove(-len);
		while (!bisstart()) {
			/* fast loop - delta will be 0 if matched */
			while (delta[Buff()] && !bisstart())
				bmove(delta[Buff()]);
			/* slow loop */
			for (i = 0;
			     i <= len &&
				     ((char)STRIP(Buff()) == str[i] ||
				      (!exact &&
				       tolower(STRIP(Buff())) == str[i]));
			     ++i, bmove1())
				;
			if (i > len) {
				/* we matched! */
				bmove(-len - 1);
				return TRUE;
			}
			/* compute shift. shift must be backward! */
			bmove(delta[Buff()] + i < 0 ? delta[Buff()] : -i - 1);
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
		strup(upper);
		Insearch = TRUE;
	}
	return (Curbuff->bmode & EXACT) ? is : upper;
}
