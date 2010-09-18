/* make.c - Zedit make commands
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

/* This is cleared in Zmake and set in Znexterror.
 * If clear, the make buffer is scrolled up. Once a next error is
 * called, the buffer is kept at the error line.
 */
static int NextErrorCalled;

/* Do a "make" command - basically a shell command in the ".make" buffer */
Proc Zmake()
{
#ifndef BORDER3D
	Buffer *mbuff;
#endif

	NextErrorCalled = 0;	/* reset it */
	Arg = 0;
	if (Argp) {
		Argp = FALSE;
		if (Getarg("Make: ", mkcmd, STRMAX))
			return;
	}
	Saveall(TRUE);
#ifdef BORDER3D
	Refresh();		/* update mode lines from Saveall */
	RunMakeCmd();
#else
#if XWINDOWS
	if (Vars[VPOPMAKE].val) {
		RunMakeCmd();
		Refresh();	/* update mode lines from Saveall */
		return;
	}
#endif
#if PIPESH || XWINDOWS
	mbuff = Cfindbuff(MAKEBUFF);
	if (mbuff && mbuff->child != EOF) {
		Echo("Killing current make.");
		Unvoke(mbuff, TRUE);
		Clrecho();
	}
#endif
	mbuff = Cmdtobuff(MAKEBUFF, mkcmd);
	if (mbuff)
		Message(mbuff, mkcmd);
	else
		Error("Unable to execute make.");
#endif
}

/* Do a "make" command - basically a shell command in the ".make" buffer */
Proc Zgrep()
{
	Buffer *mbuff;
	char cmd[STRMAX * 2];

	NextErrorCalled = 0;	/* reset it */
	Arg = 0;
	if (Argp) {
		Argp = FALSE;
		if (Getarg("grep command: ", grepcmd, STRMAX))
			return;
	}
	sprintf(cmd, "sh -c '%s ", grepcmd);
	if (Getarg("grep: ", cmd + strlen(cmd), STRMAX))
		return;
	strcat(cmd, "'");
	Saveall(TRUE);
#if PIPESH || XWINDOWS
	mbuff = Cfindbuff(MAKEBUFF);
	if (mbuff && mbuff->child != EOF) {
		Error("Make buffer in use...");
		return;
	}
#endif
	Dbg("grep (%s)\n", cmd);
	mbuff = Cmdtobuff(MAKEBUFF, cmd);
	if (mbuff)
		Message(mbuff, cmd);
	else
		Error("Unable to execute grep.");
}

Proc Znexterr()
{
#ifdef BORDER3D
	ZmakeNextErr();
#else
	WDO *wdo;
	Buffer *save, *mbuff;
	char fname[STRMAX + 1];
	char path[PATHMAX + 1];
	int line;

#if XWINDOWS
	if (Vars[VPOPMAKE].val && ZmakeNextErr())
		return;
#endif

	mbuff = Cfindbuff(MAKEBUFF);
	if (!mbuff) {
		Tbell();
		return;
	}
	save = Curbuff;
	Bswitchto(mbuff);
	if (!NextErrorCalled) {
		NextErrorCalled = 1;
		Btostart();
	}
	line = Parse(fname);
	if (line) {
		Vsetmrk(Curbuff->mark);
		Bmrktopnt(Curbuff->mark);
		Tobegline();
		Bswappnt(Curbuff->mark);
		Vsetmrk(Curbuff->mark);
		wdo = Findwdo(mbuff);
		if (wdo)
			Mrktomrk(wdo->wstart, Curbuff->mark);
		Pathfixup(path, fname);
		Findfile(path, FALSE);
		Argp = TRUE;
		Arg = line;
		Zlgoto();
		Tobegline();
	} else {
		Btoend();
		Bmrktopnt(Curbuff->mark);
		Bswitchto(save);
		Echo("No more errors");
	}
	Argp = FALSE;
	Arg = 0;
#endif
}

#if PIPESH || XWINDOWS
/* kill the make */
Proc Zkill()
{
	Unvoke(Cfindbuff(MAKEBUFF), FALSE);
}
#else
Proc Zkill() { Tbell(); }
#endif

/* Check if it is a warning or an error.
 * Currently works for GCC, g++, MIPs.
 */
static Boolean IsWarning;

static Boolean Warning()
{
	if (Argp) {
		if (IsWarning)
			return TRUE;

		if (Buff() == ':') {
			char word[10], *p;
			Getbword(p = word, 10, Isnotws);
			if (*p == ':')
				++p;
			return strcmp(p, "warning:") == 0;
		}
	}
	return 0;
}

/* Find the next error in the .make buffer.
 * Ignores lines that start with a white space.
 * Supported:
 *	CC		"<fname>", line <line>: <msg>
 *	GNU C		<fname>:<line>: (error: | warning:) <msg>
 *      G++		<fname>:<line>: [warning:] <msg>
 *	HP CC		cc: "<fname>", line <line>: <msg>
 *	AS		as: "<fname>", line <line>: <msg>
 *	High C		[Ew] "<fname>",L<line>/C<column>: <msg>
 *	Microsoft	<fname>(<line>) : <msg>
 *	MIPS C		cfe: (Warning|Error): <fname>, <line>: <msg>
 *	MIPS AS		as(0|1): (Warning|Error): <fname>, line <line>: <msg>
 *
 *	ignores		conflicts: <line>
 */
int Parse(char *fname)
{
	char word[41], *p;
	int line, n;

	while (!Bisend()) {
		IsWarning = 0;

		/* get first word in line */
		n = Getbword(word, 40, Isnotws);

		/* check for: as: cc: */
		if (strcmp(word, "as:") == 0 || strcmp(word, "cc:") == 0)
			Bmove(4);
		/* check for cfe:/as0:/as1: (MIPS) */
		else if (strcmp(word, "cfe:") == 0 ||
			 strcmp(word, "as0:") == 0 ||
			 strcmp(word, "as1:") == 0) {
			Bmove(5);
			IsWarning = Buff() == 'W';
			Bmove(IsWarning ? 9 : 7);
		}
		/* check High C for "E " or "w " */
		else if (n == 1 && (*word == 'E' || *word == 'w'))
			Bmove(2);
		else if (strcmp(word, "conflicts:") == 0) {
			Bcsearch(NL);	/* skip line */
			continue;
		}
		/* try to get the fname */
		if (Buff() == '"')
			Bmove1();
		for (p = fname; !strchr("\",:(\n", Buff()); Bmove1())
			*p++ = Buff();
		*p = '\0';
		if (Buff() == '"')
			Bmove1();

		/* try to get the line */
		if (Buff() == ':' || Buff() == '(')
			Bmove1();
		else if (Buff() == ',') {
			while (!isdigit(Buff()) && Buff() != '\n' && !Bisend())
				Bmove1();
		}

		/* look for line number */
		line = Batoi();
		if (line != 0 && !Warning())
			return line;

		/* skip to next line */
		Bcsearch(NL);
	}
	return 0;
}
