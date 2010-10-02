/* help.c - Zedit help command
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
#include "keys.h"

#ifdef HELP
static char *Htype[] = {
	"Special",
	"Other",
	"Variables",
	"Cursor",
	"Copy/Delete",
	"Search/Replace",
	"File",
	"Buffer/Window",
	"Display",
	"Mode",
	"Help/Status",
	"Bindings",
	"Shell",
};
#define HTYPES	(sizeof(Htype) / sizeof(char *))


/* a "sentence" ends in a tab, NL, or two consecutive spaces */
static int Issentence(void)
{
	static Byte prev = '\0';
	int rc;

	rc = Buff() != '\t' && Buff() != NL && !(Buff() == ' ' && prev == ' ');
	prev = rc ? Buff() : '\0';
	return rc;
}

void Zhelp(void)
{
	static Byte level = 0, z;
	struct buff *tbuff, *was;
	FILE *fp = NULL;
	char str[STRMAX + 1];
	int i;

	if (level) {
		tbuff = Cfindbuff(HELPBUFF);
		if (tbuff != Curbuff) {
			/* just switch to the .help buffer */
			strcpy(Lbufname, Curbuff->bname);
			Bgoto(tbuff);
			return;
		}
	}
	switch (level) {
	case 0:
		/* create the window */
		for (z = 0; z < NUMFUNCS && Cnames[z].fnum != ZHELP; ++z)
			;
		fp = Findhelp(z, TRUE, str);
		if (fp == NULL)
			return;
		was = Curbuff;
		if (WuseOther(HELPBUFF)) {
			strcpy(Lbufname, was->bname);
			Curbuff->bmode |= VIEW;
		} else {
			fclose(fp);
			break;
		}
		/* fall thru to level 1 */

	case 1:
		/* read in the top level */
		if (!fp) {
			fp = Findhelp(z, TRUE, str);
			if (!fp)
				break;
		}
		bempty();
		while (fgets(str, STRMAX, fp) && *str != ':')
			binstr(str);
		fclose(fp);
		btostart();
		bcsearch('n');
		Curbuff->bmodf = FALSE;
		level = 2;
		break;

	case 2:
		/* accept input from top level and create secondary level */
		Getbword(str, STRMAX, Issentence);
		for (i = 0; i < HTYPES; ++i)
			if (strcmp(str, Htype[i]) == 0) {
				Helpit(i);
				level = 3;
			}
		break;

	case 3:
		/* accept input from secondary level and display help */
		bmakecol(bgetcol(FALSE, 0) < 40 ? 5 : 45, FALSE);
		Getbword(str, 30, Issentence);
		if (strncmp(str, "Top", 3) == 0) {
			level = 1;
			Zhelp();
		} else {
			for (i = 0; i < NUMFUNCS; ++i)
				if (strcmp(str, Cnames[i].name) == 0) {
					level = 4;
					Help(i, TRUE);
					return;
				}
			for (i = 0; i < VARNUM; ++i)
				if (strcmp(str, Vars[i].vname) == 0) {
					level = 4;
					Help(i, FALSE);
					return;
				}
		}
		break;

	case 4:
		/* go back to secondary level */
		Helpit(-1);
		level = 3;
		break;
	}
}

/* If type == -1, use saved value */
void Helpit(int type)
{
	static int was;
	char buff[STRMAX];
	int col = 5, i;

	Echo("Please wait...");
	bempty();
	if (type == -1)
		type = was;
	else
		was = type;
	sprintf(buff, "- %s Help -\n\n", Htype[type]);
	Tindent((Colmax - strlen(buff)) >> 1);
	binstr(buff);
	if (type == H_VAR)
		for (i = 0; i < VARNUM; ++i)
			Dispit(Vars[i].vname, &col);
	else
		for (i = 0; i < NUMFUNCS; ++i)
			if (Cnames[i].htype == type)
				Dispit(Cnames[i].name, &col);
	if (col == 45)
		binsert('\n');
	binstr("\n     Top Level Help Menu");
	btostart();
	Curbuff->bmodf = FALSE;
	Clrecho();
}


void Dispit(char *name, int *col)
{
	bmakecol(*col, TRUE);
	binstr(name);
	if (*col == 45)
		binsert('\n');
	*col ^= 40;
}

static void dump_bindings(char *buff, int fnum)
{
	int k, found = 0;

	binstr("\nBinding(s): ");

	for (k = 0; k < NUMKEYS; ++k)
		if (Keys[k] == fnum)
			if (notdup_key(k)) {
				if (found)
					binstr(",  ");
				else
					found = TRUE;
				binstr(Dispkey(k, buff));
			}

	if (!found)
		binstr("Unbound");
}

void Help(int code, Boolean func)
{
	FILE *fp;
	char buff[BUFSIZ], *p;

	Arg = 0;
	if (code < 0)
		return;
	fp = Findhelp(code, func, buff);
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
				dump_bindings(buff, Cnames[code].fnum);
		} else {
			binstr("\nCurrent value: ");
			Varval(code);
		}
		btostart();
	}
}

FILE *Findhelp(int code, Boolean func, char *buff)
{
	FILE *fp;
	char *ptr;
	int len;

	Findpath(buff, ZHFILE, FINDPATHS, TRUE);
	fp = fopen(buff, "r");
	if (!fp) {
		Echo("Unable to Open Help File");
		return NULL;
	}
	ptr = func ? Cnames[code].name : Vars[code].vname;
	len = strlen(ptr);

	Echo("Looking in help file...");
	while (fgets(buff, STRMAX, fp))
		if (*buff == ':' && strncmp(ptr, &buff[1], len) == 0) {
			Clrecho();
			return fp;
		}
	fclose(fp);
	Echo("No Help");
	return NULL;
}

#else
void Zhelp(void) { Tbell(); }
void KillHelp(void) {}
#endif /* HELP */

int Isnotws(void)
{
	return Buff() != '\n' && Buff() != '\t' && Buff() != ' ';
}

