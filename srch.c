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


static Boolean promptsearch(char *prompt, int type);
static void promptreplace(int type);
static Boolean dosearch(void);
static Boolean replaceone(int, Boolean *, Boolean *, Byte *, Boolean);


Boolean Insearch;	/* set by nocase, reset by getarg */

static char old[STRMAX + 1];	/* Search string */
static char new[STRMAX + 1];	/* Replace string */
static Boolean searchdir[2];	/* Current direction for Again. */
static struct mark *Gmark;	/* used by global search routines */

#define QHELP	\
"Options: ' ' 'y'=change; 'n'=don't; '.'=change & quit; 'u'=undo; '^G'=quit"

static void doincsrch(char *prompt, Boolean forward)
{
	Boolean go = TRUE;
	char str[STRMAX + 1], promptstr[40];
	int cmd, i = 0, count = 0;
	struct mark marks[STRMAX];

	memset(str, '\0', sizeof(str));
	strcpy(promptstr, nocase(prompt));
	while (go) {
		zrefresh();
		putpaw("%s: %s", promptstr, str);
		cmd = tgetcmd();

		if (cmd == 0x13) { /* CTRL-S */
			struct mark tmark;

			if (*str == '\0') {
				/* use last search */
				int n = strlen(old);
				if (n == 0)
					continue;
				strcpy(str, old);
				for (i = 0; i < n; ++i)
					bmrktopnt(&marks[i]);
			}

			bmrktopnt(&tmark); /* save in case search fails */
		again:
			if (bstrsearch(str, forward))
				bmove(i);
			else if (++count == 2) {
				if (strstr(promptstr, "Wrap") == NULL)
					strcat(promptstr, " (Wrapped)");
				count = 0;
				btostart();
				goto again;
			} else {
				bpnttomrk(&tmark);
				tbell();
			}

			continue;
		}

		if (isprint(cmd)) {
			if (i < STRMAX) {
				bmrktopnt(&marks[i]);
				str[i++] = cmd;
				bmove(-i);
				if (bstrsearch(str, forward)) {
					bmove(i);
				} else {
					bpnttomrk(&marks[--i]);
					str[i] = '\0';
					tbell();
				}
			} else
				tbell();
		} else if (Keys[cmd] == ZRDELCHAR) {
			if (i > 0) {
				bpnttomrk(&marks[--i]);
				str[i] = '\0';
			} else
				tbell();
		} else if (Keys[cmd] != ZNOTIMPL) {
			if (cmd != CR)
				PUSHCMD(cmd);
			go = FALSE;
		}
		count = 0;
	}
	clrecho();
	if (Keys[cmd] == ZABORT)
		bpnttomrk(&marks[0]);
	else
		strcpy(old, str);
	searchdir[0] = forward;
	searchdir[1] = 0;
}

void Zincsrch(void)
{
	doincsrch("I-search", FORWARD);
}

void Zrincsrch(void)
{
	doincsrch("Reverse I-search", BACKWARD);
}

void Zsearch(void)
{
	promptsearch("Search: ", FORWARD);
}

void Zrsearch(void)
{
	promptsearch("Reverse Search: ", BACKWARD);
}

void Zgsearch(void)
{
	if (getarg(nocase("Global Search: "), old, STRMAX))
		return;
	cswitchto(Bufflist);
	btostart();
	searchdir[0] = FORWARD;
	searchdir[1] = SGLOBAL;
	Zagain();
}

void Zresrch(void)
{
	promptsearch("RE Search: ", REGEXP);
}

void Zgresrch(void)
{
	if (getarg(nocase("Global RE Search: "), old, STRMAX))
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
		while (!promptsearch("", AGAIN)) {
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
		promptsearch("Search: ", AGAIN);
}

void Zreplace(void)
{
	promptreplace(FORWARD);
}

void Zquery(void)
{
	promptreplace(QUERY);
}

void Zrereplace(void)
{
	promptreplace(REGEXP);
}

static void doreplace(int type)
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
		rc = compile((Byte *)old, ebuf, &ebuf[ESIZE]);
	if (rc)
		regerr(rc);
	else if (Argp) {
		for (tbuff = Bufflist; tbuff && !exit; tbuff = tbuff->next) {
			cswitchto(tbuff);
			bmrktopnt(&tmark);
			btostart();
			while (replaceone(type, &query, &exit, ebuf, crgone) &&
			       !exit)
				;
			bpnttomrk(&tmark);
		}
		clrecho();
		cswitchto(pmark->mbuff);
	} else if (!replaceone(type, &query, &exit, ebuf, crgone) && !exit)
		echo("Not Found");
	else
		clrecho();

	bpnttomrk(pmark);
	unmark(pmark);
}

static void promptreplace(int type)
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
	if (getarg(nocase(prompt), old, STRMAX))
		return;
	rc = getarg(nocase("with: "), new, STRMAX);
	if (rc == ABORT)
		return;
	if (rc)
		*new = '\0';

	doreplace(type);
}

static Boolean replaceone(int type, Boolean *query, Boolean *exit, Byte *ebuf,
			  Boolean crgone)
{
	Boolean found = FALSE;
	char tchar = ',', *ptr;
	int dist, changeprev = 0;
	struct mark *prevmatch;

	prevmatch = bcremrk();
	echo("Searching...");
	while (!*exit &&
	       (type == REGEXP ? step(ebuf) : bstrsearch(old, FORWARD))) {
		found = TRUE;
		if (*query) {
replace:
			echo("Replace? ");
			zrefresh();
input:
			switch (tchar = tgetcmd()) {
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
				echo("Replacing...");
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
				echo(QHELP);
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
			zrefresh();
			if (ask("Confirm? ") != YES) {
				/* change it back */
				if (type == REGEXP) {
					for (dist = 0;
					     !bisatmrk(REstart);
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
			echo("Searching...");
		else if (tkbrdy())
			*exit = TRUE;
	}
	unmark(prevmatch);
	return found;
}

static Boolean promptsearch(char *prompt, int type)
{
	if (*old == '\0' || type != AGAIN) {
		if (getarg(nocase(prompt), old, STRMAX)) {
			Arg = 0;
			return ABORT;
		}
		searchdir[0] = type == AGAIN ? FORWARD : type;
		searchdir[1] = 0;
	}
	return dosearch();
}

static Boolean dosearch(void)
{
	Boolean found = TRUE;
	Byte ebuf[ESIZE];
	int fcnt = 0, rc;
	struct mark *tmark;

	tmark = bcremrk();
	bmove(searchdir[0] == BACKWARD ? -1 : 1);
	echo("Searching...");
	if (searchdir[0] == REGEXP) {
		rc = compile((Byte *)old, ebuf, &ebuf[ESIZE]);
		if (rc == 0) {
			while (Arg-- > 0 && found) {
				found = step(ebuf);
				if (found)
					++fcnt;
			}
		} else
			regerr(rc);
		if (found)
			bpnttomrk(REstart);
	} else {
		while (Arg-- > 0 && found) {
			found = bstrsearch(old, searchdir[0]);
			if (found)
				++fcnt;
			bmove1();
		}
		bmove(-1);
	}
	if (!found) {
		bpnttomrk(tmark);
		if (fcnt) {
			echo("Found ");
			titot(fcnt);
		} else
			echo("Not Found");
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
Boolean bstrsearch(char *str, Boolean forward)
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

char *nocase(char *prompt)
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
