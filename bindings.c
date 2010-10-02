/* bindings.c - Zedit binding commands
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
#include <sys/stat.h>

Byte CRdefault = ZNEWLINE;

static Boolean bindfile(char *fname, int mode);
static Boolean bindone(char *prompt, int first, int *key);

void Zbind(void)
{
	int f, key;

	if (Argp) {
		Bind();
		Arg = 0;
		return;
	}
	f = Getplete("Bind: ", (char *)NULL, (char **)Cnames,
		     CNAMESIZE, NUMFUNCS);
	if (f != -1) {
		if (bindone("Key: ", TRUE, &key)) {
			Keys[key] = Cnames[f].fnum;
			if (key == CR)
				CRdefault = Keys[key];
		} else
			tbell();
	}
	clrecho();
}


void Zkeybind(void)
{
	char str[STRMAX];
	int rc;
	unsigned key;

	Arg = 0;
	Echo("Key: ");
	key = Keys[Tgetcmd()];
	if (key == ZCTRLX) {
		Echo("Key: C-X ");
		key = Keys[Tgetcmd() + 256];
	} else if (key == ZMETA) {
		Echo("Key: M-");
		key = Keys[Tgetcmd() + 128];
	}

	if (key == ZNOTIMPL)
		Echo("Unbound");
	else
		for (rc = 0; rc < NUMFUNCS; ++rc)
			if (Cnames[rc].fnum == key) {
				sprintf(str, "Bound to %s", Cnames[rc].name);
				Echo(str);
			}
}

/* Don't display both C-X A and C-X a if bound to same Ditto for Meta */
Boolean notdup_key(int k)
{
	return ((k < (256 + 'a') || k > (256 + 'z')) &&
		(k < (128 + 'a') || k > (128 + 'z'))) ||
		Keys[k] != Keys[k - ('a' - 'A')];
}

void Zcmdbind(void)
{
	char line[STRMAX];
	int f, k, found = 0;

	Arg = 0;
	*PawStr = '\0';
	f = Getplete("Command: ", NULL, (char **)Cnames, CNAMESIZE, NUMFUNCS);
	if (f != -1) {
		for (k = 0; k < NUMKEYS; ++k)
			if (Keys[k] == Cnames[f].fnum)
				if (notdup_key(k)) {
					if (found)
						strcat(PawStr, " or ");
					strcat(PawStr, dispkey(k, line));
					if (strlen(PawStr) > Colmax)
						break;
					found = TRUE;
				}
		if (found)
			Echo(PawStr);
		else
			Echo("Unbound");
	}
}

static void out(char *line, FILE *fp)
{
	if (fp)
		fputs(line, fp);
	else
		binstr(line);
}

static void outto(FILE *fp, int col)
{
	int i;

	for (i = bgetcol(FALSE, 0); i < col; ++i)
		out(" ", fp);
}

void Zdispbinds(void)
{
	Boolean found;
	FILE *fp;
	char line[STRMAX];
	int f;
	unsigned k;

	if (Argp) {
		*line = '\0';
		if (Getarg("Output file: ", line, STRMAX))
			return;
		fp = fopen(line, "w");
		if (fp == NULL) {
			Echo("Unable to create");
			return;
		}
	} else {
		fp = NULL;
		WuseOther(LISTBUFF);
	}
	Echo("Please Wait...");
	out("COMMAND                            PERMS     BINDING\n", fp);
	for (f = 0; f < NUMFUNCS; ++f)
		if (Cnames[f].fnum != ZNOTIMPL && Cnames[f].fnum != ZINSERT) {
			sprintf(line, "%-35s%cw%c       ", Cnames[f].name,
				Cmds[Cnames[f].fnum][1] == Znotimpl ? '-' : 'r',
				Cmds[Cnames[f].fnum][2] == Znotimpl ? '-' : 'p'
				);
			out(line, fp);
			found = FALSE;
			for (k = 0; k < NUMKEYS; ++k)
				if (Keys[k] == Cnames[f].fnum)
					if (notdup_key(k)) {
						if (found)
							outto(fp, 45);
						out(dispkey(k, line), fp);
						out("\n", fp);
						found = TRUE;
					}
			if (!found)
				out("Unbound\n", fp);
		}
	btostart();
	if (!fp)
		Curbuff->bmodf = FALSE;
	clrecho();
	Arg = 0;
}

static char *bindfname(char *fname)
{
#ifdef XWINDOWS
	strcpy(fname, ".zb.X");
#else
	sprintf(fname, ".zb.%s", Term);
#endif
	return fname;
}

void loadbind(void)
{
	char fname[30], path[PATHMAX + 1];
	int i;

	bindfname(fname);
	for (i = FINDPATHS; i && (i = Findpath(path, fname, i, TRUE)); --i)
		bindfile(path, READ_BINARY);
	fcheck();
}


/* save a bindings file.
 * Use Findpath starting at the $HOME directory to find where to
 * save the bindings file. If no bindings file exists, save in the
 * $HOME dir.
 */
void Zsavebind(void)
{
	char fname[30], path[PATHMAX + 1];
	int i, n;

	bindfname(fname);
	for (n = 0, i = 3; i && (i = Findpath(path, fname, i, TRUE)); --i)
		n = i;
	if (n)
		Findpath(path, fname, n, TRUE);
	else
		sprintf(path, "%s/%s", Me->pw_dir, fname);
	if (Argp && Getfname("Bind File: ", path))
		return;
	if (bindfile(path, WRITE_MODE)) {
		sprintf(PawStr, "%s written.", path);
		Echo(PawStr);
	}
}

static Boolean bindfile(char *fname, int mode)
{
	char version[3];
	int fd, modesave, rc = FALSE;

	fd = open(fname, mode, Cmask);
	if (fd == EOF) {
		if (mode == WRITE_MODE)
			Error("Unable to Create Bindings File");
		return FALSE;
	}

	modesave = Curbuff->bmode;		/* set mode to normal !!! */
	Curbuff->bmode = NORMAL;
	Curwdo->modeflags = INVALID;
	if (mode == WRITE_MODE) {
		write(fd, "01", 2);
		if (write(fd, (char *)Keys, 510) != 510)
			Error("Unable to Write Bindings File");
		else
			rc = TRUE;
	} else {
		read(fd, version, 2);
		if (*version != '0')
			Error("Incompatible Bindings File");
		else if (read(fd, (char *)Keys, NUMKEYS) == -1)
			Error("Unable to Read Bindings File");
		else {
			CRdefault = Keys[CR];
			rc = TRUE;
		}
	}

	(void)close(fd);
	Curbuff->bmode = modesave;
	Curwdo->modeflags = INVALID;

	return rc;
}

static Boolean bindone(char *prompt, int first, int *key)
{
	Echo(prompt);
	*key = Tgetcmd();
	if (Keys[*key] == ZABORT)
		return FALSE;
	else if (Keys[*key] == ZQUOTE) {
		Arg = 0;
		Zquote();
		*key = Cmd;
	} else if (first && Keys[*key] == ZMETA)
		if (bindone("Key: M-", FALSE, key))
			*key += 128;
		else
			return FALSE;
	else if (first && Keys[*key] == ZCTRLX) {
		if (bindone("Key: C-X ", FALSE, key))
			*key += 256;
		else
			return FALSE;
	}
	return TRUE;
}

char *dispkey(unsigned key, char *s)
{
	char *p;
	int j;

	*s = '\0';
#ifdef XWINDOWS
	if (key >= ZXK_START && key < NUMKEYS)
		return strcpy(s, KeyNames[key - ZXK_START]);
#else
	if (key > SPECIAL_START)
		return strcpy(s, Tkeys[key - SPECIAL_START].label);
	if (key > 127)
		strcpy(s, key < 256 ? "M-" : "C-X ");
#endif
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
