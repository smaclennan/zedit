/* Zedit search functions
 * Copyright (C) 1988-2018 Sean MacLennan
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


/** @addtogroup zedit
 * @{
 */

static int promptsearch(const char *prompt, int type);
static void promptreplace(int type);
static bool dosearch(void);
static bool replaceone(int, bool *, bool *, struct regexp *, bool);


bool Insearch;	/* set by nocase, reset by getarg */

static char olds[STRMAX + 1];	/* Search string */
static char news[STRMAX + 1];	/* Replace string */
static int searchdir[2];	/* Current direction for Again. */

#define QHELP	\
"Options: ' ' 'y'=change; 'n'=don't; '.'=change & quit; 'u'=undo; '^G'=quit"

static bool bstrsearch(const char *str, bool forward)
{
	if (forward)
		return bm_search(Bbuff, str, Curbuff->bmode & EXACT);
	else
		return bm_rsearch(Bbuff, str, Curbuff->bmode & EXACT);
}

static void doincsrch(const char *prompt, bool forward)
{
	bool go = true;
	char str[STRMAX + 1], promptstr[40];
	int cmd, i = 0, count = 0;
	struct mark marks[STRMAX];

	bmrktopnt(Bbuff, &marks[0]); /* make sure this is set */
	memset(str, '\0', sizeof(str));
	strlcpy(promptstr, nocase(prompt), sizeof(promptstr));
	while (go) {
		zrefresh();
		putpaw("%s: %s", promptstr, str);
		cmd = tgetkb();

		if (cmd == TC_UNKNOWN) {
			tbell();
			continue;
		}

		if (cmd == 0x13) { /* CTRL-S */
			struct mark tmark;

			if (*str == '\0') {
				/* use last search */
				int n = strlen(olds);
				if (n == 0)
					continue;
				strcpy(str, olds);
				for (i = 0; i < n; ++i)
					bmrktopnt(Bbuff, &marks[i]);
			}

			 /* save in case search fails */
			bmrktopnt(Bbuff, &tmark);
again:
			if (!bstrsearch(str, forward)) {
				if (++count == 2) {
					strconcat(promptstr, sizeof(promptstr),
						  nocase(prompt), " (Wrapped)",
						  NULL);
					count = 0;
					btostart(Bbuff);
					goto again;
				} else {
					bpnttomrk(Bbuff, &tmark);
					tbell();
				}
			}

			continue;
		}

		if (isprint(cmd)) {
			if (i < STRMAX) {
				bmrktopnt(Bbuff, &marks[i]);
				str[i++] = cmd;
				bmove(Bbuff, -i);
				if (!bstrsearch(str, forward)) {
					bpnttomrk(Bbuff, &marks[--i]);
					str[i] = '\0';
					tbell();
				}
			} else
				tbell();
		} else if (Keys[cmd] == ZDELETE_PREVIOUS_CHAR) {
			if (i > 0) {
				bpnttomrk(Bbuff, &marks[--i]);
				str[i] = '\0';
			} else
				tbell();
		} else if (Keys[cmd] != ZNOTIMPL) {
			if (cmd != CR)
				Cmdpushed = cmd;
			go = false;
		}
		count = 0;
	}
	clrpaw();
	if (Keys[cmd] == ZABORT)
		bpnttomrk(Bbuff, &marks[0]);
	else
		strcpy(olds, str);
	searchdir[0] = forward;
	searchdir[1] = 0;
}

void Zincremental_search(void)
{
	doincsrch("I-search", FORWARD);
}

void Zsearch(void)
{
	promptsearch("Search: ", FORWARD);
}

void Zreverse_search(void)
{
	promptsearch("Reverse Search: ", BACKWARD);
}

void Zword_search(void)
{
	getbword(olds, sizeof(olds), bisword);

	searchdir[0] = FORWARD;
	searchdir[1] = 0;
	promptsearch(NULL, AGAIN);
}

void Zre_search(void)
{
	promptsearch("RE Search: ", REGEXP);
}

void Zagain(void)
{
	if (searchdir[0] & AGAIN_WRAP) {
		searchdir[0] &= ~AGAIN_WRAP;
		btostart(Bbuff);
		putpaw("Wrapped....");
		dosearch();
	} else if (promptsearch("Search: ", AGAIN) == 0)
		searchdir[0] |= AGAIN_WRAP;
}

void Zreplace(void)
{
	promptreplace(FORWARD);
}

void Zquery_replace(void)
{
	promptreplace(QUERY);
}

void Zre_replace(void)
{
	promptreplace(REGEXP);
}

static void doreplace(int type)
{
	bool exit = false, crgone, query;
	struct regexp re;
	struct mark *pmark, tmark;
	struct zbuff *tbuff, *save = Curbuff;
	int rc = 0;

	Arg = 0;

	query = type == FORWARD ? false : true;

	crgone = *olds && *(olds + strlen(olds) - 1) == '\n';
	pmark = bcremark(Bbuff);
	if (!pmark) {
		tbell();
		return;
	}

	if (type == REGEXP)
		rc = re_compile(&re, olds, REG_EXTENDED);
	if (rc) {
		re_error(rc, &re, PawStr, COLMAX);
		error("%s", PawStr);
	} else if (Argp) {
		foreachbuff(tbuff) {
			if (exit)
				break;
			cswitchto(tbuff);
			bmrktopnt(Bbuff, &tmark);
			btostart(Bbuff);
			while (replaceone(type, &query, &exit, &re, crgone))
				if (exit)
					break;
			bpnttomrk(Bbuff, &tmark);
		}
		clrpaw();
		cswitchto(save);
	} else if (!replaceone(type, &query, &exit, &re, crgone) && !exit)
		putpaw("Not Found");
	else
		clrpaw();

	bpnttomrk(Bbuff, pmark);
	bdelmark(pmark);
	if (type == REGEXP)
		re_free(&re);
}

static void promptreplace(int type)
{
	const char *prompt = NULL;
	int rc;

	switch (type) {
	case FORWARD:
		prompt = "Replace: ";		break;
	case QUERY:
		prompt = "QReplace: ";		break;
	case REGEXP:
		prompt = "RE Replace: ";	break;
	}
	if (getarg(nocase(prompt), olds, STRMAX))
		return;
	rc = getarg(nocase("with: "), news, STRMAX);
	if (rc == ABORT)
		return;
	if (rc)
		*news = '\0';

	doreplace(type);
}

static bool next_replace(struct regexp *re, struct mark *REstart, int type)
{
	if (type == REGEXP)
		return re_step(Bbuff, re, REstart);

	if (bstrsearch(olds, FORWARD)) {
		bmove(Bbuff, -(int)strlen(olds));
		return true;
	}

	return false;
}

static bool replaceone(int type, bool *query, bool *exit,
		       struct regexp *re, bool crgone)
{
	bool found = false;
	char tchar = ',', *ptr;
	int dist, changeprev = 0;
	struct mark *prevmatch;
	struct mark *REstart = NULL;

	if (type == REGEXP) {
		REstart = bcremark(Bbuff);
		if (!REstart) {
			tbell();
			return false;
		}
	}

	prevmatch = bcremark(Bbuff);
	if (!prevmatch) {
		bdelmark(REstart);
		tbell();
		return false;
	}
	putpaw("Searching...");
	while (!*exit && next_replace(re, REstart, type)) {
		found = true;
		if (*query) {
replace:
			putpaw("Replace? ");
			zrefresh();
input:
			switch (tchar = tgetkb()) {
			case ' ':
			case ',':	/* handled later */
			case 'Y':
			case 'y':
				break;	/* do the change */

			case '.':
				*exit = true; /* change, then abort */
				break;

			case '!':
				*query = false; /* global change */
				putpaw("Replacing...");
				break;

			case 'U': /* goto prev match */
			case 'u':
				if (type == REGEXP)
					tbell();
				else {
					bpnttomrk(Bbuff, prevmatch);
					if (changeprev) {
						bdelete(Bbuff, strlen(news));
						binstr(Bbuff, olds);
						bpnttomrk(Bbuff, prevmatch);
					}
				}
				goto replace;

			case '?':
				putpaw(QHELP);
				goto input;

			case 'S': /* skip file */
			case 's':
				*exit = true;
				continue;

			case 'q': /* abort */
			case 'Q':
				*exit = true;
				continue;

			default:
				if (Keys[(int)tchar] == ZABORT)
					*exit = true;
				else {
					bmrktopnt(Bbuff, prevmatch);
					changeprev = 0;
					/* skip and continue */
					bmove1(Bbuff);
				}
				continue;
			}
		}
		bmrktopnt(Bbuff, prevmatch);
		changeprev = 1;

		/* change it! */
		if (type == REGEXP) {
			/* force killtomrk to delete previous kill */
			Lfunc = ZNOTIMPL;
			killtomrk(REstart);
			for (ptr = news; *ptr; ++ptr)
				switch (*ptr) {
				case '\\':
					binsert(Bbuff, *(++ptr) ? *ptr : '\\');
					break;
				case '&':
					Zyank();
					break;
				default:
					binsert(Bbuff, *ptr);
				}
		} else {
			bdelete(Bbuff, strlen(olds));
			binstr(Bbuff, news);
		}
		if (*query && tchar == ',') {
			zrefresh();
			if (ask("Confirm? ") != YES) {
				/* change it back */
				if (type == REGEXP) {
					for (dist = 0;
						 !bisatmrk(Bbuff, REstart);
						 ++dist, bmove(Bbuff, -1))
						;
					bdelete(Bbuff, dist);
					Zyank();
				} else {
					int len = strlen(news);
					bmove(Bbuff, -len);
					bdelete(Bbuff, len);
					binstr(Bbuff, olds);
				}
			}
		}
		/* special case for "^" && "$" search strings */
		if (type == REGEXP && (ISNL(Buff()) || re->circf) && !crgone)
			bmove1(Bbuff);
		if (*query)
			putpaw("Searching...");
		else if (tkbrdy())
			*exit = true;
	}
	bdelmark(prevmatch);
	bdelmark(REstart);
	return found;
}

static int promptsearch(const char *prompt, int type)
{
	if (*olds == '\0' || type != AGAIN) {
		if (getarg(nocase(prompt), olds, STRMAX)) {
			Arg = 0;
			return ABORT;
		}
		searchdir[0] = type == AGAIN ? FORWARD : type;
		searchdir[1] = 0;
	}
	return dosearch();
}

static bool dosearch(void)
{
	int fcnt = 0, rc;
	struct mark save, fmark;

	bmrktopnt(Bbuff, &save);
	if (searchdir[0] == REGEXP) {
		struct regexp re;

		rc = re_compile(&re, olds, REG_EXTENDED);
		if (rc) {
			re_error(rc, &re, PawStr, COLMAX);
			error("%s", PawStr);
		} else {
			while (Arg-- > 0)
				if (re_step(Bbuff, &re, NULL)) {
					bmrktopnt(Bbuff, &fmark);
					++fcnt;
				} else
					break;
			re_free(&re);
		}
	} else
		while (Arg-- > 0)
			if (bstrsearch(olds, searchdir[0])) {
				bmrktopnt(Bbuff, &fmark);
				++fcnt;
			} else
				break;
	if (fcnt)
		bpnttomrk(Bbuff, &fmark);
	else {
		bpnttomrk(Bbuff, &save);
		putpaw("Not Found");
	}
	Arg = 0;
	return fcnt;
}

char *nocase(const char *prompt)
{
	static char is[20], upper[20];

	if (prompt) {
		const char *p;
		char *u;

		strcpy(is, prompt);
		for (p = prompt, u = upper; *p; ++p, ++u)
			*u = toupper(*p);
		Insearch = true;
	}
	return (Curbuff->bmode & EXACT) ? is : upper;
}
/* @} */
