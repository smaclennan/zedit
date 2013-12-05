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

static char *dispkey(unsigned key, char *s)
{
	char *p;
	int j;

	*s = '\0';
	if (is_special(key))
		return strcpy(s, special_label(key));
	if (key > 127)
		strcpy(s, key < 256 ? "M-" : "C-X ");
	j = key & 0x7f;
	if (j == 27)
		strcat(s, "ESC");
	else if (j < 32 || j == 127) {
		strcat(s, "C-");
		p = s + strlen(s);
		*p++ = j ^ '@';
		*p = '\0';
	} else if (j == 32)
		strcat(s, "Space");
	else {
		p = s + strlen(s);
		*p++ = j;
		*p = '\0';
	}
	return s;
}

static void dump_bindings(int fnum)
{
	int k, found = 0;
	char buff[BUFSIZ];

	binstr("\nBinding(s): ");

	for (k = 0; k < NUMKEYS; ++k)
		if (Keys[k] == fnum) {
			if (found)
				binstr(",  ");
			else
				found = true;
			binstr(dispkey(k, buff));
		}

	if (!found)
		binstr("Unbound");
}

void dump_doc(const char *doc)
{
	binstr("\n\n");

	for (; *doc; ++doc)
		if (*doc == ' ') {
			Cmd = *doc;
			Zfill_check();
		} else
			binsert(*doc);
	binsert('\n');
}

void Zhelp(void)
{
	Cmd = delayprompt("Help: a=apropos f=function k=key v=variable");
	switch (Cmd) {
	case 'a':
		Zhelp_apropos();
		break;
	case 'f':
		Zhelp_function();
		break;
	case 'k':
		Zhelp_key();
		break;
	case 'v':
		Zhelp_variable();
		break;
	default:
		Zabort();
	}
}

void Zhelp_function(void)
{
	int rc = getplete("Function: ", NULL, (char **)Cnames,
			  CNAMESIZE, NUMFUNCS);
	if (rc == -1)
		return;

	if (Argp) {
		struct buff *buff = cmakebuff(HELPBUFF, NULL);
		if (buff == NULL)
			return;
		cswitchto(buff);
		bempty();
	} else
		wuseother(HELPBUFF);

	binstr(Cnames[rc].name);

	dump_doc(Cnames[rc].doc);

	if (Cnames[rc].fnum != ZNOTIMPL &&
	    Cnames[rc].fnum != ZINSERT)
		dump_bindings(Cnames[rc].fnum);

	btostart();
	Curbuff->bmodf = false;
}

void Zhelp_apropos(void)
{
	char word[STRMAX], line[80];
	int i, j, n, match = 0;

	*word = '\0';
	if (getarg("Word: ", word, sizeof(word)))
		return;

	for (i = j = n = 0; i < NUMFUNCS; ++i)
		if (strstr(Cnames[i].name, word)) {
			if (match++ == 0)
				wuseother(HELPBUFF);
			n += sprintf(line + n, "%-24s", Cnames[i].name);
			if (++j == 3) {
				binstr(line);
				binsert('\n');
				j = n = 0;
			}
		}

	if (j) {
		binstr(line);
		binsert('\n');
	}

	if (match == 0)
		putpaw("No matches.");
	else {
		btostart();
		Curbuff->bmodf = false;
	}
}

void Zhelp_key(void)
{
	char kstr[12];
	int rc;
	unsigned raw, key;

	Arg = 0;
	putpaw("Key: ");
	raw = tgetcmd();
	key = Keys[raw];
	if (key == ZCTRL_X) {
		putpaw("Key: C-X ");
		raw = tgetcmd() + 256;
		key = Keys[raw];
	} else if (key == ZMETA) {
		putpaw("Key: M-");
		raw = tgetcmd() + 128;
		key = Keys[raw];
	}

	if (key == ZNOTIMPL)
		putpaw("%s Unbound", dispkey(raw, kstr));
	else
		for (rc = 0; rc < NUMFUNCS; ++rc)
			if (Cnames[rc].fnum == key)
				putpaw("%s Bound to %s",
					dispkey(raw, kstr), Cnames[rc].name);
}
