/* help.c - Zedit help command
 * Copyright (C) 1988-2013 Sean MacLennan
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
#include "keys.h"

#if HELP || BUILTIN_DOCS
static char *Htype[] = {
	"Bindings",
	"Buffer/Window",
	"Copy/Delete",
	"Cursor",
	"Display",
	"File",
	"Help/Status",
	"Mode",
	"Other",
	"Search/Replace",
	"Shell",
	"Special",
#if HELP
	"Variables",
#endif
};
#define HTYPES	(sizeof(Htype) / sizeof(char *))

static void dump_bindings(int fnum)
{
	int k, found = 0;
	char buff[BUFSIZ];

	binstr("\nBinding(s): ");

	for (k = 0; k < NUMKEYS; ++k)
		if (Keys[k] == fnum)
			if (notdup_key(k)) {
				if (found)
					binstr(",  ");
				else
					found = true;
				binstr(dispkey(k, buff));
			}

	if (!found)
		binstr("Unbound");
}
#endif

#if BUILTIN_DOCS
#include "func-docs.h"

/***
 * Displays help on any of the Zedit functions.
 * Prompts for the function with full completion.
 */
void Zhelp_function(void)
{
	int rc = getplete("Function: ", NULL, (char **)Cnames, CNAMESIZE, NUMFUNCS);
	if (rc == -1)
		return;

	wuseother(HELPBUFF);

	binstr(Cnames[rc].name);
	binstr("\n\n");

	binstr(func_docs[rc]);

	if (Cnames[rc].fnum != ZNOTIMPL &&
	    Cnames[rc].fnum != ZINSERT)
		dump_bindings(Cnames[rc].fnum);

	btostart();
}

/***
 * The Zedit functions are grouped into categories. This command lets
 * you see the grouped functions.
 */
void Zhelp_group(void)
{
	char line[80];
	int i, j, n;
	int rc = getplete("Category: ", NULL, (char **)Htype, sizeof(char *), HTYPES);
	if (rc == -1)
		return;

	wuseother(HELPBUFF);

	binstr(Htype[rc]);
	binstr("\n\n");

	for (i = j = n = 0; i < NUMFUNCS; ++i)
		if (Cnames[i].htype == rc) {
			n += sprintf(line + n, "%-24s", Cnames[i].name);
			if (++j == 3) {
				binstr(line);
				binsert('\n');
				j = n = 0;
			}
		}

	if (j)
		binstr(line);
	binsert('\n');

	btostart();
}
#else
void Zhelp_function(void) { tbell(); }
#endif

#if HELP
/* a "sentence" ends in a tab, NL, or two consecutive spaces */
static int issentence(void)
{
	static Byte prev = '\0';
	int rc;

	rc = Buff() != '\t' && Buff() != NL && !(Buff() == ' ' && prev == ' ');
	prev = rc ? Buff() : '\0';
	return rc;
}

static void dispit(char *name, int *col)
{
	bmakecol(*col, true);
	binstr(name);
	if (*col == 45)
		binsert('\n');
	*col ^= 40;
}

/* If type == -1, use saved value */
static void helpit(int type)
{
	static int was;
	char buff[STRMAX];
	int col = 5, i;

	putpaw("Please wait...");
	bempty();
	if (type == -1)
		type = was;
	else
		was = type;
	sprintf(buff, "- %s Help -\n\n", Htype[type]);
	tindent((Colmax - strlen(buff)) >> 1);
	binstr(buff);
	if (type == H_VAR)
		for (i = 0; i < VARNUM; ++i)
			dispit(Vars[i].vname, &col);
	else
		for (i = 0; i < NUMFUNCS; ++i)
			if (Cnames[i].htype == type)
				dispit(Cnames[i].name, &col);
	if (col == 45)
		binsert('\n');
	binstr("\n     Top Level Help Menu");
	btostart();
	Curbuff->bmodf = false;
	clrpaw();
}

static void massage(char *buff)
{
	while (*buff) {
		if (*buff == ' ' || *buff == '/')
			*buff = '-';
		else if (isupper(*buff))
			*buff = tolower(*buff);
		else if (*buff == '\n')
			*buff = '\0';
		++buff;
	}
}

static FILE *findhelp(int code, bool func, char *buff)
{
	FILE *fp = NULL;
	char *ptr;
	int len;

	if (findpath(buff, ZHFILE))
		fp = fopen(buff, "r");
	if (!fp) {
		putpaw("Unable to Open Help File");
		return NULL;
	}
	ptr = func ? Cnames[code].name : Vars[code].vname;
	len = strlen(ptr);

	putpaw("Looking in help file...");
	while (fgets(buff, STRMAX, fp)) {
		massage(buff);
		if (*buff == ':' && strncmp(ptr, &buff[1], len) == 0) {
			clrpaw();
			return fp;
		}
	}
	fclose(fp);
	putpaw("No Help");
	return NULL;
}

static void help(int code, bool func)
{
	FILE *fp;
	char buff[BUFSIZ], *p;

	Arg = 0;
	if (code < 0)
		return;
	fp = findhelp(code, func, buff);
	if (fp) {
		bempty();
		p = buff + 1;
		do
			binstr(p);
		while (fgets(p = buff, BUFSIZ, fp) && *buff != ':');
		fclose(fp);

		/* setup to display either Bindings or Current value */
		if (func) {
			if (Cnames[code].fnum != ZNOTIMPL &&
			    Cnames[code].fnum != ZINSERT)
				dump_bindings(Cnames[code].fnum);
		} else {
			binstr("\nCurrent value: ");
			varval(code);
		}
		btostart();
	}
}

/***
 * .(b C
 * - Zedit Help -
 *
 *
 * Bindings
 * Buffer/Window
 * Copy/Delete
 * Cursor
 * Display
 * File
 * Help/Status
 * Macro
 * Mode
 * Search/Replace
 * Shell
 * Other
 *
 * Variables
 * .)b
 */
void Zhelp(void)
{
	static Byte level = 0, z;
	struct buff *tbuff;
	FILE *fp = NULL;
	char str[STRMAX + 1];
	int i;

	if (level) {
		tbuff = cfindbuff(HELPBUFF);
		if (tbuff != Curbuff) {
			/* just switch to the .help buffer */
			strcpy(Lbufname, Curbuff->bname);
			bgoto(tbuff);
			return;
		}
	}
	switch (level) {
	case 0:
		/* create the window */
		for (z = 0; z < NUMFUNCS && Cnames[z].fnum != ZHELP; ++z)
			;
		fp = findhelp(z, true, str);
		if (fp == NULL)
			return;
		cmakebuff(HELPBUFF, NULL);
		/* fall thru to level 1 */

	case 1:
		/* read in the top level */
		if (!fp) {
			fp = findhelp(z, true, str);
			if (!fp)
				break;
		}
		bempty();
		while (fgets(str, STRMAX, fp) && *str != ':') {
			char *p;

			for (p = str; isspace(*p); ++p)
				;
			if (*p) {
				binstr(str);
				break;
			}
		}
		while (fgets(str, STRMAX, fp) && *str != ':')
			binstr (str);
		fclose(fp);
		btostart();
		bcsearch('n');
		Curbuff->bmodf = false;
		level = 2;
		break;

	case 2:
		/* accept input from top level and create secondary level */
		getbword(str, STRMAX, issentence);
		for (i = 0; i < HTYPES; ++i)
			if (strcmp(str, Htype[i]) == 0) {
				helpit(i);
				level = 3;
			}
		break;

	case 3:
		/* accept input from secondary level and display help */
		bmakecol(bgetcol(false, 0) < 40 ? 5 : 45, false);
		getbword(str, 30, issentence);
		if (strncmp(str, "Top", 3) == 0) {
			level = 1;
			Zhelp();
		} else {
			for (i = 0; i < NUMFUNCS; ++i)
				if (strcmp(str, Cnames[i].name) == 0) {
					level = 4;
					help(i, true);
					return;
				}
			for (i = 0; i < VARNUM; ++i)
				if (strcmp(str, Vars[i].vname) == 0) {
					level = 4;
					help(i, false);
					return;
				}
		}
		break;

	case 4:
		/* go back to secondary level */
		helpit(-1);
		level = 3;
		break;
	}
}
#else
void Zhelp(void) { tbell(); }
#endif
